/*
 *
 * CosmosLikeConstants
 *
 * Created by Alexis Le Provost on 06/11/2019.
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
#include <wallet/cosmos/cosmos.hpp>

namespace ledger {
        namespace core {
                namespace cosmos {
                        namespace constants {
                                // use raw char array here to be compliant with rapidjson
                                constexpr const char kMsgDelegate[] = "cosmos-sdk/MsgDelegate";
                                constexpr const char kMsgDeposit[] = "cosmos-sdk/MsgDeposit";
                                constexpr const char kMsgRedelegate[] = "cosmos-sdk/MsgBeginRedelegate";
                                constexpr const char kMsgSend[] = "cosmos-sdk/MsgSend";
                                constexpr const char kMsgSubmitProposal[] = "cosmos-sdk/MsgSubmitProposal";
                                constexpr const char kMsgUndelegate[] = "cosmos-sdk/MsgUndelegate";
                                constexpr const char kMsgVote[] = "cosmos-sdk/MsgVote";
                                constexpr const char kMsgWithdrawDelegationReward[] = "cosmos-sdk/MsgWithdrawDelegationReward";
                                constexpr const char kMsgMultiSend[] = "cosmos-sdk/MsgMultiSend";
                                constexpr const char kMsgCreateValidator[] = "cosmos-sdk/MsgCreateValidator";
                                constexpr const char kMsgEditValidator[] = "cosmos-sdk/MsgEditValidator";
                                constexpr const char kMsgSetWithdrawAddress[] = "cosmos-sdk/MsgSetWithdrawAddress";
                                constexpr const char kMsgWithdrawDelegatorReward[] = "cosmos-sdk/MsgWithdrawDelegatorReward";
                                constexpr const char kMsgWithdrawValidatorCommission[] = "cosmos-sdk/MsgWithdrawValidatorCommission";
                                constexpr const char kMsgUnjail[] = "cosmos-sdk/MsgUnjail";

                                constexpr const char kAccountNumber[] = "account_number";
                                constexpr const char kAddress[] = "address";
                                constexpr const char kAmount[] = "amount";
                                constexpr const char kChainId[] = "chain_id";
                                constexpr const char kCoins[] = "coins";
                                constexpr const char kCommission[] = "commission";
                                constexpr const char kCommissionRate[] = "commission_rate";
                                constexpr const char kCommissionRates[] = "commission_rates";
                                constexpr const char kCommissionMaxRate[] = "commission_max_rate";
                                constexpr const char kCommissionMaxChangeRate[] = "commission_max_change_rate";
                                constexpr const char kContent[] = "content";
                                constexpr const char kDelegatorAddress[] = "delegator_address";
                                constexpr const char kDenom[] = "denom";
                                constexpr const char kDepositor[] = "depositor";
                                constexpr const char kDescription[] = "description";
                                constexpr const char kDetails[] = "details";
                                constexpr const char kFee[] = "fee";
                                constexpr const char kFromAddress[] = "from_address";
                                constexpr const char kGas[] = "gas";
                                constexpr const char kGasUsed[] = "gas_used";
                                constexpr const char kHeight[] = "height";
                                constexpr const char kIdentity[] = "identity";
                                constexpr const char kInitialDeposit[] = "initial_deposit";
                                constexpr const char kInputs[] = "inputs";
                                constexpr const char kLog[] = "log";
                                constexpr const char kLogs[] = "logs";
                                constexpr const char kMaxHeight[] = "maxheight";
                                constexpr const char kMemo[] = "memo";
                                constexpr const char kMessage[] = "msg";
                                constexpr const char kMessages[] = "msgs";
                                constexpr const char kMinHeight[] = "minheight";
                                constexpr const char kMinSelfDelegation[] = "min_self_delegation";
                                constexpr const char kMoniker[] = "moniker";
                                constexpr const char kMsgIndex[] = "msg_index";
                                constexpr const char kOption[] = "option";
                                constexpr const char kOutputs[] = "outputs";
                                constexpr const char kProposalId[] = "proposal_id";
                                constexpr const char kProposer[] = "proposer";
                                constexpr const char kPubKey[] = "pub_key";
                                constexpr const char kSequence[] = "sequence";
                                constexpr const char kSignature[] = "signature";
                                constexpr const char kSuccess[] = "success";
                                constexpr const char kTimestamp[] = "timestamp";
                                constexpr const char kTitle[] = "title";
                                constexpr const char kToAddress[] = "to_address";
                                constexpr const char kTxArray[] = "txs";
                                constexpr const char kTxHash[] = "txhash";
                                constexpr const char kTx[] = "tx";
                                constexpr const char kType[] = "type";
                                constexpr const char kUpdateTime[] = "update_time";
                                constexpr const char kValidatorAddress[] = "validator_address";
                                constexpr const char kValidatorDstAddress[] = "validator_dst_address";
                                constexpr const char kValidatorSrcAddress[] = "validator_src_address";
                                constexpr const char kValue[] = "value";
                                constexpr const char kVoter[] = "voter";
                                constexpr const char kWebsite[] = "website";
                                constexpr const char kWithdrawAddress[] = "withdraw_address";

                                // Explorer specific constants
                                constexpr const char kBlockMeta[] = "block_meta";
                                constexpr const char kBlockId[] = "block_id";
                                constexpr const char kHash[] = "hash";
                                constexpr const char kHeader[] = "header";
                                constexpr const char kTime[] = "time";

                                // cosmos/cosmos-sdk Event / Attribute types as of
                                // https://github.com/cosmos/cosmos-sdk/tree/43137ee893cefbdb2aacd25ef4ec39eacf6ae70c

                                // Common in all messages
                                constexpr const char kEventTypeMessage[] = "message";

                                constexpr const char kAttributeKeyAction[] = "action";
                                constexpr const char kAttributeKeyModule[] = "module";
                                constexpr const char kAttributeKeySender[] = "sender";
                                constexpr const char kAttributeKeyAmount[] = "amount";

                                // Staking
                                constexpr const char kEventTypeCompleteUnbonding[] =
                                    "complete_unbonding";
                                constexpr const char kEventTypeCompleteRedelegation[] =
                                    "complete_redelegation";
                                constexpr const char kEventTypeCreateValidator[] =
                                    "create_validator";
                                constexpr const char kEventTypeEditValidator[] = "edit_validator";
                                constexpr const char kEventTypeDelegate[] = "delegate";
                                constexpr const char kEventTypeUnbond[] = "unbond";
                                constexpr const char kEventTypeRedelegate[] = "redelegate";

                                constexpr const char kAttributeKeyValidator[] = "validator";
                                constexpr const char kAttributeKeyCommissionRate[] =
                                    "commission_rate";
                                constexpr const char kAttributeKeyMinSelfDelegation[] =
                                    "min_self_delegation";
                                constexpr const char kAttributeKeySrcValidator[] =
                                    "source_validator";
                                constexpr const char kAttributeKeyDstValidator[] =
                                    "destination_validator";
                                constexpr const char kAttributeKeyDelegator[] = "delegator";
                                constexpr const char kAttributeKeyCompletionTime[] =
                                    "completion_time";
                                constexpr const char kAttributeValueStakingCategory[] = "staking";

                                // Distribution
                                constexpr const char kEventTypeSetWithdrawAddress[] =
                                    "set_withdraw_address";
                                constexpr const char kEventTypeRewards[] = "rewards";
                                constexpr const char kEventTypeCommission[] = "commission";
                                constexpr const char kEventTypeWithdrawRewards[] =
                                    "withdraw_rewards";
                                constexpr const char kEventTypeWithdrawCommission[] =
                                    "withdraw_commission";
                                constexpr const char kEventTypeProposerReward[] = "proposer_reward";

                                constexpr const char kAttributeKeyWithdrawAddress[] =
                                    "withdraw_address";

                                constexpr const char kAttributeValueDistributionCategory[] =
                                    "distribution";

                                // gov
                                constexpr const char kEventTypeSubmitProposal[] = "submit_proposal";
                                constexpr const char kEventTypeProposalDeposit[] =
                                    "proposal_deposit";
                                constexpr const char kEventTypeProposalVote[] = "proposal_vote";
                                constexpr const char kEventTypeInactiveProposal[] =
                                    "inactive_proposal";
                                constexpr const char kEventTypeActiveProposal[] = "active_proposal";

                                constexpr const char kAttributeKeyProposalResult[] =
                                    "proposal_result";
                                constexpr const char kAttributeKeyOption[] = "option";
                                constexpr const char kAttributeKeyProposalID[] = "proposal_id";
                                constexpr const char kAttributeKeyVotingPeriodStart[] =
                                    "voting_period_start";
                                constexpr const char kAttributeValueGovernanceCategory[] =
                                    "governance";
                                constexpr const char kAttributeValueProposalDropped[] =
                                    "proposal_dropped";  // didn't meet min deposit
                                constexpr const char kAttributeValueProposalPassed[] =
                                    "proposal_passed";  // met vote quorum
                                constexpr const char kAttributeValueProposalRejected[] =
                                    "proposal_rejected";  // didn't meet vote quorum
                                constexpr const char kAttributeValueProposalFailed[] =
                                    "proposal_failed";  // error on proposal handler
                                constexpr const char kAttributeKeyProposalType[] = "proposal_type";

                                // bank
                                constexpr const char kEventTypeTransfer[] = "transfer";

                                constexpr const char kAttributeKeyRecipient[] = "recipient";

                                constexpr const char kAttributeValueBankCategory[] = "bank";

                                // crisis
                                constexpr const char kEventTypeInvariant[] = "invariant";

                                constexpr const char kAttributeValueCrisis[] = "crisis";
                                constexpr const char kAttributeKeyRoute[] = "route";

                                // slashing
                                constexpr const char kEventTypeSlash[] = "slash";
                                constexpr const char kEventTypeLiveness[] = "liveness";

                                constexpr const char kAttributeKeyAddress[] = "address";
                                constexpr const char kAttributeKeyHeight[] = "height";
                                constexpr const char kAttributeKeyPower[] = "power";
                                constexpr const char kAttributeKeyReason[] = "reason";
                                constexpr const char kAttributeKeyJailed[] = "jailed";
                                constexpr const char kAttributeKeyMissedBlocks[] = "missed_blocks";

                                constexpr const char kAttributeValueDoubleSign[] = "double_sign";
                                constexpr const char kAttributeValueMissingSignature[] =
                                    "missing_signature";
                                constexpr const char kAttributeValueSlashingCategory[] = "slashing";

                                // mint
                                constexpr const char kEventTypeMint[] = "mint";

                                constexpr const char kAttributeKeyBondedRatio[] = "bonded_ratio";
                                constexpr const char kAttributeKeyInflation[] = "inflation";
                                constexpr const char kAttributeKeyAnnualProvisions[] =
                                    "annual_provisions";
                                }  // namespace constants

                        static constexpr const char* msgTypeToChars(MsgType type) {
                                switch (type) {
                                        case MsgType::MSGSEND:
                                                return constants::kMsgSend;
                                        case MsgType::MSGDELEGATE:
                                                return constants::kMsgDelegate;
                                        case MsgType::MSGUNDELEGATE:
                                                return constants::kMsgUndelegate;
                                        case MsgType::MSGREDELEGATE:
                                                return constants::kMsgRedelegate;
                                        case MsgType::MSGSUBMITPROPOSAL:
                                                return constants::kMsgSubmitProposal;
                                        case MsgType::MSGVOTE:
                                                return constants::kMsgVote;
                                        case MsgType::MSGDEPOSIT:
                                                return constants::kMsgDeposit;
                                        case MsgType::MSGWITHDRAWDELEGATIONREWARD:
                                                return constants::kMsgWithdrawDelegationReward;
                                        case MsgType::MSGMULTISEND:
                                            return constants::kMsgMultiSend;
                                        case MsgType::MSGCREATEVALIDATOR:
                                            return constants::kMsgCreateValidator;
                                        case MsgType::MSGEDITVALIDATOR:
                                            return constants::kMsgEditValidator;
                                        case MsgType::MSGSETWITHDRAWADDRESS:
                                            return constants::kMsgSetWithdrawAddress;
                                        case MsgType::MSGWITHDRAWDELEGATORREWARD:
                                            return constants::kMsgWithdrawDelegatorReward;
                                        case MsgType::MSGWITHDRAWVALIDATORCOMMISSION:
                                            return constants::kMsgWithdrawValidatorCommission;
                                        case MsgType::MSGUNJAIL:
                                            return constants::kMsgUnjail;
                                        case MsgType::UNSUPPORTED:
                                        default:
                                                return "";
                                }
                        }

                        static inline constexpr bool strings_equal(char const * a, char const * b) {
                                return *a == *b && (*a == '\0' || strings_equal(a + 1, b + 1));
                        }

                        static constexpr MsgType stringToMsgType(const char* string) {
                                if (strings_equal(string, constants::kMsgSend)) {
                                        return MsgType::MSGSEND;
                                } else if (strings_equal(string, constants::kMsgDelegate)) {
                                        return MsgType::MSGDELEGATE;
                                } else if (strings_equal(string, constants::kMsgUndelegate)) {
                                        return MsgType::MSGUNDELEGATE;
                                } else if (strings_equal(string, constants::kMsgRedelegate)) {
                                        return MsgType::MSGREDELEGATE;
                                } else if (strings_equal(string, constants::kMsgSubmitProposal)) {
                                        return MsgType::MSGSUBMITPROPOSAL;
                                } else if (strings_equal(string, constants::kMsgVote)) {
                                        return MsgType::MSGVOTE;
                                } else if (strings_equal(string, constants::kMsgDeposit)) {
                                        return MsgType::MSGDEPOSIT;
                                } else if (strings_equal(string, constants::kMsgWithdrawDelegationReward)) {
                                        return MsgType::MSGWITHDRAWDELEGATIONREWARD;
                                } else if (strings_equal(string, constants::kMsgMultiSend)) {
                                        return MsgType::MSGMULTISEND;
                                } else if (strings_equal(string, constants::kMsgCreateValidator)) {
                                        return MsgType::MSGCREATEVALIDATOR;
                                } else if (strings_equal(string, constants::kMsgEditValidator)) {
                                        return MsgType::MSGEDITVALIDATOR;
                                } else if (strings_equal(string, constants::kMsgSetWithdrawAddress)) {
                                        return MsgType::MSGSETWITHDRAWADDRESS;
                                } else if (strings_equal(string, constants::kMsgWithdrawDelegatorReward)) {
                                        return MsgType::MSGWITHDRAWDELEGATORREWARD;
                                } else if (strings_equal(string, constants::kMsgWithdrawValidatorCommission)) {
                                        return MsgType::MSGWITHDRAWVALIDATORCOMMISSION;
                                } else if (strings_equal(string, constants::kMsgUnjail)) {
                                        return MsgType::MSGUNJAIL;
                                } else {
                                        return MsgType::UNSUPPORTED;
                                }
                        }
                }
        }
}
