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

        // Migration to modularized : move the namespaces in the cosmos migrations header
        /// Helper names on column numbers in cosmos_messages table.
        /// See Migrations.cpp file for the source of truth.
        namespace {
            const constexpr int COL_MSG_UID = 0;
            const constexpr int COL_MSG_TXUID = 1;
            const constexpr int COL_MSG_MSGTYPE = 2;
            const constexpr int COL_MSG_LOG = 3;
            const constexpr int COL_MSG_SUCCESS = 4;
            const constexpr int COL_MSG_MSGINDEX = 5;
            const constexpr int COL_MSG_FROMADDR = 6;
            const constexpr int COL_MSG_TOADDR = 7;
            const constexpr int COL_MSG_AMOUNT = 8;
            const constexpr int COL_MSG_DELADDR = 9;
            const constexpr int COL_MSG_VALADDR = 10;
            const constexpr int COL_MSG_VALSRCADDR = 11;
            const constexpr int COL_MSG_VALDESTADDR = 12;
            const constexpr int COL_MSG_PROPTYPE = 13;
            const constexpr int COL_MSG_PROPTITLE = 14;
            const constexpr int COL_MSG_PROPDESC = 15;
            const constexpr int COL_MSG_PROPOSER = 16;
            const constexpr int COL_MSG_VOTER = 17;
            const constexpr int COL_MSG_PROPID = 18;
            const constexpr int COL_MSG_VOTEOPT = 19;
            const constexpr int COL_MSG_DEPOSITOR = 20;
        }

        /// Helper names on column numbers in cosmos_multisend_io table.
        /// See Migrations.cpp file for the source of truth.
        namespace {
            const constexpr int COL_MIO_MSGUID = 0;
            const constexpr int COL_MIO_FROM = 1;
            const constexpr int COL_MIO_TO = 2;
            const constexpr int COL_MIO_AMOUNT = 3;
        }

    void CosmosLikeTransactionDatabaseHelper::inflateMessage(
        soci::session &sql, soci::row const &row, cosmos::Message &msg)
    {
        msg.uid = row.get<std::string>(COL_MSG_UID);
        auto msgType = row.get<std::string>(COL_MSG_MSGTYPE);
        msg.type = msgType;
        msg.log.success = (row.get<int32_t>(COL_MSG_SUCCESS) == 1);
        msg.log.log = row.get<std::string>(COL_MSG_LOG);
        msg.log.messageIndex = row.get<int32_t>(COL_MSG_MSGINDEX);
        switch (cosmos::stringToMsgType(msgType.c_str())) {
        case api::CosmosLikeMsgType::MSGSEND: {
            inflateMsgSendSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGDELEGATE: {
            inflateMsgDelegateSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGUNDELEGATE: {
            inflateMsgUndelegateSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGBEGINREDELEGATE: {
            inflateMsgBeginRedelegateSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL: {
            inflateMsgSubmitProposalSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGVOTE: {
            inflateMsgVoteSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGDEPOSIT: {
            inflateMsgDepositSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD: {
            inflateMsgWithdrawDelegationRewardSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD: {
            inflateMsgWithdrawDelegatorRewardSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION: {
            inflateMsgWithdrawValidatorCommissionSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS: {
            inflateMsgSetWithdrawAddressSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGUNJAIL: {
            inflateMsgUnjailSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGMULTISEND: {
            inflateMsgMultiSendSpecifics(sql, row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGFEES: {
            inflateMsgFeesSpecifics(row, msg);
        } break;
        case api::CosmosLikeMsgType::MSGEDITVALIDATOR:
        case api::CosmosLikeMsgType::MSGCREATEVALIDATOR: {
            throw make_exception(
                api::ErrorCode::UNSUPPORTED_OPERATION,
                "inflate MsgCreateValidator / MsgEditValidator");
        } break;
        case api::CosmosLikeMsgType::UNSUPPORTED:
        default: {
            msg.content = cosmos::MsgUnsupported();
        } break;
        }
    }

        static void inflateTransaction(soci::session &sql,
                                       const soci::row &row,
                                       cosmos::Transaction &tx) {

            // This schema is not namespaced because it only corresponds to the format of a specific
            // join on the cosmos schema, not a single table schema.
            //
            // See CosmosLikeTransactionDatabaseHelper::getTransactionByHash for column names.
            const constexpr int COL_TX_UID = 0;
            const constexpr int COL_TX_HASH = 1;
            const constexpr int COL_TX_TIME = 2;
            const constexpr int COL_TX_FEE = 3;
            const constexpr int COL_TX_GAS = 4;
            const constexpr int COL_TX_GASUSED = 5;
            const constexpr int COL_TX_MEMO = 6;
            const constexpr int COL_TX_BLOCKUID = 7;
            const constexpr int COL_BLK_HASH = 8;
            const constexpr int COL_BLK_HEIGHT = 9;
            const constexpr int COL_BLK_TIME = 10;
            const constexpr int COL_BLK_CURRNAME = 11;

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
                CosmosLikeTransactionDatabaseHelper::inflateMessage(sql, msgRow, msg);
                tx.messages.push_back(msg);

                cosmos::MessageLog log;
                log.log = msgRow.get<std::string>(COL_MSG_LOG);
                log.success = soci::get_number<int32_t>(msgRow, COL_MSG_SUCCESS) != 0;
                log.messageIndex = soci::get_number<int32_t>(msgRow, COL_MSG_INDEX);
                tx.logs.push_back(log);
            }
        }

        void CosmosLikeTransactionDatabaseHelper::insertMessage(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            switch (cosmos::stringToMsgType(msg.type.c_str())) {
            case api::CosmosLikeMsgType::MSGSEND: {
                insertMsgSend(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGDELEGATE: {
                insertMsgDelegate(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGUNDELEGATE: {
                insertMsgUndelegate(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGBEGINREDELEGATE: {
                insertMsgBeginRedelegate(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL: {
                insertMsgSubmitProposal(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGVOTE: {
                insertMsgVote(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGDEPOSIT: {
                insertMsgDeposit(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD: {
                insertMsgWithdrawDelegationReward(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD: {
                insertMsgWithdrawDelegatorReward(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION: {
                insertMsgWithdrawValidatorCommission(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS: {
                insertMsgSetWithdrawAddress(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGUNJAIL: {
                insertMsgUnjail(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGMULTISEND: {
                insertMsgMultiSend(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGFEES: {
                insertMsgFees(sql, txUid, msg, log);
            } break;
            case api::CosmosLikeMsgType::MSGCREATEVALIDATOR:
            case api::CosmosLikeMsgType::MSGEDITVALIDATOR:
            case api::CosmosLikeMsgType::UNSUPPORTED:
            default: {
                insertMsgGeneric(sql, txUid, msg, log);
            } break;
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

    void CosmosLikeTransactionDatabaseHelper::inflateMsgSendSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgSend();
        auto &content = boost::get<cosmos::MsgSend>(msg.content);
        content.fromAddress = row.get<std::string>(COL_MSG_FROMADDR);
        content.toAddress = row.get<std::string>(COL_MSG_TOADDR);
        soci::stringToCoins(row.get<std::string>(COL_MSG_AMOUNT), content.amount);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgDelegateSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgDelegate();
        auto &content = boost::get<cosmos::MsgDelegate>(msg.content);
        content.delegatorAddress = row.get<std::string>(COL_MSG_DELADDR);
        content.validatorAddress = row.get<std::string>(COL_MSG_VALADDR);
        soci::stringToCoin(row.get<std::string>(COL_MSG_AMOUNT), content.amount);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgUndelegateSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgUndelegate();
        auto &content = boost::get<cosmos::MsgUndelegate>(msg.content);
        content.delegatorAddress = row.get<std::string>(COL_MSG_DELADDR);
        content.validatorAddress = row.get<std::string>(COL_MSG_VALADDR);
        soci::stringToCoin(row.get<std::string>(COL_MSG_AMOUNT), content.amount);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgBeginRedelegateSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgBeginRedelegate();
        auto &content = boost::get<cosmos::MsgBeginRedelegate>(msg.content);
        content.delegatorAddress = row.get<std::string>(COL_MSG_DELADDR);
        content.validatorSourceAddress = row.get<std::string>(COL_MSG_VALSRCADDR);
        content.validatorDestinationAddress = row.get<std::string>(COL_MSG_VALDESTADDR);
        soci::stringToCoin(row.get<std::string>(COL_MSG_AMOUNT), content.amount);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgSubmitProposalSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgSubmitProposal();
        auto &content = boost::get<cosmos::MsgSubmitProposal>(msg.content);
        content.content.type = row.get<std::string>(COL_MSG_PROPTYPE);
        content.content.title = row.get<std::string>(COL_MSG_PROPTITLE);
        content.content.descr = row.get<std::string>(COL_MSG_PROPDESC);
        content.proposer = row.get<std::string>(COL_MSG_PROPOSER);
        soci::stringToCoins(row.get<std::string>(COL_MSG_AMOUNT), content.initialDeposit);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgVoteSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgVote();
        auto &content = boost::get<cosmos::MsgVote>(msg.content);
        content.voter = row.get<std::string>(COL_MSG_VOTER);
        content.proposalId = row.get<std::string>(COL_MSG_PROPID);
        content.option =
            api::from_string<api::CosmosLikeVoteOption>(row.get<std::string>(COL_MSG_VOTEOPT));
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgDepositSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgDeposit();
        auto &content = boost::get<cosmos::MsgDeposit>(msg.content);
        content.depositor = row.get<std::string>(COL_MSG_DEPOSITOR);
        content.proposalId = row.get<std::string>(COL_MSG_PROPID);
        soci::stringToCoins(row.get<std::string>(COL_MSG_AMOUNT), content.amount);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgWithdrawDelegationRewardSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgWithdrawDelegationReward();
        auto &content = boost::get<cosmos::MsgWithdrawDelegationReward>(msg.content);
        content.delegatorAddress = row.get<std::string>(COL_MSG_DELADDR);
        content.validatorAddress = row.get<std::string>(COL_MSG_VALADDR);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgWithdrawDelegatorRewardSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgWithdrawDelegatorReward();
        auto &content = boost::get<cosmos::MsgWithdrawDelegatorReward>(msg.content);
        content.delegatorAddress = row.get<std::string>(COL_MSG_DELADDR);
        content.validatorAddress = row.get<std::string>(COL_MSG_VALADDR);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgWithdrawValidatorCommissionSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgWithdrawValidatorCommission();
        auto &content = boost::get<cosmos::MsgWithdrawValidatorCommission>(msg.content);
        content.validatorAddress = row.get<std::string>(COL_MSG_VALADDR);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgSetWithdrawAddressSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgSetWithdrawAddress();
        auto &content = boost::get<cosmos::MsgSetWithdrawAddress>(msg.content);
        content.delegatorAddress = row.get<std::string>(COL_MSG_DELADDR);
        content.withdrawAddress = row.get<std::string>(COL_MSG_TOADDR);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgUnjailSpecifics(
        soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgUnjail();
        auto &content = boost::get<cosmos::MsgUnjail>(msg.content);
        content.validatorAddress = row.get<std::string>(COL_MSG_VALADDR);
    }

    void CosmosLikeTransactionDatabaseHelper::inflateMsgMultiSendSpecifics(
        soci::session &sql, soci::row const &row, cosmos::Message &msg)
    {
        msg.content = cosmos::MsgMultiSend();
        auto &content = boost::get<cosmos::MsgMultiSend>(msg.content);
        // Make a join on cosmos_multisend_io to fill the message
        soci::rowset<soci::row> multiSendRows =
            (sql.prepare << "SELECT * FROM cosmos_multisend_io WHERE message_uid = :msgUid",
             soci::use(msg.uid));
        for (auto &multiSendRow : multiSendRows) {
            const auto inputAddress =
                multiSendRow.get<Option<std::string>>(COL_MIO_FROM).getValueOr("");
            const auto outputAddress = multiSendRow.get<Option<std::string>>(COL_MIO_TO).getValueOr("");
            if (!inputAddress.empty()) {
                cosmos::MultiSendInput newInput;
                newInput.fromAddress = inputAddress;
                soci::stringToCoins(multiSendRow.get<std::string>(COL_MIO_AMOUNT), newInput.coins);
                content.inputs.push_back(newInput);
            }
            else if (!outputAddress.empty()) {
                cosmos::MultiSendOutput newOutput;
                newOutput.toAddress = outputAddress;
                soci::stringToCoins(multiSendRow.get<std::string>(COL_MIO_AMOUNT), newOutput.coins);
                content.outputs.push_back(newOutput);
            }
        }
    }

        void CosmosLikeTransactionDatabaseHelper::inflateMsgFeesSpecifics(soci::row const& row, cosmos::Message& msg) {
            msg.content = cosmos::MsgFees();
            auto &content = boost::get<cosmos::MsgFees>(msg.content);
            content.payerAddress = row.get<std::string>(COL_MSG_FROMADDR);
            soci::stringToCoin(row.get<std::string>(COL_MSG_AMOUNT), content.fees);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgSend(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgSend>(msg.content);
            const auto coins = soci::coinsToString(m.amount);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, from_address, to_address, amount) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.fromAddress), soci::use(m.toAddress), soci::use(coins);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgDelegate(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgDelegate>(msg.content);
            const auto coin = soci::coinToString(m.amount);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, delegator_address, validator_address, amount) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.delegatorAddress), soci::use(m.validatorAddress), soci::use(coin);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgUndelegate(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgUndelegate>(msg.content);
            const auto coin = soci::coinToString(m.amount);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, delegator_address, validator_address, amount) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :ta, :amount)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.delegatorAddress), soci::use(m.validatorAddress), soci::use(coin);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgBeginRedelegate(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgBeginRedelegate>(msg.content);
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

        void CosmosLikeTransactionDatabaseHelper::insertMsgSubmitProposal(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgSubmitProposal>(msg.content);
            const auto coins = soci::coinsToString(m.initialDeposit);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, proposer, content_type,"
                   "content_title, content_description, amount) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :proposer,"
                   ":ctype, :ctitle, :cdescription, :amount)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex), soci::use(m.proposer),
                soci::use(m.content.type), soci::use(m.content.title),
                soci::use(m.content.descr), soci::use(coins);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgVote(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgVote>(msg.content);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, proposal_id, voter,"
                   "vote_option) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :pid, :voter, :opt)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.proposalId), soci::use(m.voter), soci::use(api::to_string(m.option));
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgDeposit(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgDeposit>(msg.content);
            const auto coins = soci::coinsToString(m.amount);
            sql << "INSERT INTO cosmos_messages (uid, transaction_uid, "
                   "message_type, log, success, "
                   "msg_index, depositor, proposal_id, amount) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :dep, :pid, :amount)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex), soci::use(m.depositor),
                soci::use(m.proposalId), soci::use(coins);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgWithdrawDelegationReward(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgWithdrawDelegationReward>(msg.content);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, delegator_address, validator_address) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :va)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.delegatorAddress), soci::use(m.validatorAddress);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgWithdrawDelegatorReward(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgWithdrawDelegatorReward>(msg.content);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, delegator_address, validator_address) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :va)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.delegatorAddress), soci::use(m.validatorAddress);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgWithdrawValidatorCommission(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgWithdrawValidatorCommission>(msg.content);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, validator_address) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :va)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.validatorAddress);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgSetWithdrawAddress(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgSetWithdrawAddress>(msg.content);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, delegator_address, to_address) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :da, :withdraw)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.delegatorAddress), soci::use(m.withdrawAddress);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgUnjail(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgUnjail>(msg.content);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, validator_address) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :va)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.validatorAddress);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgMultiSend(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgMultiSend>(msg.content);
            std::vector<cosmos::Coin> totalAmount;

            // This snippet left-folds a iterable<vector<cosmos::Coin>>
            // into vector<cosmos::Coin> using addition.
            // Hopefully transform_reduce or flatten functions can help us later.
            // The map intermediate helps for out-of-order denominations.
            std::unordered_map<std::string, BigInt> denomToAmt;
            std::for_each(m.inputs.cbegin(), m.inputs.cend(), [&denomToAmt](const auto &input) {
                std::for_each(
                    input.coins.cbegin(),
                    input.coins.cend(),
                    [&denomToAmt](const auto &amountDenom) {
                        auto searchDenom = denomToAmt.find(amountDenom.denom);
                        if (searchDenom == denomToAmt.end()) {
                            denomToAmt.insert(
                                {amountDenom.denom, BigInt::fromString(amountDenom.amount)});
                            return;
                        }
                        searchDenom->second =
                            searchDenom->second + BigInt::fromString(amountDenom.amount);
                    });
            });
            // Add each pair of denomToAmt into totalAmount
            totalAmount.reserve(denomToAmt.size());
            for (const auto &pair : denomToAmt) {
                totalAmount.emplace_back(pair.second.toString(), pair.first);
            }

            // Insert the global message information in cosmos_messages
            const auto coins = soci::coinsToString(totalAmount);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, amount) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :amount)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex), soci::use(coins);

            // Insert each input and each output to the cosmos_multisend_io table
            std::for_each(m.inputs.cbegin(), m.inputs.cend(), [&sql, &msg](const auto &input) {
                const auto inputCoins = soci::coinsToString(input.coins);
                sql << "INSERT INTO cosmos_multisend_io (message_uid, from_address, amount) "
                       "VALUES (:uid, :fa, :amt)",
                    soci::use(msg.uid), soci::use(input.fromAddress), soci::use(inputCoins);
            });

            std::for_each(m.outputs.cbegin(), m.outputs.cend(), [&sql, &msg](const auto &output) {
                const auto outputCoins = soci::coinsToString(output.coins);
                sql << "INSERT INTO cosmos_multisend_io (message_uid, to_address, amount) "
                       "VALUES (:uid, :ta, :amt)",
                    soci::use(msg.uid), soci::use(output.toAddress), soci::use(outputCoins);
            });
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgFees(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            const auto &m = boost::get<cosmos::MsgFees>(msg.content);
            const auto &fees = soci::coinToString(m.fees);
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, log,"
                   "success, msg_index, from_address, amount) "
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi, :fa, :amount)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex),
                soci::use(m.payerAddress), soci::use(fees);
        }

        void CosmosLikeTransactionDatabaseHelper::insertMsgGeneric(
            soci::session &sql,
            std::string const &txUid,
            cosmos::Message const &msg,
            cosmos::MessageLog const &log)
        {
            // Do record the message type, even if unsupported
            sql << "INSERT INTO cosmos_messages (uid,"
                   "transaction_uid, message_type, "
                   "log, success, msg_index)"
                   "VALUES (:uid, :tuid, :mt, :log, :success, :mi)",
                soci::use(msg.uid), soci::use(txUid), soci::use(msg.type), soci::use(log.log),
                soci::use(log.success ? 1 : 0), soci::use(log.messageIndex);
        }
    }
}
