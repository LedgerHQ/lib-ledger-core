/*
 *
 * TezosLikeOperationDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/01/2021.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "CosmosLikeOperationsDatabaseHelper.hpp"
#include <database/PreparedStatement.hpp>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <api/BigInt.hpp>
#include <crypto/SHA256.hpp>
#include <unordered_set>
#include <debug/Benchmarker.h>
#include <wallet/tezos/database/TezosLikeTransactionDatabaseHelper.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/cosmos/database/SociCosmosAmount.hpp>
#include <wallet/cosmos/CosmosLikeMessage.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>



using namespace soci;

namespace {
    using namespace ledger::core;

    // Cosmos operations
    struct CosmosOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> msgUid;

        void update(const std::string& op, const std::string& msg) {
            uid.push_back(op);
            msgUid.push_back(msg);
        }

        void clear() {
            uid.clear();
            msgUid.clear();
        }
    };

    const auto UPSERT_COSMOS_OPERATION = db::stmt<CosmosOperationBinding>(
            "INSERT INTO cosmos_operations VALUES(:uid, :message_uid) "
            "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, use(b.uid), use(b.msgUid);
            });

   // Transaction
    struct TransactionBinding {
        std::vector<std::string> uid; 
        std::vector<std::string> hash; 
        std::vector<Option<std::string>> blockUid; 
        std::vector<std::string> date;
        std::vector<std::string> fee;
        std::vector<std::string> gas; 
        std::vector<Option<std::string>> gasUsed; 
        std::vector<std::string> memo;
        
        void update(const cosmos::Transaction& tx) {
            Option<std::string> txblockUid;
            if (tx.block.nonEmpty() && !tx.block.getValue().hash.empty()) {
                txblockUid = BlockDatabaseHelper::createBlockUid(tx.block.getValue());
            }
            auto txdate = DateUtils::toJSON(tx.timestamp);
            auto txfee = soci::coinsToString(tx.fee.amount);
            auto txgas = tx.fee.gas.toString();
            auto txgasUsed = tx.gasUsed.flatMap<std::string>([](const BigInt &g) {
                return g.toString();
            });

            uid.push_back(tx.uid);
            hash.push_back(tx.hash);
            blockUid.push_back(txblockUid);
            date.push_back(txdate);
            fee.push_back(txfee);
            gas.push_back(txgas);
            gasUsed.push_back(txgasUsed);
            memo.push_back(tx.memo);
        }

        void clear() {
            uid.clear();
            hash.clear();
            blockUid.clear();
            date.clear();
            fee.clear();
            gas.clear();
            gasUsed.clear();
            memo.clear();
        }
    };

    const auto UPSERT_TRANSACTION = db::stmt<TransactionBinding>(
        "INSERT INTO cosmos_transactions("
        "uid, hash, block_uid, time, fee_amount, gas, gas_used, memo"
        ") VALUES (:uid, :hash, :block_uid, :time, :fee, :gas, :gas_used, :memo)"
        " ON CONFLICT(uid) DO UPDATE SET block_uid = :buid, gas_used = :gused",
            [] (auto& s, auto&  b) {
                s, 
                use(b.uid), 
                use(b.hash), 
                use(b.blockUid), 
                use(b.date), 
                use(b.fee),
                use(b.gas), 
                use(b.gasUsed), 
                use(b.memo),
                use(b.blockUid), 
                use(b.gasUsed);          
            });



    // Messages: generic class
    struct MessageBinding {
        std::vector<std::string> uid;
        std::vector<std::string> txUid;
        std::vector<std::string> type;
        std::vector<std::string> log;
        std::vector<int32_t> success;
        std::vector<int32_t> messageIndex;
        
        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            uid.push_back(msg.uid);
            txUid.push_back(transactionUid);
            type.push_back(msg.type);
            log.push_back(messageLog.log);
            success.push_back(messageLog.success ? 1 : 0);
            messageIndex.push_back(messageLog.messageIndex);
        }

        void clear() {
            uid.clear();
            txUid.clear();
            type.clear();
            log.clear();
            success.clear();
            messageIndex.clear();
        }

        bool empty() const {
            return uid.empty();
        }
    };
    const auto UPSERT_MESSAGE_GENERIC = db::stmt<MessageBinding>(
        "INSERT INTO cosmos_messages ("
        "uid, transaction_uid, message_type, log, success, msg_index)"
        "VALUES (:uid, :tuid, :mt, :log, :success, :mi) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex);   
            });

    // Messages: Send
    struct SendMessageBinding: public MessageBinding {
        std::vector<std::string> fromAddress;
        std::vector<std::string> toAddress;
        std::vector<std::string> coins;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
            const auto &m = boost::get<cosmos::MsgSend>(msg.content);
            const auto coinsAmount = soci::coinsToString(m.amount);
            fromAddress.push_back(m.fromAddress);
            toAddress.push_back(m.toAddress);
            coins.push_back(coinsAmount);
        }

        void clear() {
            MessageBinding::clear();
            fromAddress.clear();
            toAddress.clear();
            coins.clear();

        }
    };
    const auto UPSERT_MESSAGE_SEND = db::stmt<SendMessageBinding>(
        "INSERT INTO cosmos_messages (uid,"
            "transaction_uid, message_type, log,"
            "success, msg_index, from_address, to_address, amount) "
            "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount) "
            "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex),
                soci::use(b.fromAddress), soci::use(b.toAddress), soci::use(b.coins); 
            });

    // Messages: Delegate
    struct DelegateMessageBinding: public MessageBinding {
        std::vector<std::string> delegatorAddress;
        std::vector<std::string> validatorAddress;
        std::vector<std::string> coin;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
            const auto &m = boost::get<cosmos::MsgDelegate>(msg.content);
            const auto coinAmount = soci::coinToString(m.amount);
            delegatorAddress.push_back(m.delegatorAddress);
            validatorAddress.push_back(m.validatorAddress);
            coin.push_back(coinAmount);
        }

        void clear() {
            MessageBinding::clear();
            delegatorAddress.clear();
            validatorAddress.clear();
            coin.clear();
        }
    };
    const auto UPSERT_MESSAGE_DELEGATE = db::stmt<DelegateMessageBinding>(
        "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, delegator_address, validator_address, amount) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.delegatorAddress), soci::use(b.validatorAddress), soci::use(b.coin);   
            });

    // Messages: Undelegate
    struct UndelegateMessageBinding: public MessageBinding {
        std::vector<std::string> delegatorAddress;
        std::vector<std::string> validatorAddress;
        std::vector<std::string> coin;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
            const auto &m = boost::get<cosmos::MsgUndelegate>(msg.content);
            const auto coinAmount = soci::coinToString(m.amount);
            delegatorAddress.push_back(m.delegatorAddress);
            validatorAddress.push_back(m.validatorAddress);
            coin.push_back(coinAmount);
        }

        void clear() {
            MessageBinding::clear();
            delegatorAddress.clear();
            validatorAddress.clear();
            coin.clear();
        }
    };
    const auto UPSERT_MESSAGE_UNDELEGATE = db::stmt<UndelegateMessageBinding>(
      "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, delegator_address, validator_address, amount) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.delegatorAddress), soci::use(b.validatorAddress), soci::use(b.coin);  
            });

    // Messages: BeginRedelegate
    struct BeginRedelegateMessageBinding: public MessageBinding {
        std::vector<std::string> delegatorAddress;
        std::vector<std::string> validatorSourceAddress;
        std::vector<std::string> validatorDestinationAddress;
        std::vector<std::string> coin;
        
        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
            const auto &m = boost::get<cosmos::MsgBeginRedelegate>(msg.content);
            const auto coinAmount = soci::coinToString(m.amount);
            delegatorAddress.push_back(m.delegatorAddress);
            validatorSourceAddress.push_back(m.validatorSourceAddress);
            validatorDestinationAddress.push_back(m.validatorDestinationAddress);
            coin.push_back(coinAmount);
        }

        void clear() {
            MessageBinding::clear();
            delegatorAddress.clear();
            validatorSourceAddress.clear();
            validatorDestinationAddress.clear();
            coin.clear();
        }
    };
    const auto UPSERT_MESSAGE_BEGIN_REDELEGATE = db::stmt<BeginRedelegateMessageBinding>(
       "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, delegator_address, validator_src_address,"
           "validator_dst_address, amount) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :fa, :ta, :amount) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.delegatorAddress), soci::use(b.validatorSourceAddress), soci::use(b.validatorDestinationAddress), soci::use(b.coin);   
            });
    
    // Messages: SubmitProposal
    struct SubmitProposalMessageBinding: public MessageBinding {
        std::vector<std::string> proposer;
        std::vector<std::string> contentType;
        std::vector<std::string> contentTitle;
        std::vector<std::string> contentDescr;
        std::vector<std::string> coins;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
            const auto &m = boost::get<cosmos::MsgSubmitProposal>(msg.content);
            const auto coinsAmount = soci::coinsToString(m.initialDeposit);
            proposer.push_back(m.proposer);
            contentType.push_back(m.content.type);
            contentTitle.push_back(m.content.title);
            contentDescr.push_back(m.content.descr);
            coins.push_back(coinsAmount);
        }

        void clear() {
            MessageBinding::clear();
            proposer.clear();
            contentType.clear();
            contentTitle.clear();
            contentDescr.clear();
            coins.clear();
        }
    };
    const auto UPSERT_MESSAGE_SUBMIT_PROPOSAL = db::stmt<SubmitProposalMessageBinding>(
       "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, proposer, content_type,"
           "content_title, content_description, amount) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :proposer,"
           ":ctype, :ctitle, :cdescription, :amount) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.proposer), soci::use(b.contentType), soci::use(b.contentTitle), 
                soci::use(b.contentDescr), soci::use(b.coins);
            });

    // Messages: Vote
    struct VoteMessageBinding: public MessageBinding {
        std::vector<std::string> proposalId;
        std::vector<std::string> voter;
        std::vector<std::string> option;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);

            const auto &m = boost::get<cosmos::MsgVote>(msg.content);
            proposalId.push_back(m.proposalId);
            voter.push_back(m.voter);
            option.push_back(api::to_string(m.option));
        }

        void clear() {
            MessageBinding::clear();
            proposalId.clear();
            voter.clear();
            option.clear();
        }
    };
    const auto UPSERT_MESSAGE_VOTE = db::stmt<VoteMessageBinding>(
        "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, proposal_id, voter,"
           "vote_option) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :pid, :voter, :opt) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.proposalId), soci::use(b.voter), soci::use(b.option); 
            });

    // Messages: Deposit
    struct DepositMessageBinding: public MessageBinding {
        std::vector<std::string> depositor;
        std::vector<std::string> proposalId;
        std::vector<std::string> coins;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
            const auto &m = boost::get<cosmos::MsgDeposit>(msg.content);
            const auto coinsAmount = soci::coinsToString(m.amount);
            depositor.push_back(m.depositor);
            proposalId.push_back(m.proposalId);
            coins.push_back(coinsAmount);
        }

        void clear() {
            MessageBinding::clear();
            depositor.clear();
            proposalId.clear();
            coins.clear();
        }
    };
    const auto UPSERT_MESSAGE_DEPOSIT = db::stmt<DepositMessageBinding>(
        "INSERT INTO cosmos_messages (uid, transaction_uid, "
           "message_type, log, success, "
           "msg_index, depositor, proposal_id, amount) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :dep, :pid, :amount) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.depositor), soci::use(b.proposalId), soci::use(b.coins);   
            });

    // Messages: WithdrawDelegationReward
    struct WithdrawDelegationRewardMessageBinding: public MessageBinding {
        std::vector<std::string> delegatorAddress;
        std::vector<std::string> validatorAddress;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
            const auto &m = boost::get<cosmos::MsgWithdrawDelegationReward>(msg.content);
            delegatorAddress.push_back(m.delegatorAddress);
            validatorAddress.push_back(m.validatorAddress);
        }

        void clear() {
            MessageBinding::clear();
            delegatorAddress.clear();
            validatorAddress.clear();
        }
    };
    const auto UPSERT_MESSAGE_WITHDRAW_DELEGATION_REWARD = db::stmt<WithdrawDelegationRewardMessageBinding>(
        "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, delegator_address, validator_address) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :va) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.delegatorAddress), soci::use(b.validatorAddress);   
            });

    // Messages: WithdrawDelegatorReward
    struct WithdrawDelegatorRewardMessageBinding: public MessageBinding {
        std::vector<std::string> delegatorAddress;
        std::vector<std::string> validatorAddress;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
           const auto &m = boost::get<cosmos::MsgWithdrawDelegatorReward>(msg.content);
            delegatorAddress.push_back(m.delegatorAddress);
            validatorAddress.push_back(m.validatorAddress);
        }

        void clear() {
            MessageBinding::clear();
            delegatorAddress.clear();
            validatorAddress.clear();
        }
    };
    const auto UPSERT_MESSAGE_WITHDRAW_DELEGATOR_REWARD = db::stmt<WithdrawDelegatorRewardMessageBinding>(
       "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, delegator_address, validator_address) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :va) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.delegatorAddress), soci::use(b.validatorAddress); 
            });

    // Messages: WithdrawValidatorCommission
    struct WithdrawValidatorCommissionMessageBinding: public MessageBinding {
        std::vector<std::string> validatorAddress;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            
            const auto &m = boost::get<cosmos::MsgWithdrawValidatorCommission>(msg.content);
            validatorAddress.push_back(m.validatorAddress);
        }

        void clear() {
            MessageBinding::clear();
            validatorAddress.clear();
        }
    };
    const auto UPSERT_MESSAGE_WITHDRAW_VALIDATOR_COMMISSION = db::stmt<WithdrawValidatorCommissionMessageBinding>(
        "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, validator_address) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :va) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.validatorAddress);   
            });

    // Messages: SetWithdrawAddress
    struct SetWithdrawAddressMessageBinding: public MessageBinding {
        std::vector<std::string> delegatorAddress;
        std::vector<std::string> withdrawAddress;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
    
            const auto &m = boost::get<cosmos::MsgSetWithdrawAddress>(msg.content);
            delegatorAddress.push_back(m.delegatorAddress);
            withdrawAddress.push_back(m.withdrawAddress);
        }

        void clear() {
            MessageBinding::clear();
            delegatorAddress.clear();
            withdrawAddress.clear();
        }
    };
    const auto UPSERT_MESSAGE_SET_WITHDRAW_ADDRESS = db::stmt<SetWithdrawAddressMessageBinding>(
       "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, delegator_address, to_address) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :withdraw) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.delegatorAddress), soci::use(b.withdrawAddress);   
            });

    // Messages: Unjail
    struct UnjailMessageBinding: public MessageBinding {
        std::vector<std::string> validatorAddress;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
    
            const auto &m = boost::get<cosmos::MsgUnjail>(msg.content);
            validatorAddress.push_back(m.validatorAddress);
        }

        void clear() {
            MessageBinding::clear();
            validatorAddress.clear();
        }
    };
    const auto UPSERT_MESSAGE_UNJAIL = db::stmt<UnjailMessageBinding>(
       "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, validator_address) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :va) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.validatorAddress); 
            });

    // Messages: MultiSend
    struct MultiSendMessageBinding: public MessageBinding {
        std::vector<std::string> coins;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
            const auto &m = boost::get<cosmos::MsgMultiSend>(msg.content);
            std::vector<cosmos::Coin> totalAmount;

            // This snippet left-folds a iterable<vector<cosmos::Coin>>
            // into vector<cosmos::Coin> using addition.
            // Hopefully transform_reduce or flatten functions can help us later.
            // The map intermediate helps for out-of-order denominations.
            std::unordered_map<std::string, BigInt> denomToAmt;
            std::for_each(m.inputs.cbegin(), m.inputs.cend(), [&denomToAmt](const auto &input) {
                std::for_each(
                    input.coins.cbegin(), input.coins.cend(), [&denomToAmt](const auto &amountDenom) {
                        auto searchDenom = denomToAmt.find(amountDenom.denom);
                        if (searchDenom == denomToAmt.end()) {
                            denomToAmt.insert({amountDenom.denom, BigInt::fromString(amountDenom.amount)});
                            return;
                        }
                        searchDenom->second = searchDenom->second + BigInt::fromString(amountDenom.amount);
                    });
            });
            // Add each pair of denomToAmt into totalAmount
            totalAmount.reserve(denomToAmt.size());
            for (const auto &pair : denomToAmt) {
                totalAmount.emplace_back(pair.second.toString(), pair.first);
            }

            // Insert the global message information in cosmos_messages
            const auto coinsAmount = soci::coinsToString(totalAmount);
            coins.push_back(coinsAmount);
        }

        void clear() {
            MessageBinding::clear();
            coins.clear();
        }
    };
    const auto UPSERT_MESSAGE_MULTISEND = db::stmt<MultiSendMessageBinding>(
        "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, amount) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :amount) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex),
                soci::use(b.coins);
            });

    // Messages: Fees
    struct FeesMessageBinding: public MessageBinding {
        std::vector<std::string> payerAddress;
        std::vector<std::string> fees;

        void update(std::string const &transactionUid, cosmos::Message const &msg, cosmos::MessageLog const &messageLog) {
            MessageBinding::update(transactionUid, msg, messageLog);
    
            const auto &m = boost::get<cosmos::MsgFees>(msg.content);
            const auto &f = soci::coinToString(m.fees);
            payerAddress.push_back(m.payerAddress);
            fees.push_back(f);
        }

        void clear() {
            MessageBinding::clear();
            payerAddress.clear();
            fees.clear();
        }
    };
    const auto UPSERT_MESSAGE_FEES = db::stmt<FeesMessageBinding>(
        "INSERT INTO cosmos_messages (uid,"
           "transaction_uid, message_type, log,"
           "success, msg_index, from_address, amount) "
           "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :amount) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.txUid), soci::use(b.type), 
                soci::use(b.log), soci::use(b.success), soci::use(b.messageIndex), 
                soci::use(b.payerAddress), soci::use(b.fees);   
            });

    struct MultiSendInputBinding {
        std::vector<std::string> uid;
        std::vector<std::string> fromAddress;
        std::vector<std::string> inputCoins;
        
        void update(cosmos::Message const &msg) {
            const auto &m = boost::get<cosmos::MsgMultiSend>(msg.content);
            for (const auto &input: m.inputs) {    
                const auto coins = soci::coinsToString(input.coins);
                uid.push_back(msg.uid);
                fromAddress.push_back(input.fromAddress);
                inputCoins.push_back(coins);
            }      
        }

        void clear() {
            uid.clear();
            fromAddress.clear();
            inputCoins.clear();
        }

        bool empty() const {
            return uid.empty();
        }
    };
    const auto UPSERT_MULTISEND_INPUT = db::stmt<MultiSendInputBinding>(
        "INSERT INTO cosmos_multisend_io (message_uid, from_address, amount) "
            "VALUES (:uid, :fa, :amt) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.fromAddress), soci::use(b.inputCoins);  
            });

    struct MultiSendOutputBinding {
        std::vector<std::string> uid;
        std::vector<std::string> toAddress;
        std::vector<std::string> outputCoins;
        
        void update(cosmos::Message const &msg) {
            const auto &m = boost::get<cosmos::MsgMultiSend>(msg.content);
            for (const auto &output: m.outputs) {
                const auto coins = soci::coinsToString(output.coins);
                uid.push_back(msg.uid);
                toAddress.push_back(output.toAddress);
                outputCoins.push_back(coins);
            }          
        }

        void clear() {
            uid.clear();
            toAddress.clear();
            outputCoins.clear();
        }

        bool empty() const {
            return uid.empty();
        }
    };
    const auto UPSERT_MULTISEND_OUTPUT = db::stmt<MultiSendOutputBinding>(
        "INSERT INTO cosmos_multisend_io (message_uid, to_address, amount) "
            "VALUES (:uid, :ta, :amt) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                soci::use(b.uid), soci::use(b.toAddress), soci::use(b.outputCoins);  
            });
}

namespace ledger {
    namespace core {
        void CosmosLikeOperationsDatabaseHelper::bulkInsert(soci::session &sql,
                const std::vector<CosmosLikeOperation> &operations) {
            if (operations.empty())
                return;
            Benchmarker rawInsert("raw_db_insert_cosmos", nullptr);
            rawInsert.start();

            PreparedStatement<OperationBinding> operationStmt;
            PreparedStatement<BlockBinding> blockStmt;
            PreparedStatement<CosmosOperationBinding> cosmosOpStmt;
            PreparedStatement<TransactionBinding> transactionStmt;
            PreparedStatement<MessageBinding> genericMessageStmt;
            PreparedStatement<SendMessageBinding> sendMessageStmt;
            PreparedStatement<DelegateMessageBinding> delegateMessageStmt;
            PreparedStatement<UndelegateMessageBinding> undelegateMessageStmt;
            PreparedStatement<BeginRedelegateMessageBinding> beginRedelegateMessageStmt;
            PreparedStatement<SubmitProposalMessageBinding> submitProposalMessageStmt;
            PreparedStatement<VoteMessageBinding> voteMessageStmt;
            PreparedStatement<DepositMessageBinding> depositMessageStmt;
            PreparedStatement<WithdrawDelegationRewardMessageBinding> withdrawDelegationRewardMessageStmt;
            PreparedStatement<WithdrawDelegatorRewardMessageBinding> withdrawDelegatorRewardMessageStmt;
            PreparedStatement<WithdrawValidatorCommissionMessageBinding> withdrawValidatorCommissionMessageStmt;
            PreparedStatement<SetWithdrawAddressMessageBinding> setWithdrawAddressMessageStmt;
            PreparedStatement<UnjailMessageBinding> unjailMessageStmt;
            PreparedStatement<MultiSendMessageBinding> multiSendMessageStmt;
            PreparedStatement<FeesMessageBinding> feesMessageStmt;
            PreparedStatement<MultiSendInputBinding> multisendInStmt;
            PreparedStatement<MultiSendOutputBinding> multisendOutStmt;

            BulkInsertDatabaseHelper::UPSERT_OPERATION(sql, operationStmt);
            BulkInsertDatabaseHelper::UPSERT_BLOCK(sql, blockStmt);
            UPSERT_COSMOS_OPERATION(sql, cosmosOpStmt);
            UPSERT_TRANSACTION(sql, transactionStmt);
            UPSERT_MESSAGE_GENERIC(sql, genericMessageStmt);
            UPSERT_MESSAGE_SEND(sql, sendMessageStmt);
            UPSERT_MESSAGE_DELEGATE(sql, delegateMessageStmt);
            UPSERT_MESSAGE_UNDELEGATE(sql, undelegateMessageStmt);
            UPSERT_MESSAGE_BEGIN_REDELEGATE(sql, beginRedelegateMessageStmt);
            UPSERT_MESSAGE_SUBMIT_PROPOSAL(sql, submitProposalMessageStmt);
            UPSERT_MESSAGE_VOTE(sql, voteMessageStmt);
            UPSERT_MESSAGE_DEPOSIT(sql, depositMessageStmt);
            UPSERT_MESSAGE_WITHDRAW_DELEGATION_REWARD(sql, withdrawDelegationRewardMessageStmt);
            UPSERT_MESSAGE_WITHDRAW_DELEGATOR_REWARD(sql, withdrawDelegatorRewardMessageStmt);
            UPSERT_MESSAGE_WITHDRAW_VALIDATOR_COMMISSION(sql, withdrawValidatorCommissionMessageStmt);
            UPSERT_MESSAGE_SET_WITHDRAW_ADDRESS(sql, setWithdrawAddressMessageStmt);
            UPSERT_MESSAGE_UNJAIL(sql, unjailMessageStmt);
            UPSERT_MESSAGE_MULTISEND(sql, multiSendMessageStmt);
            UPSERT_MESSAGE_FEES(sql, feesMessageStmt);
            UPSERT_MULTISEND_INPUT(sql, multisendInStmt);
            UPSERT_MULTISEND_OUTPUT(sql, multisendOutStmt);

            for (auto& op : operations) {
                const auto& tx = std::static_pointer_cast<CosmosLikeTransactionApi>(op.getTransaction())->getRawData();

                // Upsert block
                if (tx.block.nonEmpty()) {
                    blockStmt.bindings.update(tx.block.getValue());
                }
                // Upsert operation
                operationStmt.bindings.update(op);
                // Upsert transaction    
                transactionStmt.bindings.update(tx);
                // messages
                const auto& msg = std::static_pointer_cast<CosmosLikeMessage>(
                    op.getMessage())->getRawData();
                const auto& log = op.getMessageLog();

                switch (cosmos::stringToMsgType(msg.type.c_str())) {
                    case api::CosmosLikeMsgType::MSGSEND: {
                        sendMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGDELEGATE: {
                        delegateMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGUNDELEGATE: {
                        undelegateMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGBEGINREDELEGATE: {
                        beginRedelegateMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL: {
                        submitProposalMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGVOTE: {
                        voteMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGDEPOSIT: {
                        depositMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD: {
                        withdrawDelegationRewardMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD: {
                        withdrawDelegatorRewardMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION: {
                        withdrawValidatorCommissionMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS: {
                        setWithdrawAddressMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGUNJAIL: {
                        unjailMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGMULTISEND: {
                        multiSendMessageStmt.bindings.update(tx.uid, msg, log);
                        multisendInStmt.bindings.update(msg);
                        multisendOutStmt.bindings.update(msg);
                    } break;
                    case api::CosmosLikeMsgType::MSGFEES: {
                        feesMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                    case api::CosmosLikeMsgType::MSGCREATEVALIDATOR:
                    case api::CosmosLikeMsgType::MSGEDITVALIDATOR:
                    case api::CosmosLikeMsgType::UNSUPPORTED:
                    default: {
                        genericMessageStmt.bindings.update(tx.uid, msg, log);
                    } break;
                }

                // cosmos operation
                cosmosOpStmt.bindings.update(op.uid, msg.uid);
            }
            
            // 1- block
            if (!blockStmt.bindings.uid.empty()) {
                blockStmt.execute();
            }

            // 2- cosmos_transaction 
            transactionStmt.execute();

            // 3- cosmos_messages
            if (!sendMessageStmt.bindings.empty()) {
                sendMessageStmt.execute();
            }
            if (!delegateMessageStmt.bindings.empty()) {
                delegateMessageStmt.execute();
            }
            if (!undelegateMessageStmt.bindings.empty()) {
                undelegateMessageStmt.execute();
            }
            if (!beginRedelegateMessageStmt.bindings.empty()) {
                beginRedelegateMessageStmt.execute();
            }
            if (!submitProposalMessageStmt.bindings.empty()) {
                submitProposalMessageStmt.execute();
            }
            if (!voteMessageStmt.bindings.empty()) {
                voteMessageStmt.execute();
            }
            if (!depositMessageStmt.bindings.empty()) {
                depositMessageStmt.execute();
            }
            if (!withdrawDelegationRewardMessageStmt.bindings.empty()) {
                withdrawDelegationRewardMessageStmt.execute();
            }
            if (!withdrawDelegatorRewardMessageStmt.bindings.empty()) {
                withdrawDelegatorRewardMessageStmt.execute();
            }
            if (!withdrawValidatorCommissionMessageStmt.bindings.empty()) {
                withdrawValidatorCommissionMessageStmt.execute();
            }
            if (!setWithdrawAddressMessageStmt.bindings.empty()) {
                setWithdrawAddressMessageStmt.execute();
            }
            if (!unjailMessageStmt.bindings.empty()) {
                unjailMessageStmt.execute();
            }
            if (!multiSendMessageStmt.bindings.empty()) {
                multiSendMessageStmt.execute();
            }
            if (!feesMessageStmt.bindings.empty()) {
                feesMessageStmt.execute();
            }
            if (!genericMessageStmt.bindings.empty()) {
                genericMessageStmt.execute();
            }

            // 4- multisend_io
            if (!multisendInStmt.bindings.empty()) {
                multisendInStmt.execute();
            }
            if (!multisendOutStmt.bindings.empty()) {
                multisendOutStmt.execute();
            }

            // 5- operations
            operationStmt.execute();
            
            // 6- cosmos_operations
            cosmosOpStmt.execute();

            rawInsert.stop();
        }
    }
}