/*
 *
 * CosmosLikeTransactionDatabaseHelper
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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


#include <wallet/cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>

#include <database/soci-option.h>
#include <database/soci-date.h>
#include <database/soci-number.h>
#include <crypto/SHA256.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>

#include <wallet/cosmos/database/SociCosmosAmount.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>

#include <api/enum_from_string.hpp>

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <unordered_map>

#include <utils/Exception.hpp>

namespace ledger {
    namespace core {

        static void inflateMessage(soci::session& sql, soci::row const& row, cosmos::Message& msg) {
            // See Migrations.cpp for column numbers
            const auto COL_UID = 0;
            const auto COL_TXUID = 1;
            const auto COL_MSGTYPE = 2;
            const auto COL_LOG = 3;
            const auto COL_SUCCESS = 4;
            const auto COL_MSGINDEX = 5;
            const auto COL_FROMADDR = 6;
            const auto COL_TOADDR = 7;
            const auto COL_AMOUNT = 8;
            const auto COL_DELADDR = 9;
            const auto COL_VALADDR = 10;
            const auto COL_VALSRCADDR = 11;
            const auto COL_VALDESTADDR = 12;
            const auto COL_PROPTYPE = 13;
            const auto COL_PROPTITLE = 14;
            const auto COL_PROPDESC = 15;
            const auto COL_PROPOSER = 16;
            const auto COL_VOTER = 17;
            const auto COL_PROPID = 18;
            const auto COL_VOTEOPT = 19;
            const auto COL_DEPOSITOR = 20;
            msg.uid = row.get<std::string>(COL_UID);
            auto msgType = row.get<std::string>(COL_MSGTYPE);
            msg.type = msgType;
            switch (cosmos::stringToMsgType(msgType.c_str())) {
                case api::CosmosLikeMsgType::MSGSEND:
                    {
                        msg.content = cosmos::MsgSend();
                        auto &content = boost::get<cosmos::MsgSend>(msg.content);
                        content.fromAddress = row.get<std::string>(COL_FROMADDR);
                        content.toAddress = row.get<std::string>(COL_TOADDR);
                        soci::stringToCoins(row.get<std::string>(COL_AMOUNT), content.amount);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGDELEGATE:
                    {
                        msg.content = cosmos::MsgDelegate();
                        auto &content = boost::get<cosmos::MsgDelegate>(msg.content);
                        content.delegatorAddress = row.get<std::string>(COL_DELADDR);
                        content.validatorAddress = row.get<std::string>(COL_VALADDR);
                        soci::stringToCoin(row.get<std::string>(COL_AMOUNT), content.amount);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGUNDELEGATE:
                    {
                        msg.content = cosmos::MsgUndelegate();
                        auto &content = boost::get<cosmos::MsgUndelegate>(msg.content);
                        content.delegatorAddress = row.get<std::string>(COL_DELADDR);
                        content.validatorAddress = row.get<std::string>(COL_VALADDR);
                        soci::stringToCoin(row.get<std::string>(COL_AMOUNT), content.amount);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGBEGINREDELEGATE:
                    {
                        msg.content = cosmos::MsgBeginRedelegate();
                        auto &content = boost::get<cosmos::MsgBeginRedelegate>(msg.content);
                        content.delegatorAddress = row.get<std::string>(COL_DELADDR);
                        content.validatorSourceAddress = row.get<std::string>(COL_VALSRCADDR);
                        content.validatorDestinationAddress = row.get<std::string>(COL_VALDESTADDR);
                        soci::stringToCoin(row.get<std::string>(COL_AMOUNT), content.amount);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL:
                    {
                        msg.content = cosmos::MsgSubmitProposal();
                        auto &content = boost::get<cosmos::MsgSubmitProposal>(msg.content);
                        content.content.type = row.get<std::string>(COL_PROPTYPE);
                        content.content.title = row.get<std::string>(COL_PROPTITLE);
                        content.content.description = row.get<std::string>(COL_PROPDESC);
                        content.proposer = row.get<std::string>(COL_PROPOSER);
                        soci::stringToCoins(row.get<std::string>(COL_AMOUNT), content.initialDeposit);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGVOTE:
                    {
                        msg.content = cosmos::MsgVote();
                        auto &content = boost::get<cosmos::MsgVote>(msg.content);
                        content.voter = row.get<std::string>(COL_VOTER);
                        content.proposalId = row.get<std::string>(COL_PROPID);
                        content.option = api::from_string<api::CosmosLikeVoteOption>(row.get<std::string>(COL_VOTEOPT));
                    }
                    break;
                case api::CosmosLikeMsgType::MSGDEPOSIT:
                    {
                        msg.content = cosmos::MsgDeposit();
                        auto &content = boost::get<cosmos::MsgDeposit>(msg.content);
                        content.depositor = row.get<std::string>(COL_DEPOSITOR);
                        content.proposalId = row.get<std::string>(COL_PROPID);
                        soci::stringToCoins(row.get<std::string>(COL_AMOUNT), content.amount);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD:
                    {
                        msg.content = cosmos::MsgWithdrawDelegationReward();
                        auto &content = boost::get<cosmos::MsgWithdrawDelegationReward>(msg.content);
                        content.delegatorAddress = row.get<std::string>(COL_DELADDR);
                        content.validatorAddress = row.get<std::string>(COL_VALADDR);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD:
                    {
                        msg.content = cosmos::MsgWithdrawDelegatorReward();
                        auto &content = boost::get<cosmos::MsgWithdrawDelegatorReward>(msg.content);
                        content.delegatorAddress = row.get<std::string>(COL_DELADDR);
                        content.validatorAddress = row.get<std::string>(COL_VALADDR);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION:
                    {
                        msg.content = cosmos::MsgWithdrawValidatorCommission();
                        auto &content = boost::get<cosmos::MsgWithdrawValidatorCommission>(msg.content);
                        content.validatorAddress = row.get<std::string>(COL_VALADDR);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS:
                    {
                        msg.content = cosmos::MsgSetWithdrawAddress();
                        auto &content = boost::get<cosmos::MsgSetWithdrawAddress>(msg.content);
                        content.delegatorAddress = row.get<std::string>(COL_DELADDR);
                        content.withdrawAddress = row.get<std::string>(COL_TOADDR);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGUNJAIL:
                    {
                        msg.content = cosmos::MsgUnjail();
                        auto &content = boost::get<cosmos::MsgUnjail>(msg.content);
                        content.validatorAddress = row.get<std::string>(COL_VALADDR);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGMULTISEND:
                    {
                        msg.content = cosmos::MsgMultiSend();
                        auto &content = boost::get<cosmos::MsgMultiSend>(msg.content);
                        // Make a join on cosmos_multisend_io to fill the message
                        soci::rowset<soci::row> multiSendRows = (sql.prepare <<
                                "SELECT * FROM cosmos_multisend_io WHERE message_uid = :msgUid",
                                soci::use(msg.uid));
                        // See migrations.cpp for column names
                        const auto COL_MULTI_FROM = 1;
                        const auto COL_MULTI_TO = 2;
                        const auto COL_MULTI_AMT = 3;
                        for (auto &multiSendRow : multiSendRows) {
                            const auto inputAddress = multiSendRow.get<Option<std::string>>(COL_MULTI_FROM).getValueOr("");
                            const auto outputAddress = multiSendRow.get<Option<std::string>>(COL_MULTI_TO).getValueOr("");
                            if (!inputAddress.empty()) {
                                cosmos::MultiSendInput newInput;
                                newInput.fromAddress = inputAddress;
                                soci::stringToCoins(multiSendRow.get<std::string>(COL_MULTI_AMT), newInput.coins);
                                content.inputs.push_back(newInput);
                            } else if (!outputAddress.empty()) {
                                cosmos::MultiSendOutput newOutput;
                                newOutput.toAddress = outputAddress;
                                soci::stringToCoins(multiSendRow.get<std::string>(COL_MULTI_AMT), newOutput.coins);
                                content.outputs.push_back(newOutput);
                            }
                        }
                    }
                    break;
                case api::CosmosLikeMsgType::MSGFEES:
                    {
                        msg.content = cosmos::MsgFees();
                        auto& content = boost::get<cosmos::MsgFees>(msg.content);
                        content.payerAddress = row.get<std::string>(COL_FROMADDR);
                        soci::stringToCoin(row.get<std::string>(COL_AMOUNT), content.fees);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGEDITVALIDATOR:
                case api::CosmosLikeMsgType::MSGCREATEVALIDATOR:
                    {
                    throw make_exception(
                        api::ErrorCode::UNSUPPORTED_OPERATION,
                        "inflate MsgCreateValidator / MsgEditValidator");
                    }
                    break;
                case api::CosmosLikeMsgType::UNSUPPORTED:
                default:
                    {
                        msg.content = cosmos::MsgUnsupported();
                    }
                    break;
            }
        }

        static void inflateTransaction(soci::session &sql,
                                       const soci::row &row,
                                       cosmos::Transaction &tx) {

            // See CosmosLikeTransactionDatabaseHelper::getTransactionByHash for column names.
            const auto COL_TX_UID = 0;
            const auto COL_TX_HASH = 1;
            const auto COL_TX_TIME = 2;
            const auto COL_TX_FEE = 3;
            const auto COL_TX_GAS = 4;
            const auto COL_TX_GASUSED = 5;
            const auto COL_TX_MEMO = 6;
            const auto COL_TX_BLOCKUID = 7;
            const auto COL_BLK_HASH = 8;
            const auto COL_BLK_HEIGHT = 9;
            const auto COL_BLK_TIME = 10;
            const auto COL_BLK_CURRNAME = 11;

            tx.uid = row.get<std::string>(COL_TX_UID);
            tx.hash = row.get<std::string>(COL_TX_HASH);
            tx.timestamp = row.get<std::chrono::system_clock::time_point>(COL_TX_TIME);
            soci::stringToCoins(row.get<std::string>(COL_TX_FEE), tx.fee.amount);
            tx.fee.gas = row.get<std::string>(COL_TX_GAS);
            tx.gasUsed = row.get<Option<std::string>>(COL_TX_GASUSED).map<BigInt>([] (const std::string& v) {
                return BigInt::fromString(v);
            });
            tx.memo = row.get<Option<std::string>>(COL_TX_MEMO).getValueOr("");

            if (row.get_indicator(COL_TX_BLOCKUID) != soci::i_null) {
                ledger::core::api::Block block;
                block.uid = row.get<std::string>(COL_TX_BLOCKUID);
                block.blockHash = row.get<std::string>(COL_BLK_HASH);
                block.height = soci::get_number<uint64_t>(row, COL_BLK_HEIGHT);
                block.time = row.get<std::chrono::system_clock::time_point>(COL_BLK_TIME);
                block.currencyName = row.get<std::string>(COL_BLK_CURRNAME);
                tx.block = block;
            }

            soci::rowset<soci::row> msgRows = (sql.prepare <<
                    "SELECT * FROM cosmos_messages WHERE transaction_uid = :txUid ORDER BY cosmos_messages.msg_index",
                    soci::use(tx.uid));


                auto msgCount = 0;
            for (auto &msgRow : msgRows) {
                msgCount++;
                // See Migrations.cpp for column names
                const auto COL_MSG_LOG = 3;
                const auto COL_MSG_SUCCESS = 4;
                const auto COL_MSG_INDEX = 5;
                cosmos::Message msg;
                inflateMessage(sql, msgRow, msg);
                tx.messages.push_back(msg);

                cosmos::MessageLog log;
                log.log = msgRow.get<std::string>(COL_MSG_LOG);
                log.success = soci::get_number<int32_t>(msgRow, COL_MSG_SUCCESS) != 0;
                log.messageIndex = soci::get_number<int32_t>(msgRow, COL_MSG_INDEX);
                tx.logs.push_back(log);
            }
        }

        static void insertMessage(soci::session& sql,
                                  std::string const& txUid,
                                  cosmos::Message const& msg,
                                  cosmos::MessageLog const& log) {

            switch (cosmos::stringToMsgType(msg.type.c_str())) {
                case api::CosmosLikeMsgType::MSGSEND:
                    {
                        const auto& m = boost::get<cosmos::MsgSend>(msg.content);
                        const auto coins = soci::coinsToString(m.amount);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, from_address, to_address, amount) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount)",
                               soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                               soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                               soci::use(m.fromAddress), soci::use(m.toAddress), soci::use(coins);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGDELEGATE:
                    {
                        const auto& m = boost::get<cosmos::MsgDelegate>(msg.content);
                        const auto coin = soci::coinToString(m.amount);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, delegator_address, validator_address, amount) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.delegatorAddress), soci::use(m.validatorAddress), soci::use(coin);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGUNDELEGATE:
                    {
                        const auto& m = boost::get<cosmos::MsgUndelegate>(msg.content);
                        const auto coin = soci::coinToString(m.amount);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, delegator_address, validator_address, amount) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.delegatorAddress), soci::use(m.validatorAddress), soci::use(coin);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGBEGINREDELEGATE:
                    {
                        const auto& m = boost::get<cosmos::MsgBeginRedelegate>(msg.content);
                        const auto coin = soci::coinToString(m.amount);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, delegator_address, validator_src_address,"
                               "validator_dst_address, amount) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :fa, :ta, :amount)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.delegatorAddress), soci::use(m.validatorSourceAddress),
                                soci::use(m.validatorDestinationAddress), soci::use(coin);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL:
                    {
                        const auto& m = boost::get<cosmos::MsgSubmitProposal>(msg.content);
                        const auto coins = soci::coinsToString(m.initialDeposit);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, proposer, content_type,"
                               "content_title, content_description, amount) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :proposer,"
                               ":ctype, :ctitle, :cdescription, :amount)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.proposer), soci::use(m.content.type), soci::use(m.content.title),
                                soci::use(m.content.description), soci::use(coins);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGVOTE:
                    {
                        const auto& m = boost::get<cosmos::MsgVote>(msg.content);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, proposal_id, voter,"
                               "vote_option) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :pid, :voter, :opt)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.proposalId), soci::use(m.voter),
                                soci::use(api::to_string(m.option));
                    }
                    break;
                case api::CosmosLikeMsgType::MSGDEPOSIT:
                    {
                        const auto& m = boost::get<cosmos::MsgDeposit>(msg.content);
                        const auto coins = soci::coinsToString(m.amount);
                        sql << "INSERT INTO cosmos_messages (uid, transaction_uid, "
                               "message_type, log, success, "
                               "msg_index, depositor, proposal_id, amount) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :dep, :pid, :amount)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.depositor), soci::use(m.proposalId), soci::use(coins);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD:
                    {
                        const auto &m = boost::get<cosmos::MsgWithdrawDelegationReward>(msg.content);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, delegator_address, validator_address) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :va)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type),
                                soci::use(log.log), soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.delegatorAddress), soci::use(m.validatorAddress);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD:
                    {
                        const auto &m = boost::get<cosmos::MsgWithdrawDelegatorReward>(msg.content);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, delegator_address, validator_address) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :va)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type),
                                soci::use(log.log), soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.delegatorAddress), soci::use(m.validatorAddress);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION:
                    {
                        const auto &m = boost::get<cosmos::MsgWithdrawValidatorCommission>(msg.content);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, validator_address) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :va)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type),
                                soci::use(log.log), soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.validatorAddress);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS:
                    {
                        const auto &m = boost::get<cosmos::MsgSetWithdrawAddress>(msg.content);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, delegator_address, to_address) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :withdraw)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type),
                                soci::use(log.log), soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.delegatorAddress), soci::use(m.withdrawAddress);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGUNJAIL:
                    {
                        const auto &m = boost::get<cosmos::MsgUnjail>(msg.content);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, validator_address) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :va)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type),
                                soci::use(log.log), soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                                soci::use(m.validatorAddress);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGMULTISEND:
                    {
                        const auto& m = boost::get<cosmos::MsgMultiSend>(msg.content);
                        std::vector<cosmos::Coin> totalAmount;

                        // HACK : this snippet left-folds a iterable<vector<cosmos::Coin>>
                        // into vector<cosmos::Coin> using addition.
                        // Hopefully transform_reduce or flatten functions can help us later.
                        // The map intermediate helps for out-of-order denominations.
                        std::unordered_map<std::string, BigInt> denomToAmt;
                        std::for_each(
                            m.inputs.cbegin(),
                            m.inputs.cend(),
                            [&denomToAmt](const auto &input) {
                                std::for_each(
                                    input.coins.cbegin(),
                                    input.coins.cend(),
                                    [&denomToAmt](const auto &amountDenom) {
                                        auto searchDenom = denomToAmt.find(amountDenom.denom);
                                        if (searchDenom == denomToAmt.end()) {
                                            denomToAmt.insert(
                                                {amountDenom.denom,
                                                 BigInt::fromString(amountDenom.amount)});
                                            return;
                                        }
                                        searchDenom->second =
                                            searchDenom->second +
                                            BigInt::fromString(amountDenom.amount);
                                    });
                            });
                        // Add each pair of denomToAmt into totalAmount
                        totalAmount.reserve(denomToAmt.size());
                        for (const auto &pair : denomToAmt) {
                            totalAmount.emplace_back(pair.second.toString(),pair.first);
                        }

                        // Insert the global message information in cosmos_messages
                        const auto coins = soci::coinsToString(totalAmount);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, amount) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :amount)",
                               soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                               soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                               soci::use(coins);

                        // Insert each input and each output to the cosmos_multisend_io table
                        std::for_each(
                            m.inputs.cbegin(),
                            m.inputs.cend(),
                            [&sql, &msg] (const auto& input) {
                                const auto inputCoins = soci::coinsToString(input.coins);
                                sql << "INSERT INTO cosmos_multisend_io (uid, from_address, amount) "
                                    "VALUES (:uid, :fa, :amt)",
                                    soci::use(msg.uid), soci::use(input.fromAddress), soci::use(inputCoins);
                            });

                        std::for_each(
                            m.outputs.cbegin(),
                            m.outputs.cend(),
                            [&sql, &msg] (const auto& output) {
                                const auto outputCoins = soci::coinsToString(output.coins);
                                sql << "INSERT INTO cosmos_multisend_io (uid, to_address, amount) "
                                    "VALUES (:uid, :ta, :amt)",
                                    soci::use(msg.uid), soci::use(output.toAddress), soci::use(outputCoins);
                            });
                    }
                    break;
                case api::CosmosLikeMsgType::MSGFEES:
                    {
                        const auto& m = boost::get<cosmos::MsgFees>(msg.content);
                        const auto& fees = soci::coinToString(m.fees);
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, log,"
                               "success, msg_index, from_address, amount) "
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :amount)",
                               soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                               soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                               soci::use(m.payerAddress), soci::use(fees);
                    }
                    break;
                case api::CosmosLikeMsgType::MSGCREATEVALIDATOR:
                case api::CosmosLikeMsgType::MSGEDITVALIDATOR:
                case api::CosmosLikeMsgType::UNSUPPORTED:
                default:
                    {
                        // Do record the message type, even if unsupported
                        sql << "INSERT INTO cosmos_messages (uid,"
                               "transaction_uid, message_type, "
                               "log, success, msg_index)"
                               "VALUES (:uid, :tuid, :mt, :log, :success, :mi)",
                                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type),
                                soci::use(log.log), soci::use(log.success ? 1 : 0), soci::use(log.messageIndex);
                    }
                    break;
            }
        }

        static void insertTransaction(soci::session& sql, cosmos::Transaction const& tx) {
            Option<std::string> blockUid;
            if (tx.block.nonEmpty() && !tx.block.getValue().hash.empty()) {
                blockUid = BlockDatabaseHelper::createBlockUid(tx.block.getValue());
            }

            auto date = DateUtils::toJSON(tx.timestamp);
            auto fee = soci::coinsToString(tx.fee.amount);
            auto gas = tx.fee.gas.toString();
            auto gasUsed = tx.gasUsed.flatMap<std::string>([] (const BigInt& g) {
                return g.toString();
            });
            sql << "INSERT INTO cosmos_transactions("
                   "uid, hash, block_uid, time, fee_amount, gas, gas_used, memo"
                   ") VALUES (:uid, :hash, :buid, :time, :fee, :gas, :gas_used, :memo)",
                    soci::use(tx.uid), soci::use(tx.hash), soci::use(blockUid), soci::use(date),
                    soci::use(fee), soci::use(gas), soci::use(gasUsed), soci::use(tx.memo);
        }

        static std::string createCosmosMessageUid(std::string const& txUid, uint64_t msgIndex) {
            auto result = SHA256::stringToHexHash(fmt::format("uid:{}+{}", txUid, msgIndex));
            return result;
        }

        static std::string createCosmosTransactionUid(std::string const& accountUid, std::string const& txHash) {
            auto result = SHA256::stringToHexHash(fmt::format("uid:{}+{}", accountUid, txHash));
            return result;
        }

        bool CosmosLikeTransactionDatabaseHelper::transactionExists(soci::session &sql, const std::string &txUid) {
            int32_t count = 0;
            sql << "SELECT COUNT(*) FROM cosmos_transactions WHERE uid = :txUid", soci::use(txUid), soci::into(count);
            return count == 1;
        }

        void CosmosLikeTransactionDatabaseHelper::putTransaction(soci::session &sql,
                                                                 const std::string &accountUid,
                                                                 cosmos::Transaction &tx) {

            tx.uid = createCosmosTransactionUid(accountUid, tx.hash);

            if (transactionExists(sql, tx.uid)) {

                // UPDATE (we only update block information and gasUsed)
                if (tx.block.nonEmpty() && tx.block.getValue().hash.size() > 0) {
                    auto blockUid = tx.block.map<std::string>([](const cosmos::Block &block) {
                        return block.getUid();
                    });

                    auto gasUsed = tx.gasUsed.flatMap<std::string>([] (const BigInt& g) {
                        return g.toString();
                    });

                    sql << "UPDATE cosmos_transactions SET block_uid = :uid, gas_used = :gas_used "
                           "WHERE hash = :tx_hash",
                            soci::use(blockUid), soci::use(gasUsed), soci::use(tx.hash);
                }

            } else {

                // Insert block
                if (tx.block.nonEmpty() && !tx.block.getValue().hash.empty()) {
                    BlockDatabaseHelper::putBlock(sql, tx.block.getValue());
                }

                // Insert transaction
                insertTransaction(sql, tx);

                // Insert messages
                for (auto index = 0; index < tx.messages.size(); index++) {
                    auto& msg = tx.messages[index];
                    auto& log = tx.logs[index];

                    msg.uid = createCosmosMessageUid(tx.uid, index);

                    insertMessage(sql, tx.uid, msg, log);
                }
                /// There is nothing to do to handle the internal fee message
                /// as it is stored in the transaction.
            }
        }

        bool CosmosLikeTransactionDatabaseHelper::getTransactionByHash(soci::session &sql,
                                                                       const std::string &txHash,
                                                                       cosmos::Transaction &tx) {
            soci::rowset<soci::row> rows = (sql.prepare << "SELECT tx.uid, tx.hash, tx.time, "
                    "tx.fee_amount, tx.gas, tx.gas_used, tx.memo, tx.block_uid, "
                    "block.hash, block.height, block.time, block.currency_name "
                    "FROM cosmos_transactions AS tx "
                    "LEFT JOIN blocks AS block ON tx.block_uid = block.uid "
                    "WHERE tx.hash = :hash ", soci::use(txHash));

            for (auto &row : rows) {
                inflateTransaction(sql, row, tx);
                return true;
            }

            return false;
        }

        bool CosmosLikeTransactionDatabaseHelper::getMessageByUid(soci::session &sql,
                                                                  const std::string &msgUid,
                                                                  cosmos::Message &msg) {
            soci::rowset<soci::row> rows = (sql.prepare <<
                    "SELECT * FROM cosmos_messages WHERE uid = :msgUid ",
                    soci::use(msgUid));

            for (auto &row : rows) {
                inflateMessage(sql, row, msg);
                return true;
            }

            return false;
        }

    }
}
