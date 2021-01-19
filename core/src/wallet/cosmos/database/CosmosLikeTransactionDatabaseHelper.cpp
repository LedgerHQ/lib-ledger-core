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

#include <algorithm>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

#include <api/enum_from_string.hpp>
#include <crypto/SHA256.hpp>
#include <database/soci-date.h>
#include <database/soci-number.h>
#include <database/soci-option.h>
#include <utils/Exception.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>
#include <wallet/cosmos/database/SociCosmosAmount.hpp>

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
}  // namespace

/// Helper names on column numbers in cosmos_multisend_io table.
/// See Migrations.cpp file for the source of truth.
namespace {
const constexpr int COL_MIO_MSGUID = 0;
const constexpr int COL_MIO_FROM = 1;
const constexpr int COL_MIO_TO = 2;
const constexpr int COL_MIO_AMOUNT = 3;
}  // namespace

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
            api::ErrorCode::UNSUPPORTED_OPERATION, "inflate MsgCreateValidator / MsgEditValidator");
    } break;
    case api::CosmosLikeMsgType::UNSUPPORTED:
    default: {
        msg.content = cosmos::MsgUnsupported();
    } break;
    }
}

static void inflateTransaction(soci::session &sql, const soci::row &row, cosmos::Transaction &tx)
{
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
    tx.gasUsed = row.get<Option<std::string>>(COL_TX_GASUSED).map<BigInt>([](const std::string &v) {
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

    soci::rowset<soci::row> msgRows =
        (sql.prepare << "SELECT * FROM cosmos_messages WHERE transaction_uid = :txUid ORDER BY "
                        "cosmos_messages.msg_index",
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

std::string CosmosLikeTransactionDatabaseHelper::createCosmosMessageUid(std::string const &txUid, uint64_t msgIndex)
{
    auto result = SHA256::stringToHexHash(fmt::format("uid:{}+{}", txUid, msgIndex));
    return result;
}

std::string CosmosLikeTransactionDatabaseHelper::createCosmosTransactionUid(
    std::string const &accountUid, std::string const &txHash)
{
    auto result = SHA256::stringToHexHash(fmt::format("uid:{}+{}", accountUid, txHash));
    return result;
}

bool CosmosLikeTransactionDatabaseHelper::transactionExists(
    soci::session &sql, const std::string &txUid)
{
    int32_t count = 0;
    sql << "SELECT COUNT(*) FROM cosmos_transactions WHERE uid = :txUid", soci::use(txUid),
        soci::into(count);
    return count == 1;
}

bool CosmosLikeTransactionDatabaseHelper::getTransactionByHash(
    soci::session &sql, const std::string &txHash, cosmos::Transaction &tx)
{
    soci::rowset<soci::row> rows =
        (sql.prepare << "SELECT tx.uid, tx.hash, tx.time, "
                        "tx.fee_amount, tx.gas, tx.gas_used, tx.memo, tx.block_uid, "
                        "block.hash, block.height, block.time, block.currency_name "
                        "FROM cosmos_transactions AS tx "
                        "LEFT JOIN blocks AS block ON tx.block_uid = block.uid "
                        "WHERE tx.hash = :hash ",
         soci::use(txHash));

    for (auto &row : rows) {
        inflateTransaction(sql, row, tx);
        return true;
    }

    return false;
}

bool CosmosLikeTransactionDatabaseHelper::getMessageByUid(
    soci::session &sql, const std::string &msgUid, cosmos::Message &msg)
{
    soci::rowset<soci::row> rows =
        (sql.prepare << "SELECT * FROM cosmos_messages WHERE uid = :msgUid ", soci::use(msgUid));

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

void CosmosLikeTransactionDatabaseHelper::inflateMsgFeesSpecifics(
    soci::row const &row, cosmos::Message &msg)
{
    msg.content = cosmos::MsgFees();
    auto &content = boost::get<cosmos::MsgFees>(msg.content);
    content.payerAddress = row.get<std::string>(COL_MSG_FROMADDR);
    soci::stringToCoin(row.get<std::string>(COL_MSG_AMOUNT), content.fees);
}

}  // namespace core
}  // namespace ledger
