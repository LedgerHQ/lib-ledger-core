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

#include <rapidjson/document.h>

#include <utils/Option.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/common/Block.h>

#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeMessage.hpp>
#include <wallet/cosmos/cosmos.hpp>

#define COSMOS_PARSE_MSG_CONTENT(MsgType) if (out.type == "cosmos-sdk/"#MsgType) return parse##MsgType(contentNode, out.content);


namespace ledger {
    namespace core {
        using namespace cosmos::constants;
        namespace rpcs_parsers {

            template <class T>
            std::string getStringFromVariableKeys(const T& node, const std::list<std::string>& keys) {
                for (const auto& key : keys) {
                    if (node.HasMember(key.c_str())) {
                        return node[key.c_str()].GetString();
                    }
                }
                return "";
            }

            template <class T>
            void parseCoin(const T& node, cosmos::Coin& out) {
                assert((node.HasMember(kAmount)));
                assert((node.HasMember(kDenom)));
                out.amount = node[kAmount].GetString();
                out.denom = node[kDenom].GetString();
            }

            template <typename T>
            void parseCoinVector(const T& array, std::vector<cosmos::Coin>& out) {
                out.assign((std::size_t) array.Size(), cosmos::Coin());
                auto index = 0;
                for (const auto& n : array) {
                    parseCoin(n.GetObject(), out[index]);
                    index += 1;
                }
            }

            template <typename T>
            void parseMultiSendInputs(const T& array, std::vector<cosmos::MultiSendInput>& out) {
                auto index = 0;
                out.assign((std::size_t) array.Size(), cosmos::MultiSendInput());
                for (const auto& n : array) {
                    auto input = n.GetObject();
                    out[index].fromAddress = input[kAddress].GetString();
                    parseCoinVector(input[kCoins].GetArray(), out[index].coins);
                    index += 1;
                }
            }

            template <typename T>
            void parseMultiSendOutputs(const T& array, std::vector<cosmos::MultiSendOutput>& out) {
                auto index = 0;
                out.assign((std::size_t) array.Size(), cosmos::MultiSendOutput());
                for (const auto& n : array) {
                    auto output = n.GetObject();
                    out[index].toAddress = output[kAddress].GetString();
                    parseCoinVector(output[kCoins].GetArray(), out[index].coins);
                    index += 1;
                }
            }


            template <typename T>
            void parseDescription(const T& descriptionNode, cosmos::ValidatorDescription& out) {
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
            void parseCommission(const T& commissionNode, cosmos::ValidatorCommission& out) {
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

            template <class T>
            void parseBlock(const T& node, const std::string& currencyName, cosmos::Block& out) {
                out.currencyName = currencyName;
                out.hash = node[kBlockMeta].GetObject()[kBlockId].GetObject()[kHash].GetString();
                out.height = BigInt::fromString(node[kBlockMeta].GetObject()[kHeader].GetObject()[kHeight].GetString()).toUint64();
                out.time = DateUtils::fromJSON(node[kBlockMeta].GetObject()[kHeader].GetObject()[kTime].GetString());
            }

            template <typename T>
            void parseProposalContent(const T& node, cosmos::ProposalContent& out) {
                assert((node.HasMember(kType)));
                assert((node.HasMember(kDescription)));
                assert((node.HasMember(kTitle)));
                out.type = node[kType].GetString();
                out.description = node[kDescription].GetString();
                out.title = node[kTitle].GetString();
            }

            template <typename T>
            void parseFee(const T& node, cosmos::Fee& out) {
                assert((node.HasMember(kGas)));
                assert((node.HasMember(kAmount)));
                out.gas = BigInt::fromString(node[kGas].GetString());
                if (node[kAmount].IsArray()) {
                    const auto& amountArray = node[kAmount].GetArray();
                    out.amount.assign((std::size_t) amountArray.Capacity(), cosmos::Coin());
                    auto index = 0;
                    for (const auto& aNode : amountArray) {
                        parseCoin(aNode.GetObject(), out.amount[index]);
                        index += 1;
                    }
                }
            }

            template <class T>
            void parseAccount(const T& accountNode,
                    cosmos::Account& account) {
                assert((accountNode.HasMember(kValue)));
                assert((accountNode.HasMember(kType)));
                const auto& node = accountNode[kValue].GetObject();
                account.type = accountNode[kType].GetString();

                assert((node.HasMember(kAccountNumber)));
                assert((node.HasMember(kSequence)));
                assert((node.HasMember(kAddress)));
                assert((node.HasMember(kCoins)));
                account.accountNumber = node[kAccountNumber].GetString();
                account.sequence = node[kSequence].GetString();
                account.address = node[kAddress].GetString();
                const auto& balances = node[kCoins].GetArray();
                parseCoinVector(balances, account.balances);
            }

            template <typename T>
            void parseMsgSend(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgSend msg;

                assert((n.HasMember(kFromAddress)));
                assert((n.HasMember(kToAddress)));
                msg.fromAddress = n[kFromAddress].GetString();
                msg.toAddress = n[kToAddress].GetString();
                parseCoinVector(n[kAmount].GetArray(), msg.amount);
                out = msg;
            }

            template <typename T>
            void parseMsgDelegate(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgDelegate msg;

                assert((n.HasMember(kDelegatorAddress)));
                assert((n.HasMember(kValidatorAddress)));
                msg.delegatorAddress = n[kDelegatorAddress].GetString();
                msg.validatorAddress = n[kValidatorAddress].GetString();
                parseCoin(n[kAmount].GetObject(), msg.amount);
                out = msg;
            }

            template <typename T>
            void parseMsgBeginRedelegate(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgBeginRedelegate msg;

                msg.delegatorAddress = n[kDelegatorAddress].GetString();
                msg.validatorSourceAddress = n[kValidatorSrcAddress].GetString();
                msg.validatorDestinationAddress = n[kValidatorDstAddress].GetString();
                parseCoin(n[kAmount].GetObject(), msg.amount);
                out = msg;
            }

            template <typename T>
            void parseMsgUndelegate(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgUndelegate msg;

                msg.delegatorAddress = n[kDelegatorAddress].GetString();
                msg.validatorAddress = n[kValidatorAddress].GetString();
                parseCoin(n[kAmount].GetObject(), msg.amount);
                out = msg;
            }

            template <typename T>
            void parseMsgSubmitProposal(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgSubmitProposal msg;

                msg.proposer = n[kProposer].GetString();
                parseProposalContent(n[kContent].GetObject(), msg.content);
                parseCoinVector(n[kInitialDeposit].GetArray(), msg.initialDeposit);
                out = msg;
            }

            template <typename T>
            void parseMsgVote(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgVote msg;

                msg.voter = n[kVoter].GetString();
                msg.proposalId = n[kProposalId].GetString();
                const auto& option = n[kOption].GetString();
                if (option == "Yes") {
                    msg.option = cosmos::VoteOption::YES;
                } else if (option == "No") {
                    msg.option = cosmos::VoteOption::NO;
                } else if (option == "NoWithVeto") {
                    msg.option = cosmos::VoteOption::NOWITHVETO;
                } else if (option == "Abstain") {
                    msg.option = cosmos::VoteOption::ABSTAIN;
                }
                out = msg;
            }

            template <typename T>
            void parseMsgDeposit(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgDeposit msg;

                msg.depositor = n[kDepositor].GetString();
                msg.proposalId = n[kProposalId].GetString();
                parseCoinVector(n[kAmount].GetArray(), msg.amount);
                out = msg;
            }

            template <typename T>
            void parseMsgWithdrawDelegationReward(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgWithdrawDelegationReward msg;

                msg.delegatorAddress = n[kDelegatorAddress].GetString();
                msg.validatorAddress = n[kValidatorAddress].GetString();
                out = msg;
            }

            template <typename T>
            void parseMsgMultiSend(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgMultiSend msg;

                assert((n.HasMember(kInputs)));
                assert((n.HasMember(kOutputs)));

                // inputs: list<CosmosLikeMultiSendInput>;
                auto inputsArray = n[kInputs].GetArray();
                parseMultiSendInputs(inputsArray, msg.inputs);

                // outputs: list<CosmosLikeMultiSendOutput>;
                auto outputsArray = n[kInputs].GetArray();
                parseMultiSendOutputs(outputsArray, msg.outputs);

                out = msg;
            }

            template <typename T>
            void parseMsgCreateValidator(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgCreateValidator msg;
                assert((n.HasMember(kDescription)));
                assert((n.HasMember(kCommission)));
                assert((n.HasMember(kMinSelfDelegation)));
                assert((n.HasMember(kDelegatorAddress)));
                assert((n.HasMember(kValidatorAddress)));
                assert((n.HasMember(kPubKey)));
                assert((n.HasMember(kValue)));
                parseDescription(n[kDescription].GetObject(), msg.description);
                parseCommission(n[kCommission].GetObject(), msg.commission);
                msg.delegatorAddress = n[kDelegatorAddress].GetString();
                msg.validatorAddress = n[kValidatorAddress].GetString();
                msg.pubkey = n[kPubKey].GetString();
                msg.minSelfDelegation = n[kMinSelfDelegation].GetString();
                parseCoin(n[kValue].GetObject(), msg.value);

                out = msg;
            }

            template <typename T>
            void parseMsgEditValidator(const T& n, cosmos::MessageContent &out) {
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
                    msg.description = optional<cosmos::ValidatorDescription>(desc);
                }
                out = msg;
            }

            template <typename T>
            void parseMsgSetWithdrawAddress(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgSetWithdrawAddress msg;

                msg.delegatorAddress = n[kDelegatorAddress].GetString();
                msg.withdrawAddress = n[kWithdrawAddress].GetString();
                out = msg;
            }

            template <typename T>
            void parseMsgWithdrawDelegatorReward(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgWithdrawDelegatorReward msg;

                msg.delegatorAddress = n[kDelegatorAddress].GetString();
                msg.validatorAddress = n[kValidatorAddress].GetString();
                out = msg;
            }

            template <typename T>
            void parseMsgWithdrawValidatorCommission(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgWithdrawValidatorCommission msg;

                msg.validatorAddress = n[kValidatorAddress].GetString();
                out = msg;
            }

            template <typename T>
            void parseMsgUnjail(const T& n, cosmos::MessageContent &out) {
                cosmos::MsgUnjail msg;

                msg.validatorAddress = n[kValidatorAddress].GetString();
                out = msg;
            }

            template <typename T>
            void parseMessage(const T& messageNode, cosmos::Message& out) {
                out.type = messageNode[kType].GetString();
                const auto& contentNode = messageNode[kValue].GetObject();
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

            template <class T>
            void parseTransaction(const T& node,
                    cosmos::Transaction& transaction) {
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
                for (const auto& lNode : node[kLogs].GetArray()) {
                    cosmos::MessageLog log;
                    assert((lNode.HasMember(kLog)));
                    assert((lNode.HasMember(kSuccess)));
                    assert((lNode.HasMember(kMsgIndex)));
                    log.success = lNode[kSuccess].GetBool();
                    log.log = lNode[kLog].GetString();
                    log.messageIndex = BigInt::fromString(lNode[kMsgIndex].GetString()).toInt();
                    transaction.logs.emplace_back(log);
                }
                assert((node.HasMember(kTimestamp)));
                transaction.timestamp = DateUtils::fromJSON(node[kTimestamp].GetString());

                assert((node.HasMember(kTx)));
                const auto& tNode = node[kTx].GetObject();
                assert((tNode.HasMember(kValue)));
                const auto& vNode = tNode[kValue].GetObject();

                if (vNode.HasMember(kMemo)) {
                    transaction.memo = vNode[kMemo].GetString();
                }

                assert((vNode.HasMember(kMessage)));
                if (vNode[kMessage].IsArray()) {
                    const auto& msgArray = vNode[kMessage].GetArray();
                    transaction.messages.assign((std::size_t) msgArray.Capacity(), cosmos::Message());
                    auto index = 0;
                    for (const auto &mNode : msgArray) {
                        parseMessage(mNode, transaction.messages[index]);
                        index++;
                    }
                }
                assert((vNode.HasMember(kFee)));
                parseFee(vNode[kFee].GetObject(), transaction.fee);
            }

            template <typename T>
            void parseValidatorSetEntry(const T& n, cosmos::Validator &out) {
                assert((n.HasMember(kDescription)));
                parseDescription(n[kDescription].GetObject(), out.description);

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
                out.status = BigInt::fromString(n["status"].GetString()).toInt();

                out.slashTimestamps = optional<std::vector<std::chrono::system_clock::time_point>>();
            }

        }
    }
}

#undef COSMOS_PARSE_MSG_CONTENT
#endif //LEDGER_CORE_RPCS_PARSERS_HPP
