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
#include <api/CosmosLikeVoteOption.hpp>
#include <wallet/cosmos/cosmos.hpp>

namespace ledger {
namespace core {
namespace cosmos {
namespace constants {
// Explorer endpoints.
constexpr char kGaiaAccountEndpoint[] = "/auth/accounts/{}";
constexpr char kGaiaBalancesEndpoint[] = "/bank/balances/{}";
constexpr char kGaiaBlocksEndpoint[] = "/blocks/{}";
constexpr char kGaiaDelegationsEndpoint[] = "/staking/delegators/{}/delegations";
constexpr char kGaiaDistInfoEndpoint[] = "/distribution/validators/{}";
constexpr char kGaiaLatestBlockEndpoint[] = "/blocks/latest";
constexpr char kGaiaQueryRedelegationsEndpoint[] = "/staking/redelegations";
constexpr char kGaiaRedelegationsEndpoint[] = "/staking/delegators/{}/redelegations";
constexpr char kGaiaRewardsEndpoint[] = "/distribution/delegators/{}/rewards";
constexpr char kGaiaSignInfoEndpoint[] = "/slashing/validators/{}/signing_info";
constexpr char kGaiaTransactionEndpoint[] = "/txs/{}";
constexpr char kGaiaTransactionsWithPageLimitEnpoint[] = "/txs?{}&page={}&limit={}";
constexpr char kGaiaTransfersEndpoint[] = "/bank/accounts/{}/transfers";
constexpr char kGaiaUnbondingsEndpoint[] = "/staking/delegators/{}/unbonding_delegations";
constexpr char kGaiaValidatorInfoEndpoint[] = "/staking/validators/{}";
constexpr char kGaiaWithdrawAddressEndpoint[] = "/distribution/delegators/{}/withdraw_address";

// gRPC gateway explorer constants
// They take at least
// - one {} to hold the currency ("cosmos" almost all the time)
// - one {} to hold the version name
// - one or more {} for the query parameters
constexpr char kGrpcAccountEndpoint[] = "/{}/auth/{}/accounts/{}";
constexpr char kGrpcAllBalancesEndpoint[] = "/{}/bank/{}/balances/{}";
constexpr char kGrpcBalancesEndpoint[] = "/{}/bank/{}/balances/{}/{}";
constexpr char kGrpcBlocksEndpoint[] = "/blocks/{}";
constexpr char kGrpcDelegationsEndpoint[] = "/{}/staking/{}/delegations/{}";
// constexpr char kGrpcDistInfoEndpoint[] = "/distribution/validators/{}";
constexpr char kGrpcLatestBlockEndpoint[] = "/blocks/latest";
// constexpr char kGrpcQueryRedelegationsEndpoint[] = "/staking/redelegations";
constexpr char kGrpcRedelegationsEndpoint[] = "/{}/staking/{}/delegators/{}/redelegations";
constexpr char kGrpcRewardsEndpoint[] = "/{}/distribution/{}/delegators/{}/rewards";
constexpr char kGrpcSignInfoEndpoint[] = "/{}/slashing/{}/signing_infos/{}";
constexpr char kGrpcSimulationEndpoint[] = "/{}/tx/{}/simulate";
constexpr char kGrpcTransactionEndpoint[] = "/txs/{}";
constexpr char kGrpcTransactionsWithPageLimitEnpoint[] = "/txs?{}&page={}&limit={}";
// constexpr char kGrpcTransfersEndpoint[] = "/bank/accounts/{}/transfers";
constexpr char kGrpcUnbondingsEndpoint[] = "/{}/staking/{}/delegators/{}/unbonding_delegations";
constexpr char kGrpcValidatorInfoEndpoint[] = "/{}/staking/{}/validators/{}";
constexpr char kGrpcWithdrawAddressEndpoint[] = "/{}/distribution/{}/delegators/{}/withdraw_address";

// use raw char array here to be compliant with rapidjson
constexpr char kMsgBeginRedelegate[] = "cosmos-sdk/MsgBeginRedelegate";
constexpr char kMsgDelegate[] = "cosmos-sdk/MsgDelegate";
constexpr char kMsgDeposit[] = "cosmos-sdk/MsgDeposit";
constexpr char kMsgSend[] = "cosmos-sdk/MsgSend";
constexpr char kMsgSubmitProposal[] = "cosmos-sdk/MsgSubmitProposal";
constexpr char kMsgUndelegate[] = "cosmos-sdk/MsgUndelegate";
constexpr char kMsgVote[] = "cosmos-sdk/MsgVote";
constexpr char kMsgWithdrawDelegationReward[] = "cosmos-sdk/MsgWithdrawDelegationReward";
constexpr char kMsgMultiSend[] = "cosmos-sdk/MsgMultiSend";
constexpr char kMsgCreateValidator[] = "cosmos-sdk/MsgCreateValidator";
constexpr char kMsgEditValidator[] = "cosmos-sdk/MsgEditValidator";
constexpr char kMsgSetWithdrawAddress[] = "cosmos-sdk/MsgSetWithdrawAddress";
constexpr char kMsgWithdrawDelegatorReward[] = "cosmos-sdk/MsgWithdrawDelegatorReward";
constexpr char kMsgWithdrawValidatorCommission[] = "cosmos-sdk/MsgWithdrawValidatorCommission";
constexpr char kMsgUnjail[] = "cosmos-sdk/MsgUnjail";
constexpr char kMsgFees[] = "internal/MsgFees";

constexpr char kAccount[] = "account";
constexpr char kAccountNumber[] = "account_number";
constexpr char kAddress[] = "address";
constexpr char kAmount[] = "amount";
constexpr char kBalance[] = "balance";
constexpr char kBaseReq[] = "base_req";
constexpr char kBlock[] = "block";
constexpr char kChainId[] = "chain_id";
constexpr char kCoins[] = "coins";
constexpr char kCommission[] = "commission";
constexpr char kCommissionRate[] = "rate";
constexpr char kCommissionRates[] = "commission_rates";
constexpr char kCommissionMaxRate[] = "max_rate";
constexpr char kCommissionMaxChangeRate[] = "max_change_rate";
constexpr char kCompletionTime[] = "completion_time";
constexpr char kConsensusPubkey[] = "consensus_pubkey";
constexpr char kContent[] = "content";
constexpr char kCount[] = "count";
constexpr char kCreationHeight[] = "creation_height";
constexpr char kDelegation[] = "delegation";
constexpr char kDelegatorAddress[] = "delegator_address";
constexpr char kDelegationResponses[] = "delegation_responses";
constexpr char kDenom[] = "denom";
constexpr char kDepositor[] = "depositor";
constexpr char kDescription[] = "description";
constexpr char kDetails[] = "details";
constexpr char kEditValCommissionRate[] = "commission_rate";
constexpr char kEntries[] = "entries";
constexpr char kFee[] = "fee";
constexpr char kFees[] = "fees";
constexpr char kFrom[] = "from";
constexpr char kFromAddress[] = "from_address";
constexpr char kGas[] = "gas";
constexpr char kGasAdjustment[] = "gas_adjustment";
constexpr char kGasEstimate[] = "gas_estimate";
constexpr char kGasUsed[] = "gas_used";
constexpr char kGrpcType[] = "@type";
constexpr char kHeight[] = "height";
constexpr char kIdentity[] = "identity";
constexpr char kIndexOffset[] = "index_offset";
constexpr char kInitialBalance[] = "initial_balance";
constexpr char kInitialDeposit[] = "initial_deposit";
constexpr char kInputs[] = "inputs";
constexpr char kJailed[] = "jailed";
constexpr char kJailedUntil[] = "jailed_until";
constexpr char kKey[] = "key";
constexpr char kLog[] = "log";
constexpr char kLogs[] = "logs";
constexpr char kMaxHeight[] = "maxheight";
constexpr char kMemo[] = "memo";
constexpr char kMessage[] = "msg";
constexpr char kMessages[] = "msgs";
constexpr char kMinHeight[] = "minheight";
constexpr char kMinSelfDelegation[] = "min_self_delegation";
constexpr char kMissedBlocksCounter[] = "missed_blocks_counter";
constexpr char kMoniker[] = "moniker";
constexpr char kMsgIndex[] = "msg_index";
constexpr char kOperatorAddress[] = "operator_address";
constexpr char kOption[] = "option";
constexpr char kOutputs[] = "outputs";
constexpr char kProposalId[] = "proposal_id";
constexpr char kProposer[] = "proposer";
constexpr char kPubKey[] = "pub_key";
constexpr char kRawLog[] = "raw_log";
constexpr char kRedelegation[] = "redelegation";
constexpr char kRedelegationEntry[] = "redelegation_entry";
constexpr char kRedelegationResponses[] = "redelegation_responses";
constexpr char kResult[] = "result";
constexpr char kReward[] = "reward";
constexpr char kRewards[] = "rewards";
constexpr char kSelfBondRewards[] = "self_bond_rewards";
constexpr char kSecurityContact[] = "security_contact";
constexpr char kSequence[] = "sequence";
constexpr char kSignature[] = "signature";
constexpr char kSimulate[] = "simulate";
constexpr char kSignatures[] = "signatures";
constexpr char kStartHeight[] = "start_height";
constexpr char kStatus[] = "status";
constexpr char kSuccess[] = "success";
constexpr char kTimestamp[] = "timestamp";
constexpr char kTitle[] = "title";
constexpr char kToAddress[] = "to_address";
constexpr char kTokens[] = "tokens";
constexpr char kTombstoned[] = "tombstoned";
constexpr char kTotal[] = "total";
constexpr char kTotalCount[] = "total_count";
constexpr char kTxArray[] = "txs";
constexpr char kTxHash[] = "txhash";
constexpr char kTx[] = "tx";
constexpr char kType[] = "type";
constexpr char kUnbondingHeight[] = "unbonding_height";
constexpr char kUnbondingEntry[] = "unbonding_entry";
constexpr char kUnbondingResponses[] = "unbonding_responses";
constexpr char kUnbondingTime[] = "unbonding_time";
constexpr char kUpdateTime[] = "update_time";
constexpr char kValCommission[] = "val_commission";
constexpr char kValSignInfos[] = "val_signing_info";
constexpr char kValidator[] = "validator";
constexpr char kValidatorAddress[] = "validator_address";
constexpr char kValidatorDstAddress[] = "validator_dst_address";
constexpr char kValidatorSrcAddress[] = "validator_src_address";
constexpr char kValidators[] = "validators";
constexpr char kValue[] = "value";
constexpr char kVoter[] = "voter";
constexpr char kWebsite[] = "website";
constexpr char kWithdrawAddress[] = "withdraw_address";

// Explorer specific constants
constexpr char kBlockMeta[] = "block_meta";
constexpr char kBlockId[] = "block_id";
constexpr char kHash[] = "hash";
constexpr char kHeader[] = "header";
constexpr char kTime[] = "time";
constexpr char kMode[] = "mode";
constexpr char kGrpcErrorMessage[] = "message";

// cosmos/cosmos-sdk Event / Attribute types as of
// https://github.com/cosmos/cosmos-sdk/tree/43137ee893cefbdb2aacd25ef4ec39eacf6ae70c

// Common in all messages
constexpr char kEventTypeMessage[] = "message";

constexpr char kAttributeKeyAction[] = "action";
constexpr char kAttributeKeyModule[] = "module";
constexpr char kAttributeKeySender[] = "sender";
constexpr char kAttributeKeyAmount[] = "amount";
constexpr char kAttributeWithdrawDelegationReward[] = "withdraw_delegator_reward";

constexpr char kPubKeySecp256k1[] = "tendermint/PubKeySecp256k1";

// Staking
constexpr char kEventTypeCompleteUnbonding[] = "complete_unbonding";
constexpr char kEventTypeCompleteRedelegation[] = "complete_redelegation";
constexpr char kEventTypeCreateValidator[] = "create_validator";
constexpr char kEventTypeEditValidator[] = "edit_validator";
constexpr char kEventTypeDelegate[] = "delegate";
constexpr char kEventTypeUnbond[] = "unbond";
constexpr char kEventTypeRedelegate[] = "redelegate";

constexpr char kAttributeKeyValidator[] = "validator";
constexpr char kAttributeKeyCommissionRate[] = "commission_rate";
constexpr char kAttributeKeyMinSelfDelegation[] = "min_self_delegation";
constexpr char kAttributeKeySrcValidator[] = "source_validator";
constexpr char kAttributeKeyDstValidator[] = "destination_validator";
constexpr char kAttributeKeyDelegator[] = "delegator";
constexpr char kAttributeKeyCompletionTime[] = "completion_time";
constexpr char kAttributeValueStakingCategory[] = "staking";

// Distribution
constexpr char kEventTypeSetWithdrawAddress[] = "set_withdraw_address";
constexpr char kEventTypeRewards[] = "rewards";
constexpr char kEventTypeCommission[] = "commission";
constexpr char kEventTypeWithdrawRewards[] = "withdraw_rewards";
constexpr char kEventTypeWithdrawCommission[] = "withdraw_commission";
constexpr char kEventTypeProposerReward[] = "proposer_reward";

constexpr char kAttributeKeyWithdrawAddress[] = "withdraw_address";

constexpr char kAttributeValueDistributionCategory[] = "distribution";

// gov
constexpr char kEventTypeSubmitProposal[] = "submit_proposal";
constexpr char kEventTypeProposalDeposit[] = "proposal_deposit";
constexpr char kEventTypeProposalVote[] = "proposal_vote";
constexpr char kEventTypeInactiveProposal[] = "inactive_proposal";
constexpr char kEventTypeActiveProposal[] = "active_proposal";

constexpr char kAttributeKeyProposalResult[] = "proposal_result";
constexpr char kAttributeKeyOption[] = "option";
constexpr char kAttributeKeyProposalID[] = "proposal_id";
constexpr char kAttributeKeyVotingPeriodStart[] = "voting_period_start";
constexpr char kAttributeValueGovernanceCategory[] = "governance";
constexpr char kAttributeValueProposalDropped[] = "proposal_dropped";  // didn't meet min deposit
constexpr char kAttributeValueProposalPassed[] = "proposal_passed";  // met vote quorum
constexpr char kAttributeValueProposalRejected[] = "proposal_rejected";  // didn't meet vote quorum
constexpr char kAttributeValueProposalFailed[] = "proposal_failed";  // error on proposal handler
constexpr char kAttributeKeyProposalType[] = "proposal_type";

constexpr char kVoteOptionAbstain[] = "Abstain";
constexpr char kVoteOptionNo[] = "No";
constexpr char kVoteOptionNoWithVeto[] = "NoWithVeto";
constexpr char kVoteOptionYes[] = "Yes";

// bank
constexpr char kEventTypeTransfer[] = "transfer";

constexpr char kAttributeKeyRecipient[] = "recipient";

constexpr char kAttributeValueBankCategory[] = "bank";

// crisis
constexpr char kEventTypeInvariant[] = "invariant";

constexpr char kAttributeValueCrisis[] = "crisis";
constexpr char kAttributeKeyRoute[] = "route";

// slashing
constexpr char kEventTypeSlash[] = "slash";
constexpr char kEventTypeLiveness[] = "liveness";

constexpr char kAttributeKeyAddress[] = "address";
constexpr char kAttributeKeyHeight[] = "height";
constexpr char kAttributeKeyPower[] = "power";
constexpr char kAttributeKeyReason[] = "reason";
constexpr char kAttributeKeyJailed[] = "jailed";
constexpr char kAttributeKeyMissedBlocks[] = "missed_blocks";

constexpr char kAttributeValueDoubleSign[] = "double_sign";
constexpr char kAttributeValueMissingSignature[] = "missing_signature";
constexpr char kAttributeValueSlashingCategory[] = "slashing";

// mint
constexpr char kEventTypeMint[] = "mint";

constexpr char kAttributeKeyBondedRatio[] = "bonded_ratio";
constexpr char kAttributeKeyInflation[] = "inflation";
constexpr char kAttributeKeyAnnualProvisions[] = "annual_provisions";
}  // namespace constants

static constexpr const char *msgTypeToChars(MsgType type)
{
    switch (type) {
    case MsgType::MSGSEND:
        return constants::kMsgSend;
    case MsgType::MSGDELEGATE:
        return constants::kMsgDelegate;
    case MsgType::MSGUNDELEGATE:
        return constants::kMsgUndelegate;
    case MsgType::MSGBEGINREDELEGATE:
        return constants::kMsgBeginRedelegate;
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

static inline constexpr bool strings_equal(char const *a, char const *b)
{
    return *a == *b && (*a == '\0' || strings_equal(a + 1, b + 1));
}

static constexpr MsgType stringToMsgType(const char *string)
{
    if (strings_equal(string, constants::kMsgSend)) {
        return MsgType::MSGSEND;
    }
    if (strings_equal(string, constants::kMsgDelegate)) {
        return MsgType::MSGDELEGATE;
    }
    if (strings_equal(string, constants::kMsgUndelegate)) {
        return MsgType::MSGUNDELEGATE;
    }
    if (strings_equal(string, constants::kMsgBeginRedelegate)) {
        return MsgType::MSGBEGINREDELEGATE;
    }
    if (strings_equal(string, constants::kMsgSubmitProposal)) {
        return MsgType::MSGSUBMITPROPOSAL;
    }
    if (strings_equal(string, constants::kMsgVote)) {
        return MsgType::MSGVOTE;
    }
    if (strings_equal(string, constants::kMsgDeposit)) {
        return MsgType::MSGDEPOSIT;
    }
    if (strings_equal(string, constants::kMsgWithdrawDelegationReward)) {
        return MsgType::MSGWITHDRAWDELEGATIONREWARD;
    }
    if (strings_equal(string, constants::kMsgMultiSend)) {
        return MsgType::MSGMULTISEND;
    }
    if (strings_equal(string, constants::kMsgCreateValidator)) {
        return MsgType::MSGCREATEVALIDATOR;
    }
    if (strings_equal(string, constants::kMsgEditValidator)) {
        return MsgType::MSGEDITVALIDATOR;
    }
    if (strings_equal(string, constants::kMsgSetWithdrawAddress)) {
        return MsgType::MSGSETWITHDRAWADDRESS;
    }
    if (strings_equal(string, constants::kMsgWithdrawDelegatorReward)) {
        return MsgType::MSGWITHDRAWDELEGATORREWARD;
    }
    if (strings_equal(string, constants::kMsgWithdrawValidatorCommission)) {
        return MsgType::MSGWITHDRAWVALIDATORCOMMISSION;
    }
    if (strings_equal(string, constants::kMsgUnjail)) {
        return MsgType::MSGUNJAIL;
    }
    if (strings_equal(string, constants::kMsgFees)) {
        return MsgType::MSGFEES;
    }
    return MsgType::UNSUPPORTED;
}

static constexpr const char *voteOptionToChars(api::CosmosLikeVoteOption option)
{
    switch (option) {
    case api::CosmosLikeVoteOption::COSMOSVOTEABSTAIN:
        return constants::kVoteOptionAbstain;
    case api::CosmosLikeVoteOption::COSMOSVOTENO:
        return constants::kVoteOptionNo;
    case api::CosmosLikeVoteOption::COSMOSVOTENOWITHVETO:
        return constants::kVoteOptionNoWithVeto;
    case api::CosmosLikeVoteOption::COSMOSVOTEYES:
        return constants::kVoteOptionYes;
    default:
        return "unknown";
    }
}

static constexpr api::CosmosLikeVoteOption stringToVoteOption(const char *string)
{
    if (strings_equal(string, constants::kVoteOptionAbstain)) {
        return api::CosmosLikeVoteOption::COSMOSVOTEABSTAIN;
    }
    else if (strings_equal(string, constants::kVoteOptionNo)) {
        return api::CosmosLikeVoteOption::COSMOSVOTENO;
    }
    else if (strings_equal(string, constants::kVoteOptionNoWithVeto)) {
        return api::CosmosLikeVoteOption::COSMOSVOTENOWITHVETO;
    }
    else {
        return api::CosmosLikeVoteOption::COSMOSVOTEYES;
    }
}
}  // namespace cosmos
}  // namespace core
}  // namespace ledger
