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

#include <cosmos/api/CosmosLikeMsgType.hpp>
#include <cosmos/api/CosmosLikeMsgSend.hpp>
#include <cosmos/api/CosmosLikeMsgDelegate.hpp>
#include <cosmos/api/CosmosLikeMsgDeposit.hpp>
#include <cosmos/api/CosmosLikeMsgDeposit.hpp>
#include <cosmos/api/CosmosLikeMsgRedelegate.hpp>
#include <cosmos/api/CosmosLikeMsgSend.hpp>
#include <cosmos/api/CosmosLikeMsgSubmitProposal.hpp>
#include <cosmos/api/CosmosLikeMsgType.hpp>
#include <cosmos/api/CosmosLikeMsgUndelegate.hpp>
#include <cosmos/api/CosmosLikeMsgVote.hpp>
#include <cosmos/api/CosmosLikeMsgWithdrawDelegationReward.hpp>
#include <cosmos/api/CosmosLikeMsgMultiSend.hpp>
#include <cosmos/api/CosmosLikeMsgCreateValidator.hpp>
#include <cosmos/api/CosmosLikeMsgEditValidator.hpp>
#include <cosmos/api/CosmosLikeMsgSetWithdrawAddress.hpp>
#include <cosmos/api/CosmosLikeMsgWithdrawDelegatorReward.hpp>
#include <cosmos/api/CosmosLikeMsgWithdrawValidatorCommission.hpp>
#include <cosmos/api/CosmosLikeMsgUnjail.hpp>
#include <core/api/Block.hpp>

#include <core/math/BigInt.hpp>
#include <core/utils/Option.hpp>

#include <boost/variant.hpp>

#include <list>


namespace ledger {
        namespace core {
                namespace cosmos {

                        using Block = ledger::core::api::Block;

                        using ProposalContent = api::CosmosLikeContent;
                        using VoteOption = api::CosmosLikeVoteOption;

                        using ValidatorDescription = api::CosmosLikeValidatorDescription;
                        using ValidatorCommission = api::CosmosLikeValidatorCommission;

                        using CommissionRates = api::CosmosLikeCommissionRates;

                        using MultiSendInput = api::CosmosLikeMultiSendInput;
                        using MultiSendOutput = api::CosmosLikeMultiSendOutput;

                        using Coin = api::CosmosLikeAmount;
                        using MsgSend = api::CosmosLikeMsgSend;
                        using MsgDelegate = api::CosmosLikeMsgDelegate;
                        using MsgUndelegate = api::CosmosLikeMsgUndelegate;
                        using MsgRedelegate = api::CosmosLikeMsgRedelegate;
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
                            MsgRedelegate,
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

			// Small helpers to avoid very long types
			using TransactionList = std::list<std::shared_ptr<Transaction>>;
			using MsgType = ::ledger::core::api::CosmosLikeMsgType;
		}
	}
}
