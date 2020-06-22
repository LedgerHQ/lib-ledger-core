/*
 *
 * cosmos::Account
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <core/wallet/AbstractAccount.hpp>
#include <core/wallet/AbstractWallet.hpp>
#include <cosmos/api/Address.hpp>
#include <cosmos/api/Amount.hpp>
#include <cosmos/api/AmountCallback.hpp>
#include <cosmos/api/CosmosGasLimitRequest.hpp>
#include <cosmos/api/CosmosLikeAccount.hpp>
#include <cosmos/api/CosmosLikeDelegationListCallback.hpp>
#include <cosmos/api/CosmosLikeRedelegationListCallback.hpp>
#include <cosmos/api/CosmosLikeRewardListCallback.hpp>
#include <cosmos/api/CosmosLikeTransactionBuilder.hpp>
#include <cosmos/api/CosmosLikeUnbondingListCallback.hpp>
#include <cosmos/api/CosmosLikeValidatorCallback.hpp>
#include <cosmos/api/CosmosLikeValidatorListCallback.hpp>
#include <cosmos/api/ErrorCodeCallback.hpp>
#include <cosmos/api/Event.hpp>
#include <cosmos/api/StringCallback.hpp>
#include <cosmos/api_impl/Delegation.hpp>
#include <cosmos/api_impl/Operation.hpp>
#include <cosmos/cosmos.hpp>
#include <time.h>

#include <cosmos/api_impl/Redelegation.hpp>
#include <cosmos/api_impl/Reward.hpp>
#include <cosmos/api_impl/Unbonding.hpp>
#include <cosmos/explorers/BlockchainExplorer.hpp>
#include <cosmos/keychains/Keychain.hpp>
#include <cosmos/observers/BlockchainObserver.hpp>
#include <cosmos/synchronizers/AccountSynchronizer.hpp>

namespace ledger {
namespace core {
namespace cosmos {

class CosmosLikeAccount : public api::CosmosLikeAccount,
                          public core::AbstractAccount {
public:
  static const int FLAG_TRANSACTION_IGNORED = 0x00;

  CosmosLikeAccount(
      const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
      const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer,
      const std::shared_ptr<CosmosLikeBlockchainObserver> &observer,
      const std::shared_ptr<CosmosLikeAccountSynchronizer> &synchronizer,
      const std::shared_ptr<CosmosLikeKeychain> &keychain);

  /// Fill an Operation from cosmos specific data in the given transaction and
  /// message. \param [out] out The Operation to fill \param [in] wallet A
  /// pointer to the wallet inflating the operation (for currency handling)
  /// \param [in] tx The transaction data to fill Operation metadata with
  /// \param [in] msg The cosmos message data to fill Operation metadata with
  void inflateOperation(CosmosLikeOperation &out,
                        const std::shared_ptr<const AbstractWallet> &wallet,
                        const Transaction &tx, const Message &msg);

  /// Insert a transaction and all its messages in the database.
  /// \param [in] sql The sql session accessing the database
  /// \param [in] transaction The transaction to maybe insert in the database
  /// \return The index of enum for one of the operation types in the
  /// transaction. Not useful most of the time.
  int putTransaction(soci::session &sql, const Transaction &transaction);

  /// Insert a block metadata in the database
  /// \param [in] sql The sql session accessing the database
  /// \param [in] block The block to maybe insert in the database
  /// \return true if the block was actually inserted/updated. false otherwise
  /// (see BlockDatabaseHelper::putBlock)
  bool putBlock(soci::session &sql, const api::Block &block);

  std::shared_ptr<CosmosLikeKeychain> getKeychain() const;
  std::string getAddress() const;

  FuturePtr<core::Amount> getBalance() override;

  Future<AbstractAccount::AddressList> getFreshPublicAddresses() override;

  Future<std::vector<std::shared_ptr<api::Amount>>>
  getBalanceHistory(const std::string &start, const std::string &end,
                    api::TimePeriod precision) override;

  Future<api::ErrorCode>
  eraseDataSince(const std::chrono::system_clock::time_point &date) override;

  bool isSynchronizing() override;

  std::shared_ptr<api::EventBus> synchronize() override;

  void startBlockchainObservation() override;

  void stopBlockchainObservation() override;

  bool isObservingBlockchain() override;

  std::string getRestoreKey() override;

  void broadcastRawTransaction(
      const std::string &transaction,
      const std::shared_ptr<api::StringCallback> &callback) override;

  void broadcastTransaction(
      const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
      const std::shared_ptr<api::StringCallback> &callback) override;

  std::shared_ptr<api::CosmosLikeTransactionBuilder>
  buildTransaction() override;
  std::shared_ptr<api::CosmosLikeTransactionBuilder>
  buildTransaction(const std::string &senderAddress);

  std::shared_ptr<api::OperationQuery> queryOperations() override;

  void getEstimatedGasLimit(
      const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
      const std::shared_ptr<api::BigIntCallback> &callback) override;

  void
  estimateGas(const api::CosmosGasLimitRequest &request,
              const std::shared_ptr<api::BigIntCallback> &callback) override;

  // Account related data
  void
  getSequence(const std::shared_ptr<api::StringCallback> &callback) override;
  void getAccountNumber(
      const std::shared_ptr<api::StringCallback> &callback) override;
  void getWithdrawAddress(
      const std::shared_ptr<api::StringCallback> &callback) override;
  cosmos::Account getInfo() const;

  // Balances
  /// Ask the explorer for the total balance of the account.
  /// \return A Future with the total balance
  FuturePtr<core::Amount> getTotalBalance() const;
  void getTotalBalance(
      const std::shared_ptr<api::AmountCallback> &callback) override;

  /// Ask the explorer for the total balance *without the pending rewards* of
  /// the account. \return A Future with the total balance *without the pending
  /// rewards*
  FuturePtr<core::Amount> getTotalBalanceWithoutPendingRewards() const;

  /// Ask the explorer for the balance currently delegated by the account.
  /// \return A Future with the delegated balance
  FuturePtr<core::Amount> getDelegatedBalance() const;
  void getDelegatedBalance(
      const std::shared_ptr<api::AmountCallback> &callback) override;

  /// Ask the explorer for the total pending delegation rewards for the account
  /// \return A Future with the total pending rewards
  FuturePtr<core::Amount> getPendingRewardsBalance() const;
  void getPendingRewardsBalance(
      const std::shared_ptr<api::AmountCallback> &callback) override;

  /// Ask the explorer for the total amount in unbonding phase for the account.
  /// \return A Future with the total amount
  FuturePtr<core::Amount> getUnbondingBalance() const;
  void getUnbondingBalance(
      const std::shared_ptr<api::AmountCallback> &callback) override;

  /// Ask the explorer for the spendable balance of the account.
  /// \return A Future with the spendable balance
  FuturePtr<core::Amount> getSpendableBalance() const;
  void getSpendableBalance(
      const std::shared_ptr<api::AmountCallback> &callback) override;

  /// Ask the explorer for the currently active validator set on the chain.
  /// \return A Future with the list of validators
  Future<ValidatorList> getActiveValidatorSet() const;
  void getLatestValidatorSet(
      const std::shared_ptr<api::CosmosLikeValidatorListCallback> &callback)
      override;
  /// Ask the explorer for specific informations about a validator. SigningInfo
  /// and DistributionInfo are included there. \param [in] validatorAddress The
  /// 'cosmosvaloper' address of the queried validator. \return A Future with
  /// the complete information about a validator
  Future<Validator> getValidatorInfo(const std::string &validatorAddress) const;
  void getValidatorInfo(const std::string &validatorAddress,
                        const std::shared_ptr<api::CosmosLikeValidatorCallback>
                            &callback) override;

  void getDelegations(
      const std::shared_ptr<api::CosmosLikeDelegationListCallback> &callback)
      override;
  /// Ask the explorer for the current delegations of the account.
  /// \return A Future with the list of Delegations
  Future<std::vector<std::shared_ptr<api::CosmosLikeDelegation>>>
  getDelegations();

  void getPendingRewards(
      const std::shared_ptr<api::CosmosLikeRewardListCallback> &callback)
      override;
  /// Ask the explorer for the pending rewards specifics of the account.
  /// \return A Future with the list of pending rewards
  Future<std::vector<std::shared_ptr<api::CosmosLikeReward>>>
  getPendingRewards();

  // "Pending" status info
  /// Ask the explorer for the current unbondings of the account.
  /// \return A Future with the list of unbondings
  Future<std::vector<std::shared_ptr<api::CosmosLikeUnbonding>>>
  getUnbondings() const;
  void getUnbondings(const std::shared_ptr<api::CosmosLikeUnbondingListCallback>
                         &callback) override;
  /// Ask the explorer for the current redelegations of the account.
  /// \return A Future with the list of redelegations
  Future<std::vector<std::shared_ptr<api::CosmosLikeRedelegation>>>
  getRedelegations() const;
  void getRedelegations(
      const std::shared_ptr<api::CosmosLikeRedelegationListCallback> &callback)
      override;

private:
  std::shared_ptr<CosmosLikeAccount> getSelf();
  void updateFromDb();

  // These helpers stay on CosmosLikeAccount *only* because they have to use
  // their knowledge of Address information in order to correctly map operation
  // type. An operation type is always seen from the account point of view.

  /// Set the type and the amount of an Operation from a cosmos Message in the
  /// context of the account. \param [out] out the Operation to fill \param [in]
  /// msg the cosmos Message to use as source of information
  void setOperationTypeAndAmount(CosmosLikeOperation &out,
                                 const Message &msg) const;
  /// Set the type and the amount of an Operation from an unwrapped cosmos Send
  /// Message. \param [out] out the Operation to fill \param [in] innerSendMsg
  /// the cosmos send message to use, unwrapped.
  void fillOperationTypeAmountFromSend(CosmosLikeOperation &out,
                                       const MsgSend &innerSendMsg) const;
  /// Set the type and the amount of an Operation from an unwrapped cosmos
  /// MultiSend Message. \param [out] out the Operation to fill \param [in]
  /// innerMultiSendMsg the cosmos multiSend message to use, unwrapped.
  void fillOperationTypeAmountFromMultiSend(
      CosmosLikeOperation &out, const MsgMultiSend &innerMultiSendMsg) const;
  /// Set the type and the amount of an Operation from an unwrapped cosmos
  /// Delegate Message. \param [out] out the Operation to fill \param [in]
  /// innerDelegateMsg the cosmos delegate message to use, unwrapped.
  void fillOperationTypeAmountFromDelegate(
      CosmosLikeOperation &out, const MsgDelegate &innerDelegateMsg) const;
  /// Set the type and the amount of an Operation from an unwrapped cosmos
  /// Undelegate Message. \param [out] out the Operation to fill \param [in]
  /// innerUndelegateMsg the cosmos undelegate message to use, unwrapped.
  void fillOperationTypeAmountFromUndelegate(
      CosmosLikeOperation &out, const MsgUndelegate &innerUndelegateMsg) const;
  /// Set the type and the amount of an Operation from an unwrapped cosmos
  /// BeginRedelegate Message. \param [out] out the Operation to fill \param
  /// [in] innerBeginRedelegateMsg the cosmos beginRedelegate message to use,
  /// unwrapped.
  void fillOperationTypeAmountFromBeginRedelegate(
      CosmosLikeOperation &out,
      const MsgBeginRedelegate &innerBeginRedelegateMsg) const;
  /// Set the type and the amount of an Operation from an unwrapped cosmos
  /// SubmitProposal Message. \param [out] out the Operation to fill \param [in]
  /// innerSubmitProposalMsg the cosmos submitProposal message to use,
  /// unwrapped.
  void fillOperationTypeAmountFromSubmitProposal(
      CosmosLikeOperation &out,
      const MsgSubmitProposal &innerSubmitProposalMsg) const;
  /// Set the type and the amount of an Operation from an unwrapped cosmos
  /// Deposit Message. \param [out] out the Operation to fill \param [in]
  /// innerDepositMsg the cosmos deposit message to use, unwrapped.
  void
  fillOperationTypeAmountFromDeposit(CosmosLikeOperation &out,
                                     const MsgDeposit &innerDepositMsg) const;
  /// Set the type and the amount of an Operation from an unwrapped cosmos Fees
  /// Message. \param [out] out the Operation to fill \param [in] innerFeesMsg
  /// the cosmos fees message to use, unwrapped.
  void fillOperationTypeAmountFromFees(CosmosLikeOperation &out,
                                       const MsgFees &innerFeesMsg) const;

  /// Compute the exact fees paid for the Transaction, applying the consumed gas
  /// ratio if available. \param [in] tx A Cosmos Transaction to compute the
  /// fees from \return the amount of fees paid for this transaction in uatom
  static uint32_t computeFeesForTransaction(const Transaction &tx);

  std::shared_ptr<cosmos::Account> _accountData;
  std::shared_ptr<CosmosLikeKeychain> _keychain;
  std::shared_ptr<CosmosLikeBlockchainExplorer> _explorer;
  std::shared_ptr<CosmosLikeAccountSynchronizer> _synchronizer;
  std::shared_ptr<CosmosLikeBlockchainObserver> _observer;
  uint64_t _currentBlockHeight;
  std::shared_ptr<api::EventBus> _currentSyncEventBus;
  std::mutex _synchronizationLock;
};
} // namespace cosmos
} // namespace core
} // namespace ledger
