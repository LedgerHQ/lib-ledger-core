/*
 *
 * rpcs_parsers.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/06/2019.
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

#ifndef LEDGER_CORE_RPCS_PARSERS_HPP
#define LEDGER_CORE_RPCS_PARSERS_HPP

#include <cassert>
#include <numeric>

#include <cereal/external/base64.hpp>

#include <rapidjson/document.h>
#include <utils/DateUtils.hpp>
#include <utils/Option.hpp>
#include <wallet/common/Block.h>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeMessage.hpp>
#include <wallet/cosmos/cosmos.hpp>
#include <wallet/cosmos/keychains/CosmosLikeKeychain.hpp>

#define COSMOS_PARSE_MSG_CONTENT(MsgType)   \
    if (out.type == "cosmos-sdk/" #MsgType) \
        return parse##MsgType(contentNode, out.content);

namespace ledger {
namespace core {
using namespace cosmos::constants;
namespace rpcs_parsers {

template <class T>
std::string getStringFromVariableKeys(const T &node, const std::list<std::string> &keys)
{
    for (const auto &key : keys) {
        if (node.HasMember(key.c_str())) {
            return node[key.c_str()].GetString();
        }
    }
    return "";
}

template <class T>
void parseCoin(const T &node, cosmos::Coin &out)
{
    assert((node.HasMember(kAmount)));
    assert((node.HasMember(kDenom)));
    out.amount = node[kAmount].GetString();
    out.denom = node[kDenom].GetString();
}

template <typename T>
void parseCoinVector(const T &array, std::vector<cosmos::Coin> &out)
{
    out.assign((std::size_t)array.Size(), cosmos::Coin());
    auto index = 0;
    for (const auto &n : array) {
        parseCoin(n.GetObject(), out[index]);
        index += 1;
    }
}

template <typename T>
void parseMultiSendInputs(const T &array, std::vector<cosmos::MultiSendInput> &out)
{
    auto index = 0;
    out.assign((std::size_t)array.Size(), cosmos::MultiSendInput());
    for (const auto &n : array) {
        auto input = n.GetObject();
        out[index].fromAddress = input[kAddress].GetString();
        if (input[kCoins].IsArray()) {
            parseCoinVector(input[kCoins].GetArray(), out[index].coins);
        }
        index += 1;
    }
}

template <typename T>
void parseMultiSendOutputs(const T &array, std::vector<cosmos::MultiSendOutput> &out)
{
    auto index = 0;
    out.assign((std::size_t)array.Size(), cosmos::MultiSendOutput());
    for (const auto &n : array) {
        auto output = n.GetObject();
        out[index].toAddress = output[kAddress].GetString();
        if (output[kCoins].IsArray()) {
            parseCoinVector(output[kCoins].GetArray(), out[index].coins);
        }
        index += 1;
    }
}

template <typename T>
void parseDescription(const T &descriptionNode, cosmos::ValidatorDescription &out)
{
    assert((descriptionNode.HasMember(kMoniker)));
    out.moniker = descriptionNode[kMoniker].GetString();
    if (descriptionNode.HasMember(kWebsite) && descriptionNode[kWebsite].IsString()) {
        out.website = optional<std::string>(descriptionNode[kWebsite].GetString());
    }
    if (descriptionNode.HasMember(kIdentity) && descriptionNode[kIdentity].IsString()) {
        out.identity = optional<std::string>(descriptionNode[kIdentity].GetString());
    }
    if (descriptionNode.HasMember(kDetails) && descriptionNode[kDetails].IsString()) {
        out.details = optional<std::string>(descriptionNode[kDetails].GetString());
    }
}

template <typename T>
void parseCommission(const T &commissionNode, cosmos::ValidatorCommission &out)
{
    assert((commissionNode.HasMember(kUpdateTime)));
    out.updateTime = DateUtils::fromJSON(commissionNode[kUpdateTime].GetString());
    assert((commissionNode.HasMember(kCommissionRates)));
    const auto rateObject = commissionNode[kCommissionRates].GetObject();

    assert((rateObject.HasMember(kCommissionRate)));
    assert((rateObject.HasMember(kCommissionMaxRate)));
    assert((rateObject.HasMember(kCommissionMaxChangeRate)));
    out.rates.rate = rateObject[kCommissionRate].GetString();
    out.rates.maxRate = rateObject[kCommissionMaxRate].GetString();
    out.rates.maxChangeRate = rateObject[kCommissionMaxChangeRate].GetString();
}

template <typename T>
void parseDelegation(const T &delegationNode, cosmos::Delegation &out)
{
    assert(delegationNode.HasMember(kDelegatorAddress));
    assert(delegationNode.HasMember(kValidatorAddress));
    assert(delegationNode.HasMember(kBalance));
    out.delegatorAddress = delegationNode[kDelegatorAddress].GetString();
    out.validatorAddress = delegationNode[kValidatorAddress].GetString();
    out.delegatedAmount = BigInt::fromString(delegationNode[kBalance].GetString());
}

template <typename T>
void parseReward(const T &rewardNode, cosmos::Reward &out)
{
    assert(rewardNode.HasMember(kValidatorAddress));
    assert(rewardNode.HasMember(kReward));
    out.validatorAddress = rewardNode[kValidatorAddress].GetString();
    // Do nothing if the array is "null"
    if (rewardNode[kReward].IsArray()) {
        parseCoin(
            rewardNode[kReward].GetArray()[0],
            out.pendingReward);  // Assuming only one reward per validator
    }
}

template <class T>
void parseBlock(const T &node, const std::string &currencyName, cosmos::Block &out)
{
    out.currencyName = currencyName;
    out.hash = node[kBlockMeta].GetObject()[kBlockId].GetObject()[kHash].GetString();
    out.height =
        BigInt::fromString(node[kBlockMeta].GetObject()[kHeader].GetObject()[kHeight].GetString())
            .toUint64();
    out.time =
        DateUtils::fromJSON(node[kBlockMeta].GetObject()[kHeader].GetObject()[kTime].GetString());
}

template <typename T>
void parseProposalContent(const T &node, cosmos::ProposalContent &out)
{
    assert((node.HasMember(kType)));
    assert((node.HasMember(kDescription)));
    assert((node.HasMember(kTitle)));
    out.type = node[kType].GetString();
    out.descr = node[kDescription].GetString();
    out.title = node[kTitle].GetString();
}

template <typename T>
void parseFee(const T &node, cosmos::Fee &out)
{
    assert((node.HasMember(kGas)));
    assert((node.HasMember(kAmount)));
    out.gas = BigInt::fromString(node[kGas].GetString());
    if (node[kAmount].IsArray()) {
        const auto &amountArray = node[kAmount].GetArray();
        out.amount.assign((std::size_t)amountArray.Capacity(), cosmos::Coin());
        auto index = 0;
        for (const auto &aNode : amountArray) {
            parseCoin(aNode.GetObject(), out.amount[index]);
            index += 1;
        }
    }
}

template <class T>
void parseAccount(const T &accountNode, cosmos::Account &account)
{
    assert((accountNode.HasMember(kValue)));
    assert((accountNode.HasMember(kType)));
    const auto &node = accountNode[kValue].GetObject();
    account.type = accountNode[kType].GetString();

    assert((node.HasMember(kAccountNumber)));
    assert((node.HasMember(kSequence)));
    assert((node.HasMember(kAddress)));
    assert((node.HasMember(kCoins)));
    account.accountNumber = node[kAccountNumber].GetString();
    account.sequence = node[kSequence].GetString();
    account.address = node[kAddress].GetString();
    if (node[kCoins].IsArray()){
        const auto &balances = node[kCoins].GetArray();
        parseCoinVector(balances, account.balances);
    }
}

template <typename T>
void parseMsgSend(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgSend msg;

    assert((n.HasMember(kFromAddress)));
    assert((n.HasMember(kToAddress)));
    msg.fromAddress = n[kFromAddress].GetString();
    msg.toAddress = n[kToAddress].GetString();
    if (n[kAmount].IsArray()) {
        parseCoinVector(n[kAmount].GetArray(), msg.amount);
    }
    out = msg;
}

template <typename T>
void parseMsgDelegate(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgDelegate msg;

    assert((n.HasMember(kDelegatorAddress)));
    assert((n.HasMember(kValidatorAddress)));
    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.validatorAddress = n[kValidatorAddress].GetString();
    parseCoin(n[kAmount].GetObject(), msg.amount);
    out = msg;
}

template <typename T>
void parseMsgBeginRedelegate(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgBeginRedelegate msg;

    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.validatorSourceAddress = n[kValidatorSrcAddress].GetString();
    msg.validatorDestinationAddress = n[kValidatorDstAddress].GetString();
    parseCoin(n[kAmount].GetObject(), msg.amount);
    out = msg;
}

template <typename T>
void parseMsgUndelegate(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgUndelegate msg;

    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.validatorAddress = n[kValidatorAddress].GetString();
    parseCoin(n[kAmount].GetObject(), msg.amount);
    out = msg;
}

template <typename T>
void parseMsgSubmitProposal(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgSubmitProposal msg;

    msg.proposer = n[kProposer].GetString();
    parseProposalContent(n[kContent].GetObject(), msg.content);
    if (n[kInitialDeposit].IsArray()){
        parseCoinVector(n[kInitialDeposit].GetArray(), msg.initialDeposit);
    }
    out = msg;
}

template <typename T>
void parseMsgVote(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgVote msg;

    msg.voter = n[kVoter].GetString();
    msg.proposalId = n[kProposalId].GetString();
    const auto &option = n[kOption].GetString();
    if (option == "Yes") {
        msg.option = cosmos::VoteOption::COSMOSVOTEYES;
    }
    else if (option == "No") {
        msg.option = cosmos::VoteOption::COSMOSVOTENO;
    }
    else if (option == "NoWithVeto") {
        msg.option = cosmos::VoteOption::COSMOSVOTENOWITHVETO;
    }
    else if (option == "Abstain") {
        msg.option = cosmos::VoteOption::COSMOSVOTEABSTAIN;
    }
    out = msg;
}

template <typename T>
void parseMsgDeposit(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgDeposit msg;

    msg.depositor = n[kDepositor].GetString();
    msg.proposalId = n[kProposalId].GetString();
    if (n[kAmount].IsArray()){
        parseCoinVector(n[kAmount].GetArray(), msg.amount);
    }
    out = msg;
}

template <typename T>
void parseMsgWithdrawDelegationReward(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgWithdrawDelegationReward msg;

    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.validatorAddress = n[kValidatorAddress].GetString();
    out = msg;
}

template <typename T>
void parseMsgMultiSend(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgMultiSend msg;

    assert((n.HasMember(kInputs)));
    assert((n.HasMember(kOutputs)));

    // inputs: list<CosmosLikeMultiSendInput>;
    if(n[kInputs].IsArray()){
        auto inputsArray = n[kInputs].GetArray();
        parseMultiSendInputs(inputsArray, msg.inputs);
    }

    // outputs: list<CosmosLikeMultiSendOutput>;
    if(n[kOutputs].IsArray()) {
        auto outputsArray = n[kInputs].GetArray();
        parseMultiSendOutputs(outputsArray, msg.outputs);
    }

    out = msg;
}

template <typename T>
void parseMsgCreateValidator(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgCreateValidator msg;
    assert((n.HasMember(kDescription)));
    assert((n.HasMember(kCommission)));
    assert((n.HasMember(kMinSelfDelegation)));
    assert((n.HasMember(kDelegatorAddress)));
    assert((n.HasMember(kValidatorAddress)));
    assert((n.HasMember(kPubKey)));
    assert((n.HasMember(kValue)));
    parseDescription(n[kDescription].GetObject(), msg.descr);
    parseCommission(n[kCommission].GetObject(), msg.commission);
    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.validatorAddress = n[kValidatorAddress].GetString();
    msg.pubkey = n[kPubKey].GetString();
    msg.minSelfDelegation = n[kMinSelfDelegation].GetString();
    parseCoin(n[kValue].GetObject(), msg.value);

    out = msg;
}

template <typename T>
void parseMsgEditValidator(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgEditValidator msg;

    msg.validatorAddress = n[kValidatorAddress].GetString();
    if (n.HasMember(kEditValCommissionRate) && n[kEditValCommissionRate].IsString()) {
        msg.commissionRate = optional<std::string>(n[kEditValCommissionRate].GetString());
    }
    if (n.HasMember(kMinSelfDelegation) && n[kMinSelfDelegation].IsString()) {
        msg.minSelfDelegation = optional<std::string>(n[kMinSelfDelegation].GetString());
    }
    if (n.HasMember(kDescription) && n[kDescription].IsObject()) {
        cosmos::ValidatorDescription desc;
        parseDescription(n[kDescription].GetObject(), desc);
        msg.descr = optional<cosmos::ValidatorDescription>(desc);
    }
    out = msg;
}

template <typename T>
void parseMsgSetWithdrawAddress(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgSetWithdrawAddress msg;

    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.withdrawAddress = n[kWithdrawAddress].GetString();
    out = msg;
}

template <typename T>
void parseMsgWithdrawDelegatorReward(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgWithdrawDelegatorReward msg;

    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.validatorAddress = n[kValidatorAddress].GetString();
    out = msg;
}

template <typename T>
void parseMsgWithdrawValidatorCommission(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgWithdrawValidatorCommission msg;

    msg.validatorAddress = n[kValidatorAddress].GetString();
    out = msg;
}

template <typename T>
void parseMsgUnjail(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgUnjail msg;

    msg.validatorAddress = n[kValidatorAddress].GetString();
    out = msg;
}

template <typename T>
void parseMessage(
    const T &messageNode, cosmos::Message &out, const cosmos::MessageLog &associated_log)
{
    out.type = messageNode[kType].GetString();
    out.log = associated_log;
    const auto &contentNode = messageNode[kValue].GetObject();
    COSMOS_PARSE_MSG_CONTENT(MsgSend)
    COSMOS_PARSE_MSG_CONTENT(MsgDelegate)
    COSMOS_PARSE_MSG_CONTENT(MsgBeginRedelegate)
    COSMOS_PARSE_MSG_CONTENT(MsgUndelegate)
    COSMOS_PARSE_MSG_CONTENT(MsgSubmitProposal)
    COSMOS_PARSE_MSG_CONTENT(MsgVote)
    COSMOS_PARSE_MSG_CONTENT(MsgDeposit)
    COSMOS_PARSE_MSG_CONTENT(MsgWithdrawDelegationReward)
    COSMOS_PARSE_MSG_CONTENT(MsgMultiSend)
    COSMOS_PARSE_MSG_CONTENT(MsgCreateValidator)
    COSMOS_PARSE_MSG_CONTENT(MsgEditValidator)
    COSMOS_PARSE_MSG_CONTENT(MsgSetWithdrawAddress)
    COSMOS_PARSE_MSG_CONTENT(MsgWithdrawDelegatorReward)
    COSMOS_PARSE_MSG_CONTENT(MsgWithdrawValidatorCommission)
    COSMOS_PARSE_MSG_CONTENT(MsgUnjail)
}

template <typename T>
void parseSignerPubKey(const T &node, std::string &pubkey)
{
    if(!node.HasMember(kSignatures) || !node[kSignatures].IsArray() ||
       node[kSignatures].GetArray().Empty()) {
        throw make_exception(
            api::ErrorCode::ILLEGAL_STATE,
            "The received signer pubkey does not have signature information at key {}",
            kSignatures);
    }

    const auto firstSignature = node[kSignatures].GetArray()[0].GetObject();
    if (firstSignature.HasMember(kPubKey) &&
        firstSignature[kPubKey].GetObject().HasMember(kValue)) {
        pubkey = firstSignature[kPubKey].GetObject()[kValue].GetString();
    }
}

template <class T>
void parseTransaction(const T &node, cosmos::Transaction &transaction)
{
    assert((node.HasMember(kTxHash)));
    transaction.hash = node[kTxHash].GetString();
    if (node.HasMember(kHeight)) {
        cosmos::Block block;
        block.height = BigInt::fromString(node[kHeight].GetString()).toUint64();
        block.currencyName = currencies::ATOM.name;
        transaction.block = block;
    }
    if (node.HasMember(kGasUsed)) {
        transaction.gasUsed = Option<BigInt>(BigInt::fromString(node[kGasUsed].GetString()));
    }

    assert((node.HasMember(kLogs)));
    if (node[kLogs].IsArray()) {
        for (const auto &lNode : node[kLogs].GetArray()) {
            cosmos::MessageLog log;
            assert((lNode.HasMember(kLog)));
            assert((lNode.HasMember(kSuccess)));
            assert((lNode.HasMember(kMsgIndex)));
            log.success = lNode[kSuccess].GetBool();
            log.log = lNode[kLog].GetString();
            log.messageIndex = BigInt::fromString(lNode[kMsgIndex].GetString()).toInt();
            transaction.logs.emplace_back(log);
        }
    }
    assert((node.HasMember(kTimestamp)));
    transaction.timestamp = DateUtils::fromJSON(node[kTimestamp].GetString());

    assert((node.HasMember(kTx)));
    const auto &tNode = node[kTx].GetObject();
    assert((tNode.HasMember(kValue)));
    const auto &vNode = tNode[kValue].GetObject();

    if (vNode.HasMember(kMemo)) {
        transaction.memo = vNode[kMemo].GetString();
    }

    assert((vNode.HasMember(kMessage)));
    if (vNode[kMessage].IsArray()) {
        const auto &msgArray = vNode[kMessage].GetArray();
        transaction.messages.assign((std::size_t)msgArray.Capacity(), cosmos::Message());
        auto index = 0;
        for (const auto &mNode : msgArray) {
            parseMessage(mNode, transaction.messages[index], transaction.logs[index]);
            index++;
        }
    }

    assert((vNode.HasMember(kFee)));
    parseFee(vNode[kFee].GetObject(), transaction.fee);
}

template <typename T>
void parseValidatorSetEntry(const T &n, cosmos::Validator &out)
{
    assert((n.HasMember(kDescription)));
    parseDescription(n[kDescription].GetObject(), out.validatorDetails);

    assert((n.HasMember(kCommission)));
    parseCommission(n[kCommission].GetObject(), out.commission);

    assert((n.HasMember("unbonding_height")));
    out.unbondingHeight = BigInt::fromString(n["unbonding_height"].GetString()).toInt();

    assert((n.HasMember("unbonding_time")));
    if (out.unbondingHeight == 0) {
        out.unbondingTime = optional<std::chrono::system_clock::time_point>();
    }
    else {
        out.unbondingTime = optional<std::chrono::system_clock::time_point>(
            DateUtils::fromJSON(n["unbonding_time"].GetString()));
    }

    assert((n.HasMember(kMinSelfDelegation)));
    out.minSelfDelegation = n[kMinSelfDelegation].GetString();

    assert((n.HasMember("jailed")));
    out.jailed = n["jailed"].GetBool();

    // Note : we ignore delegator_shares, as it seems the actual
    // voting power is the tokens key, in uatom
    assert((n.HasMember("tokens")));
    out.votingPower = n["tokens"].GetString();

    assert((n.HasMember("operator_address")));
    out.operatorAddress = n["operator_address"].GetString();

    assert((n.HasMember("consensus_pubkey")));
    out.consensusPubkey = n["consensus_pubkey"].GetString();

    assert((n.HasMember("status")));
    out.activeStatus = BigInt::fromString(n["status"].GetString()).toInt();
}

//  Parse an unbonding entry from (/staking/delegators/{address}/unbonding_delegations)
//  {
//    "creation_height": "1346685",
//    "completion_time": "2020-04-21T12:28:37.550789506Z",
//    "initial_balance": "20000",
//    "balance": "20000"
//  }
template <typename T>
void parseUnbondingEntry(const T &n, cosmos::UnbondingEntry &out)
{
    assert((n.HasMember(kCreationHeight)));
    assert((n.HasMember(kCompletionTime)));
    assert((n.HasMember(kInitialBalance)));
    assert((n.HasMember(kBalance)));

    out.creationHeight = BigInt::fromString(n[kCreationHeight].GetString());
    out.completionTime = DateUtils::fromJSON(n[kCompletionTime].GetString());
    out.initialBalance = BigInt::fromString(n[kInitialBalance].GetString());
    out.balance = BigInt::fromString(n[kBalance].GetString());
}

// Parse unbonding result from (/staking/delegators/{address}/unbonding_delegations)
// into cosmos::Unbonding
// {
//   "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
//   "validator_address": "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn",
//   "entries": [
//     {
//       "creation_height": "1177125",
//       "completion_time": "2020-04-07T10:39:59.296697797Z",
//       "initial_balance": "500",
//       "balance": "500"
//     }
//   ]
// }
template <typename T>
void parseUnbonding(const T &n, cosmos::Unbonding &out)
{
    assert((n.HasMember(kDelegatorAddress)));
    assert((n.HasMember(kValidatorAddress)));
    assert((n.HasMember("entries")));
    out.entries = std::list<cosmos::UnbondingEntry>();
    out.delegatorAddress = n[kDelegatorAddress].GetString();
    out.validatorAddress = n[kValidatorAddress].GetString();
    if (n["entries"].IsArray()) {
        for (const auto &entry : n["entries"].GetArray()) {
            cosmos::UnbondingEntry parsedEntry;
            parseUnbondingEntry(entry.GetObject(), parsedEntry);
            out.entries.emplace_back(std::move(parsedEntry));
        }
    }
}

// Parse unbonding result from (/staking/delegators/{address}/unbonding_delegations)
// into cosmos::UnbondingList
// {
//   "height": "1347301",
//   "result": [
//     {
//       "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
//       "validator_address": "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn",
//       "entries": [
//         {
//           "creation_height": "1177125",
//           "completion_time": "2020-04-07T10:39:59.296697797Z",
//           "initial_balance": "500",
//           "balance": "500"
//         }
//       ]
//     },
//     {},
//     ...
//   ]
// }
template <typename T>
void parseUnbondingList(const T &n, cosmos::UnbondingList &out)
{
    assert((n.HasMember("result")));
    out.clear();

    if(n["result"].IsArray()){
        for (const auto &unbonding : n["result"].GetArray()) {
            cosmos::Unbonding parsedUnbonding;
            parseUnbonding(unbonding.GetObject(), parsedUnbonding);
            out.emplace_back(std::make_shared<cosmos::Unbonding>(std::move(parsedUnbonding)));
        }
    }
}

// Parse redelegation (/staking/redelegations?delegator={address} || any other query filter)
// into cosmos::RedelegationEntry
// {
//   "creation_height": 1107334,
//   "completion_time": "2020-04-01T15:46:03.941380099Z",
//   "initial_balance": "1850",
//   "shares_dst": "1850.000000000000000000",
//   "balance": "1850"
// }
template <typename T>
void parseRedelegationEntry(const T &n, cosmos::RedelegationEntry &out)
{
    assert((n.HasMember(kCreationHeight)));
    assert((n.HasMember(kCompletionTime)));
    assert((n.HasMember(kInitialBalance)));
    assert((n.HasMember(kBalance)));

    out.creationHeight = BigInt(n[kCreationHeight].GetInt());
    out.completionTime = DateUtils::fromJSON(n[kCompletionTime].GetString());
    out.initialBalance = BigInt::fromString(n[kInitialBalance].GetString());
    out.balance = BigInt::fromString(n[kBalance].GetString());
}

// Parse redelegation (/staking/redelegations?delegator={address})
// into cosmos::Redelegation
// {
//   "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
//   "validator_src_address": "cosmosvaloper1sjllsnramtg3ewxqwwrwjxfgc4n4ef9u2lcnj0",
//   "validator_dst_address": "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn",
//   "entries": [
//     {
//       "creation_height": 1107334,
//       "completion_time": "2020-04-01T15:46:03.941380099Z",
//       "initial_balance": "1850",
//       "shares_dst": "1850.000000000000000000",
//       "balance": "1850"
//     }
//   ]
// }
template <typename T>
void parseRedelegation(const T &n, cosmos::Redelegation &out)
{
    assert((n.HasMember(kDelegatorAddress)));
    assert((n.HasMember(kValidatorSrcAddress)));
    assert((n.HasMember(kValidatorDstAddress)));
    assert((n.HasMember("entries")));
    out.entries = std::list<cosmos::RedelegationEntry>();
    out.delegatorAddress = n[kDelegatorAddress].GetString();
    out.srcValidatorAddress = n[kValidatorSrcAddress].GetString();
    out.dstValidatorAddress = n[kValidatorDstAddress].GetString();
    if(n["entries"].IsArray()) {
        for (const auto &entry : n["entries"].GetArray()) {
            cosmos::RedelegationEntry parsedEntry;
            parseRedelegationEntry(entry.GetObject(), parsedEntry);
            out.entries.emplace_back(std::move(parsedEntry));
        }
    }
}

// Parse redelegation (/staking/redelegations?delegator={address})
// into cosmos::RedelegationList
// {
//   "height": "1347368",
//   "result": [
//     {
//       "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
//       "validator_src_address": "cosmosvaloper1sjllsnramtg3ewxqwwrwjxfgc4n4ef9u2lcnj0",
//       "validator_dst_address": "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn",
//       "entries": [
//         {
//           "creation_height": 1107334,
//           "completion_time": "2020-04-01T15:46:03.941380099Z",
//           "initial_balance": "1850",
//           "shares_dst": "1850.000000000000000000",
//           "balance": "1850"
//         }
//       ]
//     },
//     {
//       "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
//       "validator_src_address": "cosmosvaloper1ey69r37gfxvxg62sh4r0ktpuc46pzjrm873ae8",
//       "validator_dst_address": "cosmosvaloper1vf44d85es37hwl9f4h9gv0e064m0lla60j9luj",
//       "entries": [
//         {
//           "creation_height": 1178354,
//           "completion_time": "2020-04-07T13:08:44.940330001Z",
//           "initial_balance": "1000000",
//           "shares_dst": "1000000.000000000000000000",
//           "balance": "1000000"
//         }
//       ]
//     }
//   ]
// }
template <typename T>
void parseRedelegationList(const T &n, cosmos::RedelegationList &out)
{
    assert((n.HasMember("result")));
    out.clear();

    if (n["result"].IsArray()){
        for (const auto &redelegation : n["result"].GetArray()) {
            cosmos::Redelegation parsedRedelegation;
            parseRedelegation(redelegation.GetObject(), parsedRedelegation);
            out.emplace_back(std::make_shared<cosmos::Redelegation>(std::move(parsedRedelegation)));
        }
    }
}

// Parse distribution information (/distribution/validators/{cosmosvaloperXXX})
// {
//   "height": "1442636",
//   "result": {
//     "operator_address": "cosmos1dse76yk5jmj85jsd77ewsczc4k3u4s7az2mm8p",
//     "self_bond_rewards": [
//       {
//         "denom": "uatom",
//         "amount": "193463.275560183888483154"
//       }
//     ],
//     "val_commission": [
//       {
//         "denom": "uatom",
//         "amount": "320461734.567745567162136783"
//       }
//     ]
//   }
// }
template <typename T>
void parseDistInfo(const T &n, cosmos::ValidatorDistributionInformation &out)
{
    assert((n.HasMember("result")));
    auto resultObj = n["result"].GetObject();
    assert((resultObj.HasMember(kSelfBondRewards)));
    assert((resultObj.HasMember(kValCommission)));
    // NOTE : this function will only parse the first member of each array.
    // For the time being Cosmos is only used on CosmosHub, and the only
    // valid denom is "uatom" for those arrays

    // Do nothing if the array is "null"
    if (resultObj[kSelfBondRewards].IsArray()) {
        out.selfBondRewards =
            resultObj[kSelfBondRewards].GetArray()[0].GetObject()[kAmount].GetString();
    }
    // Do nothing if the array is "null"
    if (resultObj[kValCommission].IsArray()) {
        out.validatorCommission =
            resultObj[kValCommission].GetArray()[0].GetObject()[kAmount].GetString();
    }
}

// Parse signing information (/slashing/validators/{cosmosvalconspubXXX}/signing_info)
// {
//   "height": "1442169",
//   "result": {
//     "address": "",
//     "start_height": "0",
//     "index_offset": "4844166",
//     "jailed_until": "1970-01-01T00:00:00Z",
//     "tombstoned": false,
//     "missed_blocks_counter": "0"
//   }
// }
template <typename T>
void parseSignInfo(const T &n, cosmos::ValidatorSigningInformation &out)
{
    assert((n.HasMember("result")));
    auto resultObj = n["result"].GetObject();
    assert((resultObj.HasMember(kStartHeight)));
    assert((resultObj.HasMember(kIndexOffset)));
    assert((resultObj.HasMember(kJailedUntil)));
    assert((resultObj.HasMember(kTombstoned)));
    assert((resultObj.HasMember(kMissedBlocksCounter)));

    out.startHeight = std::stoi(resultObj[kStartHeight].GetString());
    out.indexOffset = std::stoi(resultObj[kIndexOffset].GetString());
    out.jailedUntil = DateUtils::fromJSON(resultObj[kJailedUntil].GetString());
    out.tombstoned = resultObj[kTombstoned].GetBool();
    out.missedBlocksCounter = std::stoi(resultObj[kMissedBlocksCounter].GetString());
}

}  // namespace rpcs_parsers
}  // namespace core
}  // namespace ledger

#undef COSMOS_PARSE_MSG_CONTENT
#endif  // LEDGER_CORE_RPCS_PARSERS_HPP
