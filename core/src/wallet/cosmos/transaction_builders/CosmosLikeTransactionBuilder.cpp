/*
 *
 * CosmosLikeTransactionBuilder
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

#include <wallet/cosmos/transaction_builders/CosmosLikeTransactionBuilder.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <math/BigInt.h>
#include <api/enum_from_string.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <bytes/BytesReader.h>
#include <utils/DateUtils.hpp>

#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <wallet/common/Amount.h>
#include <api/CosmosLikeMessage.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <api/CosmosLikeMsgType.hpp>
#include <api/CosmosLikeMsgSend.hpp>
#include <api/CosmosLikeMsgDelegate.hpp>
#include <api/CosmosLikeMsgUndelegate.hpp>
#include <api/CosmosLikeMsgBeginRedelegate.hpp>
#include <api/CosmosLikeMsgSubmitProposal.hpp>
#include <api/CosmosLikeMsgVote.hpp>
#include <api/CosmosLikeMsgDeposit.hpp>
#include <api/CosmosLikeMsgWithdrawDelegationReward.hpp>

#include <wallet/cosmos/CosmosLikeMessage.hpp> // Included only for the Unsupported case

using namespace rapidjson;
namespace ledger {
    namespace core {

        using namespace cosmos::constants;

        using Object = GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>>::Object;

        namespace {
            auto getString(const Object& object, const char *fieldName) {
                if (!object.HasMember(fieldName)) {
                    throw Exception(api::ErrorCode::INVALID_ARGUMENT, fmt::format("Error while getting {} from rawTransaction : non existing key", fieldName));
                }
                if (!object[fieldName].IsString()) {
                    throw Exception(api::ErrorCode::INVALID_ARGUMENT, fmt::format("Error while getting string {} from rawTransaction : not a string", fieldName));
                }
                return object[fieldName].GetString();
            }

            // auto getObject(const Object& object, const char *fieldName) {
            //     if (!object[fieldName].IsObject()) {
            //         throw Exception(api::ErrorCode::INVALID_ARGUMENT, fmt::format("Error while getting object {} from rawTransaction", fieldName));
            //     }
            //     return object[fieldName].GetObject();
            // }

            cosmos::MsgSend buildMsgSendFromRawMessage(Object const& object) {
                std::vector<cosmos::Coin> amounts;

                if (!object[kValue][kAmount].IsArray() && !object[kValue][kAmount].IsObject()) {
                    return {
                    "",
                    "",
                    amounts
                };
                } else if (object[kValue][kAmount].IsObject()) {
                    auto amountObject = object[kValue][kAmount].GetObject();
                    amounts.push_back(cosmos::Coin{
                        getString(amountObject, kAmount),
                        getString(amountObject, kDenom)
                    });
                    return {
                    getString(object[kValue].GetObject(), kFromAddress),
                    getString(object[kValue].GetObject(), kToAddress),
                    amounts
                };
                } else {

                    // We are sure object[kValue][kAmount] is an array here

                    // the size of the array of amount should be frequently equals to one
                    amounts.reserve(object[kValue][kAmount].GetArray().Size());

                    for (auto& amount : object[kValue][kAmount].GetArray()) {
                        if (amount.IsObject()) {
                            auto amountObject = amount.GetObject();
                            amounts.push_back(cosmos::Coin{
                                getString(amountObject, kAmount),
                                getString(amountObject, kDenom)
                            });
                        }
                    }

                    return {
                        getString(object[kValue].GetObject(), kFromAddress),
                        getString(object[kValue].GetObject(), kToAddress),
                        amounts
                    };
                }
            }

            cosmos::MsgDelegate buildMsgDelegateFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                auto amountObject = valueObject[kAmount].GetObject();

                return {
                    getString(valueObject, kDelegatorAddress),
                    getString(valueObject, kValidatorAddress),
                    cosmos::Coin(
                        getString(amountObject, kAmount),
                        getString(amountObject, kDenom)
                    )
                };
            }

            cosmos::MsgUndelegate buildMsgUndelegateFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                auto amountObject = valueObject[kAmount].GetObject();

                return {
                    getString(valueObject, kDelegatorAddress),
                    getString(valueObject, kValidatorAddress),
                    cosmos::Coin(
                        getString(amountObject, kAmount),
                        getString(amountObject, kDenom)
                    )
                };
            }

             cosmos::MsgBeginRedelegate buildMsgBeginRedelegateFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                auto amountObject = valueObject[kAmount].GetObject();

                return {
                    getString(valueObject, kDelegatorAddress),
                    getString(valueObject, kValidatorSrcAddress),
                    getString(valueObject, kValidatorDstAddress),
                    cosmos::Coin(
                        getString(amountObject, kAmount),
                        getString(amountObject, kDenom)
                    )
                };
            }

             cosmos::MsgSubmitProposal buildMsgSubmitProposalFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                auto contentObject = valueObject[kContent].GetObject();
                auto initialDepositArray = valueObject[kInitialDeposit].GetArray();

                auto content = api::CosmosLikeContent(
                    getString(contentObject, kType),
                    getString(contentObject, kTitle),
                    getString(contentObject, kDescription)
                );

                std::vector<cosmos::Coin> amounts;
                // the size of the array of amounts should be frequently equal to one
                amounts.reserve(initialDepositArray.Size());
                for (auto& amount : initialDepositArray) {
                    auto amountObject = amount.GetObject();
                    amounts.push_back(cosmos::Coin(
                        getString(amountObject, kAmount),
                        getString(amountObject, kDenom)
                    ));
                }

                return {
                    content,
                    getString(valueObject, kProposer),
                    amounts
                };
            }

            cosmos::MsgVote buildMsgVoteFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                return {
                    getString(valueObject, kVoter),
                    getString(valueObject, kProposalId),
                    api::from_string<api::CosmosLikeVoteOption>(getString(valueObject, kOption))
                };
            }

            cosmos::MsgDeposit buildMsgDepositFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                std::vector<cosmos::Coin> amounts;

                if (valueObject[kAmount].IsArray()) {
                    // the size of the array of amount should be frequently equals to one
                    amounts.reserve(valueObject[kAmount].GetArray().Size());

                    for (auto& amount : valueObject[kAmount].GetArray()) {
                        if (amount.IsObject()) {
                            auto amountObject = amount.GetObject();

                            amounts.push_back(cosmos::Coin{
                                getString(amountObject, kAmount),
                                getString(amountObject, kDenom)
                            });
                        }
                    }
                }

                return {
                    getString(valueObject, kDepositor),
                    getString(valueObject, kProposalId),
                    amounts
               };
            }

            cosmos::MsgWithdrawDelegationReward buildMsgWithdrawDelegationRewardFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                return {
                    getString(valueObject, kDelegatorAddress),
                    getString(valueObject, kValidatorAddress)
                };
            }

            cosmos::MsgMultiSend buildMsgMultiSendFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                std::vector<cosmos::MultiSendInput> inputs;
                std::vector<cosmos::MultiSendOutput> outputs;

                if (valueObject.HasMember(kInputs) && valueObject[kInputs].IsArray()) {
                    auto inputJson = valueObject[kInputs].GetArray();
                    inputs.reserve(inputJson.Size());
                    for (auto& input : inputJson) {
                        std::vector<cosmos::Coin> inputAmounts;
                        if (input.IsObject()) {
                            auto singleInput = input.GetObject();
                            auto coins = singleInput[kCoins].GetArray();

                            inputAmounts.reserve(coins.Size());

                            for (auto& amount : coins) {
                                if (amount.IsObject()) {
                                    auto amountObject = amount.GetObject();

                                    inputAmounts.push_back(
                                        cosmos::Coin{getString(amountObject, kAmount),
                                                              getString(amountObject, kDenom)});
                                }
                            }

                            inputs.push_back({getString(singleInput, kAddress), inputAmounts});
                        }
                    }
                }

                if (valueObject.HasMember(kOutputs) && valueObject[kOutputs].IsArray()) {
                    auto outputJson = valueObject[kOutputs].GetArray();
                    outputs.reserve(outputJson.Size());
                    for (auto& output : outputJson) {
                        std::vector<cosmos::Coin> outputAmounts;
                        if (output.IsObject()) {
                            auto singleOutput = output.GetObject();
                            auto coins = singleOutput[kCoins].GetArray();

                            outputAmounts.reserve(coins.Size());

                            for (auto& amount : coins) {
                                if (amount.IsObject()) {
                                    auto amountObject = amount.GetObject();

                                    outputAmounts.push_back(
                                        cosmos::Coin{getString(amountObject, kAmount),
                                                              getString(amountObject, kDenom)});
                                }
                            }

                            outputs.push_back({getString(singleOutput, kAddress), outputAmounts});
                        }
                    }
                }

                return {inputs, outputs};
            }

            cosmos::MsgCreateValidator buildMsgCreateValidatorFromRawMessage(
                Object const& object) {
                auto valueObject = object[kValue].GetObject();
                cosmos::ValidatorDescription description;
                cosmos::ValidatorCommission commission;
                std::string minSelfDelegation("");
                std::string delegatorAddress("");
                std::string validatorAddress("");
                std::string pubkey("");
                cosmos::Coin value;
                if (valueObject.HasMember(kDescription) && valueObject[kDescription].IsObject()) {
                    auto descriptionObject = valueObject[kDescription].GetObject();
                    description.moniker = getString(descriptionObject, kMoniker);
                    if (descriptionObject.HasMember(kIdentity) &&
                        descriptionObject[kIdentity].IsString()) {
                        description.identity =
                            optional<std::string>(getString(descriptionObject, kIdentity));
                    }
                    if (descriptionObject.HasMember(kWebsite) &&
                        descriptionObject[kWebsite].IsString()) {
                        description.website =
                            optional<std::string>(getString(descriptionObject, kWebsite));
                    }
                    if (descriptionObject.HasMember(kDetails) &&
                        descriptionObject[kDetails].IsString()) {
                        description.details =
                            optional<std::string>(getString(descriptionObject, kDetails));
                    }
                }

                if (valueObject.HasMember(kCommission) && valueObject[kCommission].IsObject()) {
                    auto commissionObject = valueObject[kCommission].GetObject();
                    commission.rates.rate = getString(commissionObject, kCommissionRate);
                    commission.rates.maxRate = getString(commissionObject, kCommissionMaxRate);
                    commission.rates.maxChangeRate =
                        getString(commissionObject, kCommissionMaxChangeRate);
                    commission.updateTime =
                        DateUtils::fromJSON(getString(commissionObject, kUpdateTime));
                }

                if (valueObject.HasMember(kValue) && valueObject[kValue].IsObject()) {
                    value = cosmos::Coin{
                        getString(valueObject[kValue].GetObject(), kAmount),
                        getString(valueObject[kValue].GetObject(), kDenom),
                    };
                }

                minSelfDelegation = getString(valueObject, kMinSelfDelegation);
                delegatorAddress = getString(valueObject, kDelegatorAddress);
                validatorAddress = getString(valueObject, kValidatorAddress);
                pubkey = getString(valueObject, kPubKey);

                return {description,
                        commission,
                        minSelfDelegation,
                        delegatorAddress,
                        validatorAddress,
                        pubkey,
                        value};
            }

            cosmos::MsgEditValidator buildMsgEditValidatorFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                optional<cosmos::ValidatorDescription> description;
                std::string validatorAddress("");
                optional<std::string> commissionRate;
                optional<std::string> minSelfDelegation;
                if (valueObject.HasMember(kDescription) && valueObject[kDescription].IsObject()) {
                    auto descriptionObject = valueObject[kDescription].GetObject();
                    description = cosmos::ValidatorDescription();
                    description->moniker = getString(descriptionObject, kMoniker);
                    if (descriptionObject.HasMember(kIdentity) &&
                        descriptionObject[kIdentity].IsString()) {
                        description->identity =
                            optional<std::string>(getString(descriptionObject, kIdentity));
                    }
                    if (descriptionObject.HasMember(kWebsite) &&
                        descriptionObject[kWebsite].IsString()) {
                        description->website =
                            optional<std::string>(getString(descriptionObject, kWebsite));
                    }
                    if (descriptionObject.HasMember(kDetails) &&
                        descriptionObject[kDetails].IsString()) {
                        description->details =
                            optional<std::string>(getString(descriptionObject, kDetails));
                    }
                }

                validatorAddress = getString(valueObject, kValidatorAddress);
                if (valueObject.HasMember(kCommissionRate) &&
                    valueObject[kCommissionRate].IsString()) {
                    commissionRate = optional<std::string>(getString(valueObject, kCommissionRate));
                }
                if (valueObject.HasMember(kMinSelfDelegation) &&
                    valueObject[kMinSelfDelegation].IsString()) {
                    minSelfDelegation =
                        optional<std::string>(getString(valueObject, kMinSelfDelegation));
                }

                return {description, validatorAddress, commissionRate, minSelfDelegation};
            }

            cosmos::MsgSetWithdrawAddress buildMsgSetWithdrawAddressFromRawMessage(
                Object const& object) {
                auto valueObject = object[kValue].GetObject();
                return cosmos::MsgSetWithdrawAddress{getString(valueObject, kDelegatorAddress),
                                                     getString(valueObject, kWithdrawAddress)};
            }

            cosmos::MsgWithdrawDelegatorReward buildMsgWithdrawDelegatorRewardFromRawMessage(
                Object const& object) {
                auto valueObject = object[kValue].GetObject();
                return cosmos::MsgWithdrawDelegatorReward{
                    getString(valueObject, kDelegatorAddress),
                    getString(valueObject, kValidatorAddress)};
            }

            cosmos::MsgWithdrawValidatorCommission
            buildMsgWithdrawValidatorCommissionFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                return cosmos::MsgWithdrawValidatorCommission{
                    getString(valueObject, kValidatorAddress)};
            }

            cosmos::MsgUnjail buildMsgUnjailFromRawMessage(Object const& object) {
                auto valueObject = object[kValue].GetObject();
                return cosmos::MsgUnjail{getString(valueObject, kValidatorAddress)};
            }
        }

        CosmosLikeTransactionBuilder::CosmosLikeTransactionBuilder(
                const std::shared_ptr<api::ExecutionContext> &context,
                const api::Currency &currency,
                const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer,
                const std::shared_ptr<spdlog::logger> &logger,
                const CosmosLikeTransactionBuildFunction &buildFunction) {
            _context = context;
            _currency = currency;
            _explorer = explorer;
            _build = buildFunction;
            _logger = logger;
            _request.wipe = false;
        }

        CosmosLikeTransactionBuilder::CosmosLikeTransactionBuilder(const CosmosLikeTransactionBuilder &cpy) {
            _currency = cpy._currency;
            _build = cpy._build;
            _request = cpy._request;
            _context = cpy._context;
            _logger = cpy._logger;
        }

         std::shared_ptr<api::CosmosLikeTransactionBuilder>  CosmosLikeTransactionBuilder::setSequence(const std::string & sequence) {
            _request.sequence = sequence;
            return shared_from_this();
        }

        std::shared_ptr<api::CosmosLikeTransactionBuilder>  CosmosLikeTransactionBuilder::setMemo(const std::string & memo) {
            _request.memo = memo;
            return shared_from_this();
        }

        std::shared_ptr<api::CosmosLikeTransactionBuilder>  CosmosLikeTransactionBuilder::setGas(const std::shared_ptr<api::Amount> & gas) {
            _request.gas = std::make_shared<BigInt>(gas->toString());
            return shared_from_this();
        }

        std::shared_ptr<api::CosmosLikeTransactionBuilder>  CosmosLikeTransactionBuilder::setFee(const std::shared_ptr<api::Amount> & fee) {
            _request.fee = std::make_shared<BigInt>(fee->toString());
            return shared_from_this();
        }

        std::shared_ptr<api::CosmosLikeTransactionBuilder> CosmosLikeTransactionBuilder::addMessage(const std::shared_ptr<api::CosmosLikeMessage> & message) {
            _request.messages.push_back(message);
            return shared_from_this();
        }

        void CosmosLikeTransactionBuilder::build(const std::shared_ptr<api::CosmosLikeTransactionCallback> &callback) {
            build().callback(_context, callback);
        }

        Future<std::shared_ptr<api::CosmosLikeTransaction>> CosmosLikeTransactionBuilder::build() {
            return _build(_request, _explorer);
        }

        std::shared_ptr<api::CosmosLikeTransactionBuilder> CosmosLikeTransactionBuilder::clone() {
            return std::make_shared<CosmosLikeTransactionBuilder>(*this);
        }

        void CosmosLikeTransactionBuilder::reset() {
            _request = CosmosLikeTransactionBuildRequest();
        }

        std::shared_ptr<api::CosmosLikeTransaction>
        api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(const api::Currency &currency,
                                                                       const std::string &rawTransaction) {
            return ::ledger::core::CosmosLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, false);
        }

        std::shared_ptr<api::CosmosLikeTransaction>
        api::CosmosLikeTransactionBuilder::parseRawSignedTransaction(const api::Currency &currency,
                                                                     const std::string &rawTransaction) {
            return ::ledger::core::CosmosLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, true);
        }

        std::shared_ptr<api::CosmosLikeTransaction>
        CosmosLikeTransactionBuilder::parseRawTransaction(const api::Currency &currency,
                                                          const std::string &rawTransaction,
                                                          bool isSigned) {
            Document document;
            document.Parse(rawTransaction.c_str());

            auto tx = std::make_shared<CosmosLikeTransactionApi>();
            tx->setCurrency(currency);
            tx->setAccountNumber(getString(document.GetObject(), kAccountNumber));
            tx->setMemo(getString(document.GetObject(), kMemo));
            tx->setSequence(getString(document.GetObject(), kSequence));

            //Get fees
            if (document[kFee].IsObject()) {
                auto feeObject = document[kFee].GetObject();

                // Gas Limit
                auto gas = std::make_shared<BigInt>(getString(feeObject, kGas));
                tx->setGas(gas);

                // Total Tx fees
                // Gas Price is then deduced with Total_Tx_fees / Gas Limit
                // TODO figure out why the fee contains an array of amounts
                if (feeObject[kAmount].IsArray()) {
                    auto fee = BigInt();

                    auto getAmount = [=] (const Object &object) -> Amount {
                        auto denom = getString(object, kDenom);
                        auto amount = getString(object, kAmount);

                        auto unit = std::find_if(currency.units.begin(), currency.units.end(), [&] (const api::CurrencyUnit &unit) {
                            return unit.name == denom;
                        });

                        assert(unit->name == "uatom"); // FIXME Temporary until all units correctly supported

                        if (unit == currency.units.end()) {
                            throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Unknown unit while parsing transaction");
                        }
                        //TODO: Fix Amount::toUnit
                        return Amount(currency, 0, BigInt(amount) * BigInt(10).pow(static_cast<unsigned short>((*unit).numberOfDecimal)));
                    };

                    // accumlate all types of fee
                    for (auto& amount : feeObject[kAmount].GetArray()) {
                        if (amount.IsObject()) {
                            fee = fee + BigInt(getAmount(amount.GetObject()).toString());
                        }
                    }

                    tx->setFee(std::make_shared<BigInt>(fee));
                }
            }

            // Msgs object
            if (document[kMessages].IsArray()) {
                std::vector<std::shared_ptr<api::CosmosLikeMessage>> messages;

                messages.reserve(document[kMessages].GetArray().Size());

                for (auto& msg: document[kMessages].GetArray()) {
                    if (msg.IsObject()) {
                        auto msgObject = msg.GetObject();

                        switch (cosmos::stringToMsgType(getString(msgObject, kType))) {
                            case api::CosmosLikeMsgType::MSGSEND:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgSend(
                                    buildMsgSendFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGDELEGATE:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgDelegate(
                                    buildMsgDelegateFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGUNDELEGATE:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgUndelegate(
                                    buildMsgUndelegateFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGBEGINREDELEGATE:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgBeginRedelegate(
                                    buildMsgBeginRedelegateFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgSubmitProposal(
                                    buildMsgSubmitProposalFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGVOTE:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgVote(
                                    buildMsgVoteFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGDEPOSIT:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgDeposit(
                                    buildMsgDepositFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgWithdrawDelegationReward(
                                    buildMsgWithdrawDelegationRewardFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGMULTISEND:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgMultiSend(
                                    buildMsgMultiSendFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGCREATEVALIDATOR:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgCreateValidator(
                                    buildMsgCreateValidatorFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGEDITVALIDATOR:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgEditValidator(
                                    buildMsgEditValidatorFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgSetWithdrawAddress(
                                    buildMsgSetWithdrawAddressFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgWithdrawDelegatorReward(
                                    buildMsgWithdrawDelegatorRewardFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgWithdrawValidatorCommission(
                                    buildMsgWithdrawValidatorCommissionFromRawMessage(msgObject)));
                                break;
                            case api::CosmosLikeMsgType::MSGUNJAIL:
                                messages.push_back(api::CosmosLikeMessage::wrapMsgUnjail(
                                    buildMsgUnjailFromRawMessage(msgObject)));
                                break;
                            default:
                            {
                                cosmos::Message msg;
                                msg.type = getString(msgObject, kType);
                                msg.content = cosmos::MsgUnsupported();
                                messages.push_back(std::make_shared<::ledger::core::CosmosLikeMessage>(msg));
                            }
                            break;
                        }
                    }
                }

                tx->setMessages(messages);
            }

            return tx;
        }
    }
}
