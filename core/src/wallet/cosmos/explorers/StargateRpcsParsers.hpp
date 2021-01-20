/*
 *
 * StargateRpcsParsers.hpp
 * ledger-core
 *
 * Created by Gerry Agbobada on 2020-11-12.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#pragma once

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
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>

#define COSMOS_PARSE_MSG_CONTENT(MsgType)   \
    if (out.type == "cosmos-sdk/" #MsgType) \
        return parse##MsgType(contentNode, out.content);

namespace ledger {
namespace core {
using namespace cosmos::constants;
namespace stargate_rpcs_parsers {

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
        if (input.HasMember(kCoins) && input[kCoins].IsArray()) {
            const auto &inputValues = input[kCoins].GetArray();
            parseCoinVector(inputValues, out[index].coins);
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
        if (output.HasMember(kCoins) && output[kCoins].IsArray()) {
            const auto &outputValues = output[kCoins].GetArray();
            parseCoinVector(outputValues, out[index].coins);
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
    if (descriptionNode.HasMember(kSecurityContact) && descriptionNode[kSecurityContact].IsString()) {
        out.securityContact = optional<std::string>(descriptionNode[kSecurityContact].GetString());
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
void parseDelegation(const T &node, cosmos::Delegation &out, const api::Currency& currency)
{
    assert((node.HasMember(kDelegation)));
    auto delegationNode = node[kDelegation].GetObject();
    assert(delegationNode.HasMember(kDelegatorAddress));
    assert(delegationNode.HasMember(kValidatorAddress));
    out.delegatorAddress = delegationNode[kDelegatorAddress].GetString();
    out.validatorAddress = delegationNode[kValidatorAddress].GetString();

    assert((node.HasMember(kBalance)));
    auto delegationBalance = node[kBalance].GetObject();
    assert((delegationBalance.HasMember(kDenom)));
    assert((delegationBalance[kDenom].GetString() == currency.units[0].code));
    assert((delegationBalance.HasMember(kAmount)));
    out.delegatedAmount = BigInt::fromString(delegationBalance[kAmount].GetString());
}

template <typename T>
void parseReward(const T &rewardNode, cosmos::Reward &out, const api::Currency& currency)
{
    assert(rewardNode.HasMember(kValidatorAddress));
    assert(rewardNode.HasMember(kReward));
    out.validatorAddress = rewardNode[kValidatorAddress].GetString();
    // Do nothing if the array is "null" or empty
    if (rewardNode[kReward].IsArray() && rewardNode[kReward].GetArray().Size() > 0) {
        // In cosmoshub, the rewards are only in uatom, so there's only 1 entry in the array
        const auto &firstReward = rewardNode[kReward].GetArray()[0];
        // Debug safety assertion
        assert((firstReward.GetObject()[kDenom].GetString() == currency.units[0].code));

        parseCoin(
            firstReward,
            out.pendingReward);  // Assuming only one reward per validator
    }
}

/// Parse the relevant informations from /blocks/{latest || height} into cosmos::Block.
/// The `currencyName` is necessary context for LibCore but not present in the JSON,
/// therefore it is injected by the caller who knows the currency context
template <class T>
void parseBlock(const T &node, const api::Currency &currency, cosmos::Block &out)
{
    out.currencyName = currency.cosmosLikeNetworkParameters.value().Identifier;
    assert((node.HasMember(kBlockId)));
    out.hash = node[kBlockId].GetObject()[kHash].GetString();
    assert((node.HasMember(kBlock)));
    auto blockMetaNode = node[kBlock].GetObject();
    assert((blockMetaNode.HasMember(kHeader)));
    out.height =
        BigInt::fromString(blockMetaNode[kHeader].GetObject()[kHeight].GetString())
            .toUint64();
    out.time =
        DateUtils::fromJSON(blockMetaNode[kHeader].GetObject()[kTime].GetString());
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
        out.amount.assign((std::size_t)amountArray.Size(), cosmos::Coin());
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
    assert((accountNode.HasMember(kAccount)));
    const auto &node = accountNode[kAccount].GetObject();
    assert((node.HasMember(kGrpcType)));
    account.type = node[kGrpcType].GetString();

    assert((node.HasMember(kAccountNumber)));
    assert((node.HasMember(kSequence)));
    assert((node.HasMember(kAddress)));
    account.accountNumber = node[kAccountNumber].GetString();
    account.sequence = node[kSequence].GetString();
    account.address = node[kAddress].GetString();

    assert((node.HasMember(kPubKey)));
    // Pubkey is null for accounts that only received funds
    // "pub_key": {
    //   "@type": "/cosmos.crypto.secp256k1.PubKey",
    //   "key": "AnDGSF7VFQs9VKEceS6cWhfajxh/mmv+Mbu9bCM9BDzK"
    // },
    // or
    // "pub_key": null
    if (node[kPubKey].IsObject()) {
      auto PubKeyObj = node[kPubKey].GetObject();
      assert((PubKeyObj.HasMember(kGrpcType)));
      assert((PubKeyObj[kGrpcType].GetString() ==
              std::string("/cosmos.crypto.secp256k1.PubKey")));
      assert((PubKeyObj.HasMember(kKey)));

      auto b64publicKey = PubKeyObj[kKey].GetString();
      account.pubkey =
          CosmosLikeExtendedPublicKey::fromBase64(
              currencies::ATOM, b64publicKey, {}, api::CosmosCurve::SECP256K1,
              api::CosmosBech32Type::PUBLIC_KEY)
              ->toBech32();
    }
}

template <typename T>
void parseMsgSend(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgSend msg;

    assert((n.HasMember(kFromAddress)));
    assert((n.HasMember(kToAddress)));
    assert((n.HasMember(kAmount)));
    msg.fromAddress = n[kFromAddress].GetString();
    msg.toAddress = n[kToAddress].GetString();
    if (n[kAmount].IsArray()) {
        const auto &amountArray = n[kAmount].GetArray();
        parseCoinVector(amountArray, msg.amount);
    }
    out = msg;
}

template <typename T>
void parseMsgDelegate(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgDelegate msg;

    assert((n.HasMember(kDelegatorAddress)));
    assert((n.HasMember(kValidatorAddress)));
    assert((n.HasMember(kAmount)));
    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.validatorAddress = n[kValidatorAddress].GetString();
    parseCoin(n[kAmount].GetObject(), msg.amount);
    out = msg;
}

template <typename T>
void parseMsgBeginRedelegate(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgBeginRedelegate msg;

    assert((n.HasMember(kDelegatorAddress)));
    assert((n.HasMember(kValidatorSrcAddress)));
    assert((n.HasMember(kValidatorDstAddress)));
    assert((n.HasMember(kAmount)));
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

    assert((n.HasMember(kDelegatorAddress)));
    assert((n.HasMember(kValidatorAddress)));
    assert((n.HasMember(kAmount)));
    msg.delegatorAddress = n[kDelegatorAddress].GetString();
    msg.validatorAddress = n[kValidatorAddress].GetString();
    parseCoin(n[kAmount].GetObject(), msg.amount);
    out = msg;
}

template <typename T>
void parseMsgSubmitProposal(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgSubmitProposal msg;

    assert((n.HasMember(kProposer)));
    assert((n.HasMember(kContent)));
    assert((n.HasMember(kInitialDeposit)));
    msg.proposer = n[kProposer].GetString();
    parseProposalContent(n[kContent].GetObject(), msg.content);
    if (n[kInitialDeposit].IsArray()){
        const auto &amountArray = n[kInitialDeposit].GetArray();
        parseCoinVector(amountArray, msg.initialDeposit);
    }
    out = msg;
}

template <typename T>
void parseMsgVote(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgVote msg;

    assert((n.HasMember(kVoter)));
    assert((n.HasMember(kProposalId)));
    assert((n.HasMember(kOption)));
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

    assert((n.HasMember(kDepositor)));
    assert((n.HasMember(kProposalId)));
    assert((n.HasMember(kAmount)));
    msg.depositor = n[kDepositor].GetString();
    msg.proposalId = n[kProposalId].GetString();
    if (n[kAmount].IsArray()){
        const auto &amountArray = n[kAmount].GetArray();
        parseCoinVector(amountArray, msg.amount);
    }
    out = msg;
}

template <typename T>
void parseMsgWithdrawDelegationReward(const T &n, cosmos::MessageContent &out)
{
    cosmos::MsgWithdrawDelegationReward msg;

    assert((n.HasMember(kDelegatorAddress)));
    assert((n.HasMember(kValidatorAddress)));
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
        const auto &inputsArray = n[kInputs].GetArray();
        parseMultiSendInputs(inputsArray, msg.inputs);
    }

    // outputs: list<CosmosLikeMultiSendOutput>;
    if(n[kOutputs].IsArray()) {
        auto outputsArray = n[kOutputs].GetArray();
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
        pubkey = "";
        return;
        // throw make_exception(
        //     api::ErrorCode::ILLEGAL_STATE,
        //     "The received signer pubkey does not have signature information at key {}",
        //     kSignatures);
    }

    const auto firstSignature = node[kSignatures].GetArray()[0].GetObject();
    if (firstSignature.HasMember(kPubKey) &&
        firstSignature[kPubKey].GetObject().HasMember(kValue)) {
        pubkey = firstSignature[kPubKey].GetObject()[kValue].GetString();
    }
}

template <class T>
void parseTransaction(const T &node, cosmos::Transaction &transaction, const api::Currency& currency)
{
    assert((node.HasMember(kTx)));
    const auto &tNode = node[kTx].GetObject();
    assert((tNode.HasMember(kValue)));
    const auto &vNode = tNode[kValue].GetObject();
    assert((node.HasMember(kTxHash)));
    transaction.hash = node[kTxHash].GetString();
    if (node.HasMember(kHeight)) {
        cosmos::Block block;
        block.height = BigInt::fromString(node[kHeight].GetString()).toUint64();
        block.currencyName = currency.name;
        transaction.block = block;
    }
    if (node.HasMember(kGasUsed)) {
        transaction.gasUsed = Option<BigInt>(BigInt::fromString(node[kGasUsed].GetString()));
    }

    assert((node.HasMember(kLogs)));
    // If the transaction does not have logs, we need to allocate memory still
    // for parseMessage call. To clean up by changing parseMessage logic.
    bool hasLogs = false;
    if (node[kLogs].IsArray()) {
        hasLogs = true;
        auto msgIndex = 0;
        for (const auto &lNode : node[kLogs].GetArray()) {
            cosmos::MessageLog log;
            log.success = true;
            log.log = fmt::format("Hardcoded as success at %s:%d", __FILE__, __LINE__);
            log.messageIndex = msgIndex;
            transaction.logs.emplace_back(log);
            msgIndex++;
        }
    } else {
        // Assuming the transaction failed here. We create a fake log entry with failure
        // using the raw_log exclusively
        assert((vNode.HasMember(kMessage)));
        for (size_t i = 0; i < vNode[kMessage].GetArray().Size(); i++) {
          cosmos::MessageLog log;
          log.success = false;
          log.log = node[kRawLog].GetString();
          log.messageIndex = i;
          transaction.logs.emplace_back(log);
        }
    }
    assert((node.HasMember(kTimestamp)));
    transaction.timestamp = DateUtils::fromJSON(node[kTimestamp].GetString());

    if (vNode.HasMember(kMemo)) {
        transaction.memo = vNode[kMemo].GetString();
    }

    assert((vNode.HasMember(kMessage)));
    if (vNode[kMessage].IsArray()) {
        const auto &msgArray = vNode[kMessage].GetArray();
        transaction.messages.assign((std::size_t)msgArray.Size(), cosmos::Message());
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

    assert((n.HasMember(kUnbondingHeight)));
    out.unbondingHeight = BigInt::fromString(n[kUnbondingHeight].GetString()).toInt();

    assert((n.HasMember(kUnbondingTime)));
    if (out.unbondingHeight == 0) {
        out.unbondingTime = optional<std::chrono::system_clock::time_point>();
    }
    else {
        out.unbondingTime = optional<std::chrono::system_clock::time_point>(
            DateUtils::fromJSON(n[kUnbondingTime].GetString()));
    }

    assert((n.HasMember(kMinSelfDelegation)));
    out.minSelfDelegation = n[kMinSelfDelegation].GetString();

    assert((n.HasMember(kJailed)));
    out.jailed = n[kJailed].GetBool();

    // Note : we ignore delegator_shares, as it seems the actual
    // voting power is the tokens key, in uatom
    assert((n.HasMember(kTokens)));
    out.votingPower = n[kTokens].GetString();

    assert((n.HasMember(kOperatorAddress)));
    out.operatorAddress = n[kOperatorAddress].GetString();

    assert((n.HasMember(kConsensusPubkey)));
    // "consensus_pubkey": {
    //         "@type": "/cosmos.crypto.ed25519.PubKey",
    //         "key": "B33RB8LHddbd+KXO1jPJHDpelfOgOGJP5yDEQYEWdyY="
    //       }

    // Debug sanity assertions
    auto consPubKeyObj = n[kConsensusPubkey].GetObject();
    assert((consPubKeyObj.HasMember(kGrpcType)));
    assert((consPubKeyObj[kGrpcType].GetString() == std::string("/cosmos.crypto.ed25519.PubKey")));
    assert((consPubKeyObj.HasMember(kKey)));

    auto b64publicKey = consPubKeyObj[kKey].GetString();
    out.consensusPubkey = CosmosLikeExtendedPublicKey::fromBase64(
        currencies::ATOM,
        b64publicKey,
        {},
        api::CosmosCurve::ED25519,
        api::CosmosBech32Type::PUBLIC_KEY_VAL_CONS)->toBech32();

    assert((n.HasMember(kStatus)));
    out.activeStatus = n[kStatus].GetString();
}

//  Parse an unbonding entry from (/{namespace}/staking/{version}/delegators/{address}/unbonding_delegations)
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

// Parse unbonding result from (/{namespace}/staking/{version}/delegators/{address}/unbonding_delegations)
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
    assert((n.HasMember(kEntries)));
    out.entries = std::list<cosmos::UnbondingEntry>();
    out.delegatorAddress = n[kDelegatorAddress].GetString();
    out.validatorAddress = n[kValidatorAddress].GetString();
    if (n[kEntries].IsArray()) {
        for (const auto &entry : n[kEntries].GetArray()) {
            cosmos::UnbondingEntry parsedEntry;
            parseUnbondingEntry(entry.GetObject(), parsedEntry);
            out.entries.emplace_back(std::move(parsedEntry));
        }
    }
}

// Parse unbonding result
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
    assert((n.HasMember(kUnbondingResponses)));
    out.clear();

    if(n[kUnbondingResponses].IsArray()){
        for (const auto &unbonding : n[kUnbondingResponses].GetArray()) {
            cosmos::Unbonding parsedUnbonding;
            parseUnbonding(unbonding.GetObject(), parsedUnbonding);
            out.emplace_back(std::make_shared<cosmos::Unbonding>(std::move(parsedUnbonding)));
        }
    }
}

// Parse redelegation
// into cosmos::RedelegationEntry
// {
//   "redelegation_entry": {
//     "creation_height": 502590,
//     "completion_time": "2021-01-01T10:56:15.231455556Z",
//     "initial_balance": "2000000",
//     "shares_dst": "2000000.000000000000000000"
//   },
//   "balance": "2000000"
// }
template <typename T>
void parseRedelegationEntry(const T &n, cosmos::RedelegationEntry &out)
{
    assert((n.HasMember(kRedelegationEntry) && n[kRedelegationEntry].IsObject()));
    auto entryNode = n[kRedelegationEntry].GetObject();
    assert((entryNode.HasMember(kCreationHeight)));
    assert((entryNode.HasMember(kCompletionTime)));
    assert((entryNode.HasMember(kInitialBalance)));
    assert((n.HasMember(kBalance)));

    out.creationHeight = BigInt(entryNode[kCreationHeight].GetInt());
    out.completionTime = DateUtils::fromJSON(entryNode[kCompletionTime].GetString());
    out.initialBalance = BigInt::fromString(entryNode[kInitialBalance].GetString());
    out.balance = BigInt::fromString(n[kBalance].GetString());
}

// Parse redelegation
// into cosmos::Redelegation
//
// {
//   "redelegation": {
//     "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
//     "validator_src_address": "cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu",
//     "validator_dst_address": "cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk",
//     "entries": null
//   },
//   "entries": [
//     {
//       "redelegation_entry": {
//         "creation_height": 502590,
//         "completion_time": "2021-01-01T10:56:15.231455556Z",
//         "initial_balance": "2000000",
//         "shares_dst": "2000000.000000000000000000"
//       },
//       "balance": "2000000"
//     }
//   ]
// }
template <typename T>
void parseRedelegation(const T &n, cosmos::Redelegation &out)
{
    assert((n.HasMember(kRedelegation) && n[kRedelegation].IsObject()));
    auto redelegationNode = n[kRedelegation].GetObject();
    assert((redelegationNode.HasMember(kDelegatorAddress)));
    assert((redelegationNode.HasMember(kValidatorSrcAddress)));
    assert((redelegationNode.HasMember(kValidatorDstAddress)));
    assert((n.HasMember(kEntries)));
    out.entries = std::list<cosmos::RedelegationEntry>();
    out.delegatorAddress = redelegationNode[kDelegatorAddress].GetString();
    out.srcValidatorAddress = redelegationNode[kValidatorSrcAddress].GetString();
    out.dstValidatorAddress = redelegationNode[kValidatorDstAddress].GetString();
    if(n[kEntries].IsArray()) {
        for (const auto &entry : n[kEntries].GetArray()) {
            cosmos::RedelegationEntry parsedEntry;
            parseRedelegationEntry(entry.GetObject(), parsedEntry);
            out.entries.emplace_back(std::move(parsedEntry));
        }
    }
}

// Parse redelegation
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
    assert((n.HasMember(kRedelegationResponses)));
    out.clear();

    if (n[kRedelegationResponses].IsArray()){
        for (const auto &redelegation : n[kRedelegationResponses].GetArray()) {
            cosmos::Redelegation parsedRedelegation;
            parseRedelegation(redelegation.GetObject(), parsedRedelegation);
            out.emplace_back(std::make_shared<cosmos::Redelegation>(std::move(parsedRedelegation)));
        }
    }
}

// Parse signing information
// {
//   "val_signing_info": {
//     "address": "cosmosvalcons14es7cmaqg5xxxfeg3w2xuge63p5rc3u2vt8ym4",
//     "start_height": "75270",
//     "index_offset": "11836",
//     "jailed_until": "1970-01-01T00:00:00Z",
//     "tombstoned": false,
//     "missed_blocks_counter": "0"
//   }
// }
template <typename T>
void parseSignInfo(const T &n, cosmos::ValidatorSigningInformation &out)
{
    assert((n.HasMember(kValSignInfos)));
    auto resultObj = n[kValSignInfos].GetObject();
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
