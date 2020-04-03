/*
 *
 * CosmosLikeMessage
 *
 * Created by El Khalil Bellakrid on  14/06/2019.
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

#include <fmt/format.h>

#include <algorithm>
#include <api/ErrorCode.hpp>
#include <api/enum_from_string.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosLikeMessage.hpp>
#include <functional>

namespace ledger {
namespace core {

using namespace cosmos::constants;

namespace {

// add a `CosmosLikeContent` to a `rapidjson::Value`
inline auto addContent(
    api::CosmosLikeContent const& content,
    rapidjson::Value& json,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value jsonContent(rapidjson::kObjectType);
    rapidjson::Value jsonString(rapidjson::kStringType);

    // api::CosmosLikeContent::type
    jsonString.SetString(
        content.type.c_str(), static_cast<rapidjson::SizeType>(content.type.length()), allocator);
    jsonContent.AddMember(kType, jsonString, allocator);

    // api::CosmosLikeContent::title
    jsonString.SetString(
        content.title.c_str(), static_cast<rapidjson::SizeType>(content.title.length()), allocator);
    jsonContent.AddMember(kTitle, jsonString, allocator);

    // api::CosmosLikeContent::description
    jsonString.SetString(
        content.description.c_str(),
        static_cast<rapidjson::SizeType>(content.description.length()),
        allocator);
    jsonContent.AddMember(kDescription, jsonString, allocator);

    json.AddMember(kContent, jsonContent, allocator);
}

// add a `CosmosLikeAmount` to a `rapidjson::Value`
inline auto addAmount(
    const std::string& key,
    api::CosmosLikeAmount const& amount,
    rapidjson::Value& json,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value jsonAmount(rapidjson::kObjectType);
    rapidjson::Value jsonString(rapidjson::kStringType);

    // api::CosmosLikeAmount::amount
    jsonString.SetString(
        amount.amount.c_str(), static_cast<rapidjson::SizeType>(amount.amount.length()), allocator);
    jsonAmount.AddMember(kAmount, jsonString, allocator);

    // api::CosmosLikeAmount::denom
    jsonString.SetString(
        amount.denom.c_str(), static_cast<rapidjson::SizeType>(amount.denom.length()), allocator);
    jsonAmount.AddMember(kDenom, jsonString, allocator);

    json.AddMember(rapidjson::Value(key.c_str(), allocator).Move(), jsonAmount, allocator);
}

// add an array of `CosmosLikeAmount` to a `rapidjson::Value`
inline auto addAmounts(
    std::vector<api::CosmosLikeAmount> const& amounts,
    rapidjson::Value& jsonAmounts,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value jsonString(rapidjson::kStringType);
    for (auto amount : amounts) {
        rapidjson::Value jsonAmount(rapidjson::kObjectType);

        // api::CosmosLikeAmount::amount
        jsonString.SetString(
            amount.amount.c_str(),
            static_cast<rapidjson::SizeType>(amount.amount.length()),
            allocator);
        jsonAmount.AddMember(kAmount, jsonString, allocator);

        // api::CosmosLikeAmount::denom
        jsonString.SetString(
            amount.denom.c_str(),
            static_cast<rapidjson::SizeType>(amount.denom.length()),
            allocator);
        jsonAmount.AddMember(kDenom, jsonString, allocator);

        jsonAmounts.PushBack(jsonAmount, allocator);
    }
}

// add a string to a rapidjson::Value
inline auto addString(
    const std::string& key,
    std::string const& value,
    rapidjson::Value& jsonValue,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value jsonString(rapidjson::kStringType);
    jsonString.SetString(
        value.c_str(), static_cast<rapidjson::SizeType>(value.length()), allocator);
    jsonValue.AddMember(rapidjson::Value(key.c_str(), allocator).Move(), jsonString, allocator);
}

// add an optional string to a rapidjson::Value
inline auto addOptionalString(
    const std::string& key,
    optional<std::string> const& value,
    rapidjson::Value& jsonValue,
    rapidjson::Document::AllocatorType& allocator) {
    if (value) {
        return addString(key, value.value(), jsonValue, allocator);
    }
}

}  // namespace

CosmosLikeMessage::CosmosLikeMessage(const cosmos::Message& msg) : _msgData(msg) {}

CosmosLikeMessage::CosmosLikeMessage(const std::shared_ptr<OperationApi>& baseOp) {
    // Necessary for design/mod backport
    // Basically we need to separate the code from CosmosLikeOperationQuery in 2 parts.
    // The part regarding the Transaction information is in OperationQuery::inflateCosmosLikeTransaction.
    // We still need to construct a message from an OperationApi. This is done here.
                // ledger::core::cosmos::Message msg;

                // std::string msgUid;
                // sql << "SELECT msg.uid "
                //     "FROM cosmos_messages AS msg "
                //     "LEFT JOIN cosmos_operations AS op ON op.message_uid = msg.uid "
                //     "WHERE op.uid = :uid",
                //     soci::use(operation.getUid()),
                //     soci::into(msgUid);

                // CosmosLikeTransactionDatabaseHelper::getMessageByUid(sql, msgUid, msg);

                // setRawData(msg);
}

void CosmosLikeMessage::setRawData(const cosmos::Message& msgData) { _msgData = msgData; }

const cosmos::Message& CosmosLikeMessage::getRawData() const { return _msgData; }

const std::string& CosmosLikeMessage::getFromAddress() const
{
    switch (getMessageType()) {
        case api::CosmosLikeMsgType::MSGSEND:
            return boost::get<cosmos::MsgSend>(_msgData.content).fromAddress;
        case api::CosmosLikeMsgType::MSGDELEGATE:
            return boost::get<cosmos::MsgDelegate>(_msgData.content).delegatorAddress;
        case api::CosmosLikeMsgType::MSGUNDELEGATE:
            return boost::get<cosmos::MsgUndelegate>(_msgData.content).delegatorAddress;
        case api::CosmosLikeMsgType::MSGBEGINREDELEGATE:
            return boost::get<cosmos::MsgBeginRedelegate>(_msgData.content).delegatorAddress;
        case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD:
            return boost::get<cosmos::MsgWithdrawDelegatorReward>(_msgData.content).delegatorAddress;
        case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD:
            return boost::get<cosmos::MsgWithdrawDelegationReward>(_msgData.content).delegatorAddress;
        default:
            throw Exception(api::ErrorCode::UNSUPPORTED_OPERATION, "message type not handled");
    }
}

api::CosmosLikeMsgType CosmosLikeMessage::getMessageType() const {
    return cosmos::stringToMsgType(_msgData.type.c_str());
}

std::string CosmosLikeMessage::getRawMessageType() const { return _msgData.type; }

// Note : this Json has not been sorted yet
// This doesn't follow the spec
// https://github.com/cosmos/ledger-cosmos-app/blob/master/docs/TXSPEC.md
// but we defer the sorting to the TransactionApi::serialize() method
rapidjson::Value CosmosLikeMessage::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value json(rapidjson::kObjectType);

    rapidjson::Value jsonString(rapidjson::kStringType);

    // cosmos::Message::type
    jsonString.SetString(
        _msgData.type.c_str(), static_cast<rapidjson::SizeType>(_msgData.type.length()), allocator);
    json.AddMember(kType, jsonString, allocator);

    rapidjson::Value jsonContent(rapidjson::kObjectType);
    if (_msgData.type == kMsgSend) {
        const auto& content = boost::get<cosmos::MsgSend>(_msgData.content);

        // cosmos::MsgSend::fromAddress
        addString(kFromAddress, content.fromAddress, jsonContent, allocator);

        // cosmos::MsgSend::toAddress
        addString(kToAddress, content.toAddress, jsonContent, allocator);

        // cosmos::MsgSend::amount
        rapidjson::Value jsonAmounts(rapidjson::kArrayType);
        addAmounts(content.amount, jsonAmounts, allocator);
        jsonContent.AddMember(kAmount, jsonAmounts, allocator);

    } else if (_msgData.type == kMsgDelegate) {
        const auto& content = boost::get<cosmos::MsgDelegate>(_msgData.content);

        // cosmos::MsgDelegate::delegatorAddress
        addString(kDelegatorAddress, content.delegatorAddress, jsonContent, allocator);

        // cosmos::MsgDelegate::validatorAddress
        addString(kValidatorAddress, content.validatorAddress, jsonContent, allocator);

        // cosmos::MsgDelegate::amount
        addAmount(kAmount, content.amount, jsonContent, allocator);

    } else if (_msgData.type == kMsgUndelegate) {
        const auto& content = boost::get<cosmos::MsgUndelegate>(_msgData.content);

        // cosmos::MsgUndelegate::delegatorAddress
        addString(kDelegatorAddress, content.delegatorAddress, jsonContent, allocator);

        // cosmos::MsgUndelegate::validatorAddress
        addString(kValidatorAddress, content.validatorAddress, jsonContent, allocator);

        // cosmos::MsgUndelegate::amount
        addAmount(kAmount, content.amount, jsonContent, allocator);

    } else if (_msgData.type == kMsgBeginRedelegate) {
        const auto& content = boost::get<cosmos::MsgBeginRedelegate>(_msgData.content);

        // cosmos::MsgBeginRedelegate::delegatorAddress
        addString(kDelegatorAddress, content.delegatorAddress, jsonContent, allocator);

        // cosmos::MsgBeginRedelegate::validatorSourceAddress
        addString(kValidatorSrcAddress, content.validatorSourceAddress, jsonContent, allocator);

        // cosmos::MsgBeginRedelegate::validatorDestinationAddress
        addString(
            kValidatorDstAddress, content.validatorDestinationAddress, jsonContent, allocator);

        // cosmos::MsgBeginRedelegate::amount
        addAmount(kAmount, content.amount, jsonContent, allocator);

    } else if (_msgData.type == kMsgSubmitProposal) {
        const auto& content = boost::get<cosmos::MsgSubmitProposal>(_msgData.content);

        // cosmos::MsgSubmitProposal::content
        addContent(content.content, jsonContent, allocator);

        // cosmos::MsgSubmitProposal::proposer
        addString(kProposer, content.proposer, jsonContent, allocator);

        // cosmos::MsgSubmitProposal::initialDeposit
        rapidjson::Value jsonAmounts(rapidjson::kArrayType);
        addAmounts(content.initialDeposit, jsonAmounts, allocator);
        jsonContent.AddMember(kInitialDeposit, jsonAmounts, allocator);

    } else if (_msgData.type == kMsgVote) {
        const auto& content = boost::get<cosmos::MsgVote>(_msgData.content);

        // cosmos::MsgVote::voter
        addString(kVoter, content.voter, jsonContent, allocator);

        // cosmos::MsgVote::proposalId
        addString(kProposalId, content.proposalId, jsonContent, allocator);

        // cosmos::MsgVote::option
        auto option = cosmos::voteOptionToChars(content.option);
        addString(kOption, option, jsonContent, allocator);

    } else if (_msgData.type == kMsgDeposit) {
        const auto& content = boost::get<cosmos::MsgDeposit>(_msgData.content);

        // cosmos::MsgDeposit::depositor
        addString(kDepositor, content.depositor, jsonContent, allocator);

        // cosmos::MsgDeposit::proposalId
        addString(kProposalId, content.proposalId, jsonContent, allocator);

        // cosmos::MsgDeposit::amount
        rapidjson::Value jsonAmounts(rapidjson::kArrayType);
        addAmounts(content.amount, jsonAmounts, allocator);
        jsonContent.AddMember(kAmount, jsonAmounts, allocator);

    } else if (_msgData.type == kMsgWithdrawDelegationReward) {
        const auto& content = boost::get<cosmos::MsgWithdrawDelegationReward>(_msgData.content);

        // cosmos::MsgWithdrawDelegationReward::delegatorAddress
        addString(kDelegatorAddress, content.delegatorAddress, jsonContent, allocator);

        // cosmos::MsgWithdrawDelegationReward::validatorAddress
        addString(kValidatorAddress, content.validatorAddress, jsonContent, allocator);

    } else if (_msgData.type == kMsgMultiSend) {
        const auto& content = boost::get<cosmos::MsgMultiSend>(_msgData.content);

        rapidjson::Value jsonInputs(rapidjson::kArrayType);
        rapidjson::Value jsonOutputs(rapidjson::kArrayType);

        // Serialize MultiSendInput (with fromAddress field)
        for (auto& input : content.inputs) {
            auto newInput = rapidjson::Value(rapidjson::kObjectType);

            addString(kAddress, input.fromAddress, newInput, allocator);

            auto newInputAmounts = rapidjson::Value(rapidjson::kArrayType);
            addAmounts(input.coins, newInputAmounts, allocator);
            newInput.AddMember(kCoins, newInputAmounts, allocator);

            jsonInputs.PushBack(newInput, allocator);
        }

        // Serialize MultiSendOutput (with toAddress field)
        for (auto& output : content.outputs) {
            auto newOutput = rapidjson::Value(rapidjson::kObjectType);

            addString(kAddress, output.toAddress, newOutput, allocator);

            auto newOutputAmounts = rapidjson::Value(rapidjson::kArrayType);
            addAmounts(output.coins, newOutputAmounts, allocator);
            newOutput.AddMember(kCoins, newOutputAmounts, allocator);

            jsonOutputs.PushBack(newOutput, allocator);
        }

        jsonContent.AddMember(kInputs, jsonInputs, allocator);
        jsonContent.AddMember(kOutputs, jsonOutputs, allocator);

    } else if (_msgData.type == kMsgCreateValidator) {
        const auto& content = boost::get<cosmos::MsgCreateValidator>(_msgData.content);

        auto jsonDesc = rapidjson::Value(rapidjson::kObjectType);
        addString(kMoniker, content.description.moniker, jsonDesc, allocator);
        addOptionalString(kIdentity, content.description.identity, jsonDesc, allocator);
        addOptionalString(kWebsite, content.description.website, jsonDesc, allocator);
        addOptionalString(kDetails, content.description.details, jsonDesc, allocator);
        jsonContent.AddMember(kDescription, jsonDesc, allocator);

        auto jsonCommission = rapidjson::Value(rapidjson::kObjectType);
        addString(kCommissionRate, content.commission.rates.rate, jsonCommission, allocator);
        addString(kCommissionMaxRate, content.commission.rates.maxRate, jsonCommission, allocator);
        addString(
            kCommissionMaxChangeRate,
            content.commission.rates.maxChangeRate,
            jsonCommission,
            allocator);
        auto date_str = DateUtils::toJSON(content.commission.updateTime);
        addString(kUpdateTime, date_str, jsonCommission, allocator);
        jsonContent.AddMember(kCommission, jsonCommission, allocator);

        addString(kMinSelfDelegation, content.minSelfDelegation, jsonContent, allocator);
        addString(kDelegatorAddress, content.delegatorAddress, jsonContent, allocator);
        addString(kValidatorAddress, content.validatorAddress, jsonContent, allocator);
        addString(kPubKey, content.pubkey, jsonContent, allocator);
        addAmount(kValue, content.value, jsonContent, allocator);

    } else if (_msgData.type == kMsgEditValidator) {
        const auto& content = boost::get<cosmos::MsgEditValidator>(_msgData.content);

        if (content.description) {
            auto description = content.description.value();
            auto jsonDesc = rapidjson::Value(rapidjson::kObjectType);
            addString(kMoniker, description.moniker, jsonDesc, allocator);
            addOptionalString(kIdentity, description.identity, jsonDesc, allocator);
            addOptionalString(kWebsite, description.website, jsonDesc, allocator);
            addOptionalString(kDetails, description.details, jsonDesc, allocator);
            jsonContent.AddMember(kDescription, jsonDesc, allocator);
        }

        addString(kValidatorAddress, content.validatorAddress, jsonContent, allocator);
        addOptionalString(kEditValCommissionRate, content.commissionRate, jsonContent, allocator);
        addOptionalString(kMinSelfDelegation, content.minSelfDelegation, jsonContent, allocator);

    } else if (_msgData.type == kMsgSetWithdrawAddress) {
        const auto& content = boost::get<cosmos::MsgSetWithdrawAddress>(_msgData.content);

        addString(kWithdrawAddress, content.withdrawAddress, jsonContent, allocator);
        addString(kDelegatorAddress, content.delegatorAddress, jsonContent, allocator);

    } else if (_msgData.type == kMsgWithdrawDelegatorReward) {
        const auto& content = boost::get<cosmos::MsgWithdrawDelegatorReward>(_msgData.content);

        addString(kValidatorAddress, content.validatorAddress, jsonContent, allocator);
        addString(kDelegatorAddress, content.delegatorAddress, jsonContent, allocator);

    } else if (_msgData.type == kMsgWithdrawValidatorCommission) {
        const auto& content = boost::get<cosmos::MsgWithdrawValidatorCommission>(_msgData.content);

        addString(kValidatorAddress, content.validatorAddress, jsonContent, allocator);

    } else if (_msgData.type == kMsgUnjail) {
        const auto& content = boost::get<cosmos::MsgUnjail>(_msgData.content);

        addString(kValidatorAddress, content.validatorAddress, jsonContent, allocator);

    }  // else the content stays as is.

    json.AddMember(kValue, jsonContent, allocator);

    return json;
}

// TODO [refacto] Templatize all the following functions
// Note that templating doesn't help much, as these names are exposed through djinni,
// and need to be defined anyway.

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgSend(
    const api::CosmosLikeMsgSend& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgSend;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgSend api::CosmosLikeMessage::unwrapMsgSend(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGSEND) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgSend");
    }
    return boost::get<cosmos::MsgSend>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgDelegate(
    const api::CosmosLikeMsgDelegate& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgDelegate;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgDelegate api::CosmosLikeMessage::unwrapMsgDelegate(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGDELEGATE) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgDelegate");
    }
    return boost::get<cosmos::MsgDelegate>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgUndelegate(
    const api::CosmosLikeMsgUndelegate& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgUndelegate;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgUndelegate api::CosmosLikeMessage::unwrapMsgUndelegate(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGUNDELEGATE) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgUndelegate");
    }
    return boost::get<cosmos::MsgUndelegate>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgBeginRedelegate(
    const api::CosmosLikeMsgBeginRedelegate& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgBeginRedelegate;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgBeginRedelegate api::CosmosLikeMessage::unwrapMsgBeginRedelegate(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGBEGINREDELEGATE) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgBeginRedelegate");
    }
    return boost::get<cosmos::MsgBeginRedelegate>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgSubmitProposal(
    const api::CosmosLikeMsgSubmitProposal& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgSubmitProposal;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgSubmitProposal api::CosmosLikeMessage::unwrapMsgSubmitProposal(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGSUBMITPROPOSAL) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgSubmitProposal");
    }
    return boost::get<cosmos::MsgSubmitProposal>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgVote(
    const api::CosmosLikeMsgVote& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgVote;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgVote api::CosmosLikeMessage::unwrapMsgVote(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGVOTE) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgVote");
    }
    return boost::get<cosmos::MsgVote>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgDeposit(
    const api::CosmosLikeMsgDeposit& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgDeposit;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgDeposit api::CosmosLikeMessage::unwrapMsgDeposit(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGDEPOSIT) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgDeposit");
    }
    return boost::get<cosmos::MsgDeposit>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgWithdrawDelegationReward(
    const api::CosmosLikeMsgWithdrawDelegationReward& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgWithdrawDelegationReward;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgWithdrawDelegationReward
api::CosmosLikeMessage::unwrapMsgWithdrawDelegationReward(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD) {
        throw Exception(
            api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgWithdrawDelegationReward");
    }
    return boost::get<cosmos::MsgWithdrawDelegationReward>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgMultiSend(
    const api::CosmosLikeMsgMultiSend& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgMultiSend;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgMultiSend api::CosmosLikeMessage::unwrapMsgMultiSend(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGMULTISEND) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgMultiSend");
    }
    return boost::get<cosmos::MsgMultiSend>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgCreateValidator(
    const api::CosmosLikeMsgCreateValidator& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgCreateValidator;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgCreateValidator api::CosmosLikeMessage::unwrapMsgCreateValidator(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGCREATEVALIDATOR) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgCreateValidator");
    }
    return boost::get<cosmos::MsgCreateValidator>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgEditValidator(
    const api::CosmosLikeMsgEditValidator& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgEditValidator;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgEditValidator api::CosmosLikeMessage::unwrapMsgEditValidator(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGEDITVALIDATOR) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgEditValidator");
    }
    return boost::get<cosmos::MsgEditValidator>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgSetWithdrawAddress(
    const api::CosmosLikeMsgSetWithdrawAddress& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgSetWithdrawAddress;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgSetWithdrawAddress api::CosmosLikeMessage::unwrapMsgSetWithdrawAddress(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgSetWithdrawAddress");
    }
    return boost::get<cosmos::MsgSetWithdrawAddress>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgWithdrawDelegatorReward(
    const api::CosmosLikeMsgWithdrawDelegatorReward& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgWithdrawDelegatorReward;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgWithdrawDelegatorReward api::CosmosLikeMessage::unwrapMsgWithdrawDelegatorReward(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD) {
        throw Exception(
            api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgWithdrawDelegatorReward");
    }
    return boost::get<cosmos::MsgWithdrawDelegatorReward>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgWithdrawValidatorCommission(
    const api::CosmosLikeMsgWithdrawValidatorCommission& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgWithdrawValidatorCommission;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgWithdrawValidatorCommission
api::CosmosLikeMessage::unwrapMsgWithdrawValidatorCommission(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION) {
        throw Exception(
            api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgWithdrawValidatorCommission");
    }
    return boost::get<cosmos::MsgWithdrawValidatorCommission>(cosmosMsg->getRawData().content);
}

std::shared_ptr<api::CosmosLikeMessage> api::CosmosLikeMessage::wrapMsgUnjail(
    const api::CosmosLikeMsgUnjail& msgContent) {
    cosmos::Message msg;
    msg.type = kMsgUnjail;
    msg.content = msgContent;
    return std::make_shared<::ledger::core::CosmosLikeMessage>(msg);
}

api::CosmosLikeMsgUnjail api::CosmosLikeMessage::unwrapMsgUnjail(
    const std::shared_ptr<api::CosmosLikeMessage>& msg) {
    auto cosmosMsg = std::dynamic_pointer_cast<ledger::core::CosmosLikeMessage>(msg);
    if (cosmosMsg->getMessageType() != api::CosmosLikeMsgType::MSGUNJAIL) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "unable to unwrap MsgUnjail");
    }
    return boost::get<cosmos::MsgUnjail>(cosmosMsg->getRawData().content);
}
}  // namespace core
}  // namespace ledger
