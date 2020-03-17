/*
 *
 * ledger-core
 *
 * Created by Pierre Pollastri.
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

#pragma once

#include <api/CosmosLikeMsgType.hpp>
#include <api/CosmosLikeMsgSend.hpp>
#include <api/CosmosLikeMsgDelegate.hpp>
#include <api/CosmosLikeMsgDeposit.hpp>
#include <api/CosmosLikeMsgDeposit.hpp>
#include <api/CosmosLikeMsgBeginRedelegate.hpp>
#include <api/CosmosLikeMsgSend.hpp>
#include <api/CosmosLikeMsgSubmitProposal.hpp>
#include <api/CosmosLikeMsgType.hpp>
#include <api/CosmosLikeMsgUndelegate.hpp>
#include <api/CosmosLikeMsgVote.hpp>
#include <api/CosmosLikeMsgWithdrawDelegationReward.hpp>
#include <api/CosmosLikeMsgMultiSend.hpp>
#include <api/CosmosLikeMsgCreateValidator.hpp>
#include <api/CosmosLikeMsgEditValidator.hpp>
#include <api/CosmosLikeMsgSetWithdrawAddress.hpp>
#include <api/CosmosLikeMsgWithdrawDelegatorReward.hpp>
#include <api/CosmosLikeMsgWithdrawValidatorCommission.hpp>
#include <api/CosmosLikeMsgUnjail.hpp>

#include <api/CosmosLikeValidator.hpp>

#include <wallet/common/Block.h>

#include <math/BigInt.h>
#include <utils/Option.hpp>

#include <boost/variant.hpp>

#include <list>


namespace ledger {
        namespace core {
                namespace cosmos {

                        using Block = ledger::core::Block;

                        using ProposalContent = api::CosmosLikeContent;
                        using VoteOption = api::CosmosLikeVoteOption;

                        using ValidatorDescription = api::CosmosLikeValidatorDescription;
                        using ValidatorCommission = api::CosmosLikeValidatorCommission;

                        using Validator = api::CosmosLikeValidator;
                        using CommissionRates = api::CosmosLikeCommissionRates;

                        using MultiSendInput = api::CosmosLikeMultiSendInput;
                        using MultiSendOutput = api::CosmosLikeMultiSendOutput;

                        using Coin = api::CosmosLikeAmount;
                        using MsgSend = api::CosmosLikeMsgSend;
                        using MsgDelegate = api::CosmosLikeMsgDelegate;
                        using MsgUndelegate = api::CosmosLikeMsgUndelegate;
                        using MsgBeginRedelegate = api::CosmosLikeMsgBeginRedelegate;
                        using MsgSubmitProposal = api::CosmosLikeMsgSubmitProposal;
                        using MsgVote = api::CosmosLikeMsgVote;
                        using MsgDeposit = api::CosmosLikeMsgDeposit;
                        using MsgWithdrawDelegationReward = api::CosmosLikeMsgWithdrawDelegationReward;
                        using MsgMultiSend = api::CosmosLikeMsgMultiSend;
                        using MsgCreateValidator = api::CosmosLikeMsgCreateValidator;
                        using MsgEditValidator = api::CosmosLikeMsgEditValidator;
                        using MsgSetWithdrawAddress = api::CosmosLikeMsgSetWithdrawAddress;
                        using MsgWithdrawDelegatorReward =
                            api::CosmosLikeMsgWithdrawDelegatorReward;
                        using MsgWithdrawValidatorCommission =
                            api::CosmosLikeMsgWithdrawValidatorCommission;
                        using MsgUnjail = api::CosmosLikeMsgUnjail;

                        struct MsgUnsupported {};

                        using MessageContent = boost::variant<
                            MsgSend,
                            MsgDelegate,
                            MsgBeginRedelegate,
                            MsgUndelegate,
                            MsgSubmitProposal,
                            MsgVote,
                            MsgDeposit,
                            MsgWithdrawDelegationReward,
                            MsgMultiSend,
                            MsgCreateValidator,
                            MsgEditValidator,
                            MsgSetWithdrawAddress,
                            MsgWithdrawDelegatorReward,
                            MsgWithdrawValidatorCommission,
                            MsgUnjail,
                            MsgUnsupported>;

                        struct Message {
                                std::string uid;
                                std::string type;
                                MessageContent content;
                        };

                        /**
                           Status of completion of a given message (success/failure and reason in case of failure),
                           MessageLog.messageIndex is the index of the message in the message list held by the transaction object.
                        */
                        struct MessageLog {
                                int32_t messageIndex;
                                bool success;
                                std::string log;
                        };

                        /**
                           Represents the fee object which is a combination of an amount of fee and the amount of gas that this fee holds.
                        */
                        struct Fee {
                                BigInt gas;
                                std::vector<Coin> amount;
                        };

                        struct Transaction {
                                std::string uid;
                                std::string hash;
                                Option<Block> block;
                                Fee fee;
                                Option<BigInt> gasUsed;
                                std::chrono::system_clock::time_point timestamp;
                                std::vector<Message> messages;
                                std::string memo;
                                std::vector<MessageLog> logs;
                        };

                        struct Account {
                                std::string type;
                                std::string address;
                                std::vector<Coin> balances;
                                std::string accountNumber;
                                std::string sequence;
                                std::chrono::system_clock::time_point lastUpdate;
                        };

                        struct Delegation {
                                std::string delegatorAddress;
                                std::string validatorAddress;
                                BigInt delegatedAmount;
                        };

                        // Small helpers to avoid very long types
                        using TransactionList = std::list<std::shared_ptr<Transaction>>;
                        using ValidatorList = std::vector<Validator>;
                        using MsgType = ::ledger::core::api::CosmosLikeMsgType;

                        struct TransactionsBulk {
                                std::vector<cosmos::Transaction> transactions;
                                bool hasNext;
                        };
                }
        }
}
