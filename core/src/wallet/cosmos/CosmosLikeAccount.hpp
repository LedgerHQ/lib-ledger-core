/*
 *
 * CosmosLikeAccount
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


#ifndef LEDGER_CORE_COSMOSLIKEACCOUNT_H
#define LEDGER_CORE_COSMOSLIKEACCOUNT_H

#include <api/CosmosLikeAccount.hpp>
#include <api/CosmosLikeDelegationListCallback.hpp>
#include <api/CosmosLikeRewardListCallback.hpp>
#include <api/CosmosLikeTransactionBuilder.hpp>
#include <api/CosmosLikeRedelegationListCallback.hpp>
#include <api/CosmosLikeUnbondingListCallback.hpp>
#include <api/Address.hpp>
#include <api/Amount.hpp>
#include <api/ErrorCodeCallback.hpp>
#include <api/Event.hpp>
#include <api/StringCallback.hpp>
#include <api/AmountCallback.hpp>
#include <api/CosmosGasLimitRequest.hpp>

#include <wallet/cosmos/api_impl/CosmosLikeUnbonding.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeRedelegation.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeDelegation.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeOperation.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeReward.hpp>
#include <wallet/cosmos/explorers/CosmosLikeBlockchainExplorer.hpp>
#include <wallet/cosmos/observers/CosmosLikeBlockchainObserver.hpp>
#include <wallet/cosmos/keychains/CosmosLikeKeychain.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <wallet/common/AbstractAccount.hpp>

#include <time.h>


#include <api/CosmosLikeValidatorListCallback.hpp>
#include <api/CosmosLikeValidatorCallback.hpp>

namespace ledger {
        namespace core {
                class CosmosLikeAccountSynchronizer;

                class CosmosLikeAccount : public api::CosmosLikeAccount, public AbstractAccount {
                        public:
                                static const int FLAG_TRANSACTION_IGNORED = 0x00;

                                CosmosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                                  int32_t index,
                                                  const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer,
                                                  const std::shared_ptr<CosmosLikeBlockchainObserver> &observer,
                                                  const std::shared_ptr<CosmosLikeAccountSynchronizer> &synchronizer,
                                                  const std::shared_ptr<CosmosLikeKeychain> &keychain);

                                std::shared_ptr<api::CosmosLikeAccount> asCosmosLikeAccount() override;

                                /// Fill an Operation from cosmos specific data in the given transaction and message.
                                /// \param [out] out The Operation to fill
                                /// \param [in] wallet A pointer to the wallet inflating the operation (for currency handling)
                                /// \param [in] tx The transaction data to fill Operation metadata with
                                /// \param [in] msg The cosmos message data to fill Operation metadata with
                                void inflateOperation(CosmosLikeOperation &out,
                                                      const std::shared_ptr<const AbstractWallet> &wallet,
                                                      const cosmos::Transaction &tx,
                                                      const cosmos::Message &msg);

                                /// Insert a transaction and all its messages in the database.
                                /// \param [in] sql The sql session accessing the database
                                /// \param [in] transaction The transaction to maybe insert in the database
                                /// \return The index of enum for one of the operation types in the transaction. Not useful most of the time.
                                int putTransaction(soci::session &sql, const cosmos::Transaction &transaction);

                                /// Insert a block metadata in the database
                                /// \param [in] sql The sql session accessing the database
                                /// \param [in] block The block to maybe insert in the database
                                /// \return true if the block was actually inserted/updated. false otherwise (see BlockDatabaseHelper::putBlock)
                                bool putBlock(soci::session &sql, const api::Block &block);

                                std::shared_ptr<CosmosLikeKeychain> getKeychain() const;

                                FuturePtr<Amount> getBalance() override;

                                Future<AbstractAccount::AddressList> getFreshPublicAddresses() override;

                                Future<std::vector<std::shared_ptr<api::Amount>>>
                                getBalanceHistory(const std::string &start,
                                                  const std::string &end,
                                                  api::TimePeriod precision) override;

                                Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point &date) override;

                                bool isSynchronizing() override;

                                std::shared_ptr<api::EventBus> synchronize() override;

                                void startBlockchainObservation() override;

                                void stopBlockchainObservation() override;

                                bool isObservingBlockchain() override;

                                std::string getRestoreKey() override;

                                void broadcastRawTransaction(const std::string &transaction, const std::shared_ptr<api::StringCallback> &callback) override;

                                void broadcastTransaction(const std::shared_ptr<api::CosmosLikeTransaction> &transaction, const std::shared_ptr<api::StringCallback>&callback) override;

                                std::shared_ptr<api::CosmosLikeTransactionBuilder> buildTransaction() override;
                                std::shared_ptr<api::CosmosLikeTransactionBuilder> buildTransaction(const std::string &senderAddress);

                                std::shared_ptr<api::OperationQuery> queryOperations() override;

                                void getEstimatedGasLimit(const std::shared_ptr<api::CosmosLikeTransaction> &transaction, const std::shared_ptr<api::BigIntCallback> &callback) override;

                                void estimateGas(const api::CosmosGasLimitRequest &request,
                                                 const std::shared_ptr<api::BigIntCallback> &callback) override;

                                // Account related data
                                void getSequence(const std::shared_ptr<api::StringCallback>& callback) override;
                                void getAccountNumber(const std::shared_ptr<api::StringCallback>& callback) override;
                                void getWithdrawAddress(const std::shared_ptr<api::StringCallback>& callback) override;
                                cosmos::Account getInfo() const;

                                // Balances
                                /// Ask the explorer for the total balance of the account.
                                /// \return A Future with the total balance
                                FuturePtr<Amount> getTotalBalance() const;
                                void getTotalBalance(const std::shared_ptr<api::AmountCallback> &callback) override;

                                /// Ask the explorer for the total balance *without the pending rewards* of the account.
                                /// \return A Future with the total balance *without the pending rewards*
                                FuturePtr<Amount> getTotalBalanceWithoutPendingRewards() const;

                                /// Ask the explorer for the balance currently delegated by the account.
                                /// \return A Future with the delegated balance
                                FuturePtr<Amount> getDelegatedBalance() const;
                                void getDelegatedBalance(const std::shared_ptr<api::AmountCallback> &callback) override;

                                /// Ask the explorer for the total pending delegation rewards for the account
                                /// \return A Future with the total pending rewards
                                FuturePtr<Amount> getPendingRewardsBalance() const;
                                void getPendingRewardsBalance(const std::shared_ptr<api::AmountCallback> &callback) override;

                                /// Ask the explorer for the total amount in unbonding phase for the account.
                                /// \return A Future with the total amount
                                FuturePtr<Amount> getUnbondingBalance() const;
                                void getUnbondingBalance(const std::shared_ptr<api::AmountCallback> &callback) override;

                                /// Ask the explorer for the spendable balance of the account.
                                /// \return A Future with the spendable balance
                                FuturePtr<Amount> getSpendableBalance() const;
                                void getSpendableBalance(const std::shared_ptr<api::AmountCallback> &callback) override;

                                /// Ask the explorer for the currently active validator set on the chain.
                                /// \return A Future with the list of validators
                                Future<cosmos::ValidatorList> getActiveValidatorSet() const;
                                void getLatestValidatorSet(const std::shared_ptr<api::CosmosLikeValidatorListCallback>& callback) override;
                                /// Ask the explorer for specific informations about a validator. SigningInfo and DistributionInfo are included there.
                                /// \param [in] validatorAddress The 'cosmosvaloper' address of the queried validator.
                                /// \return A Future with the complete information about a validator
                                Future<cosmos::Validator> getValidatorInfo(const std::string& validatorAddress) const;
                                void getValidatorInfo(const std::string &validatorAddress, const std::shared_ptr<api::CosmosLikeValidatorCallback>&callback) override;

                                void getDelegations(const std::shared_ptr<api::CosmosLikeDelegationListCallback> & callback) override;
                                /// Ask the explorer for the current delegations of the account.
                                /// \return A Future with the list of Delegations
                                Future<std::vector<std::shared_ptr<api::CosmosLikeDelegation>>> getDelegations();

                                void getPendingRewards(const std::shared_ptr<api::CosmosLikeRewardListCallback> & callback) override;
                                /// Ask the explorer for the pending rewards specifics of the account.
                                /// \return A Future with the list of pending rewards
                                Future<std::vector<std::shared_ptr<api::CosmosLikeReward>>> getPendingRewards();

                                // "Pending" status info
                                /// Ask the explorer for the current unbondings of the account.
                                /// \return A Future with the list of unbondings
                                Future<std::vector<std::shared_ptr<api::CosmosLikeUnbonding>>> getUnbondings() const;
                                void getUnbondings(const std::shared_ptr<api::CosmosLikeUnbondingListCallback>& callback) override;
                                /// Ask the explorer for the current redelegations of the account.
                                /// \return A Future with the list of redelegations
                                Future<std::vector<std::shared_ptr<api::CosmosLikeRedelegation>>> getRedelegations() const;
                                void getRedelegations(const std::shared_ptr<api::CosmosLikeRedelegationListCallback>& callback) override;

                        private:
                                std::shared_ptr<CosmosLikeAccount> getSelf();
                                void updateFromDb();

                                std::string getAddress() const;

                                // These helpers stay on CosmosLikeAccount *only* because they have to use their
                                // knowledge of Address information in order to correctly map operation type.
                                // An operation type is always seen from the account point of view.

                                /// Set the type and the amount of an Operation from a cosmos Message in the context of the account.
                                /// \param [out] out the Operation to fill
                                /// \param [in] msg the cosmos Message to use as source of information
                                void setOperationTypeAndAmount(CosmosLikeOperation &out, const cosmos::Message &msg) const;
                                /// Set the type and the amount of an Operation from an unwrapped cosmos Send Message.
                                /// \param [out] out the Operation to fill
                                /// \param [in] innerSendMsg the cosmos send message to use, unwrapped.
                                void fillOperationTypeAmountFromSend(CosmosLikeOperation &out, const cosmos::MsgSend &innerSendMsg) const;
                                /// Set the type and the amount of an Operation from an unwrapped cosmos MultiSend Message.
                                /// \param [out] out the Operation to fill
                                /// \param [in] innerMultiSendMsg the cosmos multiSend message to use, unwrapped.
                                void fillOperationTypeAmountFromMultiSend(CosmosLikeOperation &out, const cosmos::MsgMultiSend &innerMultiSendMsg) const;
                                /// Set the type and the amount of an Operation from an unwrapped cosmos Delegate Message.
                                /// \param [out] out the Operation to fill
                                /// \param [in] innerDelegateMsg the cosmos delegate message to use, unwrapped.
                                void fillOperationTypeAmountFromDelegate(CosmosLikeOperation &out, const cosmos::MsgDelegate &innerDelegateMsg) const;
                                /// Set the type and the amount of an Operation from an unwrapped cosmos Undelegate Message.
                                /// \param [out] out the Operation to fill
                                /// \param [in] innerUndelegateMsg the cosmos undelegate message to use, unwrapped.
                                void fillOperationTypeAmountFromUndelegate(CosmosLikeOperation &out, const cosmos::MsgUndelegate &innerUndelegateMsg) const;
                                /// Set the type and the amount of an Operation from an unwrapped cosmos BeginRedelegate Message.
                                /// \param [out] out the Operation to fill
                                /// \param [in] innerBeginRedelegateMsg the cosmos beginRedelegate message to use, unwrapped.
                                void fillOperationTypeAmountFromBeginRedelegate(CosmosLikeOperation &out, const cosmos::MsgBeginRedelegate &innerBeginRedelegateMsg) const;
                                /// Set the type and the amount of an Operation from an unwrapped cosmos SubmitProposal Message.
                                /// \param [out] out the Operation to fill
                                /// \param [in] innerSubmitProposalMsg the cosmos submitProposal message to use, unwrapped.
                                void fillOperationTypeAmountFromSubmitProposal(CosmosLikeOperation &out, const cosmos::MsgSubmitProposal &innerSubmitProposalMsg) const;
                                /// Set the type and the amount of an Operation from an unwrapped cosmos Deposit Message.
                                /// \param [out] out the Operation to fill
                                /// \param [in] innerDepositMsg the cosmos deposit message to use, unwrapped.
                                void fillOperationTypeAmountFromDeposit(CosmosLikeOperation &out, const cosmos::MsgDeposit &innerDepositMsg) const;
                                /// Set the type and the amount of an Operation from an unwrapped cosmos Fees Message.
                                /// \param [out] out the Operation to fill
                                /// \param [in] innerFeesMsg the cosmos fees message to use, unwrapped.
                                void fillOperationTypeAmountFromFees(CosmosLikeOperation &out, const cosmos::MsgFees &innerFeesMsg) const;

                                std::shared_ptr<cosmos::Account> _accountData;
                                std::shared_ptr<CosmosLikeKeychain> _keychain;
                                std::shared_ptr<CosmosLikeBlockchainExplorer> _explorer;
                                std::shared_ptr<CosmosLikeAccountSynchronizer> _synchronizer;
                                std::shared_ptr<CosmosLikeBlockchainObserver> _observer;
                                uint64_t _currentBlockHeight;
                                std::shared_ptr<api::EventBus> _currentSyncEventBus;
                                std::mutex _synchronizationLock;
                };
        }
}
#endif //LEDGER_CORE_COSMOSLIKEACCOUNT_H
