/*
 *
 * BitcoinLikeAccount
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/04/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "BitcoinLikeAccount.hpp"
#include <wallet/common/Operation.h>
#include <database/query/QueryBuilder.h>
#include <api/EventCode.hpp>
#include <utils/DateUtils.hpp>
#include <api/I32Callback.hpp>
#include <collections/functional.hpp>
#include <wallet/bitcoin/api_impl/BitcoinLikeOutputApi.h>
#include <api/BitcoinLikeOutputListCallback.hpp>
#include <api/BitcoinLikeInput.hpp>
#include <events/EventPublisher.hpp>
#include <events/Event.hpp>
#include <api/StringCallback.hpp>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeStrategyUtxoPicker.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeStrategyUtxoPicker.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <wallet/NetworkTypes.hpp>
#include <spdlog/logger.h>
#include <utils/DateUtils.hpp>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>


namespace ledger {
    namespace core {
        namespace bitcoin {
            BitcoinLikeAccount::BitcoinLikeAccount(
                const std::shared_ptr<AbstractWallet>& wallet,
                int32_t index,
                const std::shared_ptr<TransactionBroadcaster<BitcoinLikeNetwork>>& broadcaster,
                const std::shared_ptr<BitcoinLikeBlockchainObserver>& observer,
                const std::shared_ptr<core::AccountSynchronizer>& synchronizer,
                const std::shared_ptr<BitcoinLikeKeychain>& keychain)
                : AbstractAccount(wallet, index)
                , _observer(observer)
                , _synchronizer(synchronizer)
                , _keychain(keychain) {
                _keychain->getAllObservableAddresses(0, 40);
                _picker = std::make_shared<BitcoinLikeStrategyUtxoPicker>(getContext(), getWallet()->getCurrency());
                _currentBlockHeight = 0;
            }

            void
                BitcoinLikeAccount::inflateOperation(core::Operation &out,
                    const std::shared_ptr<const AbstractWallet>& wallet,
                    const BitcoinLikeNetwork::Transaction &tx) {
                out.accountUid = getAccountUid();
                out.block = tx.block;
                out.bitcoinTransaction = Option<BitcoinLikeNetwork::Transaction>(tx);
                out.currencyName = getWallet()->getCurrency().name;
                out.walletType = getWalletType();
                out.walletUid = wallet->getWalletUid();
                out.date = tx.receivedAt;
                out.bitcoinTransaction.getValue().block = out.block;
            }

            void
                BitcoinLikeAccount::computeOperationTrust(core::Operation &operation, const std::shared_ptr<const AbstractWallet> &wallet,
                    const BitcoinLikeNetwork::Transaction &tx) {
                if (tx.block.nonEmpty()) {
                    auto txBlockHeight = tx.block.getValue().height;
                    if (_currentBlockHeight > txBlockHeight + 5) {
                        operation.trust->setTrustLevel(api::TrustLevel::TRUSTED);
                    }
                    else if (_currentBlockHeight > txBlockHeight) {
                        operation.trust->setTrustLevel(api::TrustLevel::UNTRUSTED);
                    }
                    else if (_currentBlockHeight == txBlockHeight) {
                        operation.trust->setTrustLevel(api::TrustLevel::PENDING);
                    }

                }
                else {
                    operation.trust->setTrustLevel(api::TrustLevel::DROPPED);
                }
            }

            std::shared_ptr<BitcoinLikeKeychain> BitcoinLikeAccount::getKeychain() const {
                return _keychain;
            }


            bool BitcoinLikeAccount::isSynchronizing() {
                std::lock_guard<std::mutex> lock(_synchronizationLock);
                return _currentSyncEventBus != nullptr;
            }

            std::shared_ptr<api::EventBus> BitcoinLikeAccount::synchronize() {
                throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me");
            }

            void BitcoinLikeAccount::getUTXO(int32_t from, int32_t to,
                const std::shared_ptr<api::BitcoinLikeOutputListCallback> &callback) {
                getUTXO(from, to).callback(getMainExecutionContext(), callback);
            }

            Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>
                BitcoinLikeAccount::getUTXO(int32_t from, int32_t to) {
                return Future< std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>::failure(Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me"));
            }


            void BitcoinLikeAccount::getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) {
                getUTXOCount().callback(getMainExecutionContext(), callback);
            }

            Future<int32_t> BitcoinLikeAccount::getUTXOCount() {
                return Future<int32_t>::failure(Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me"));
            }

            bool BitcoinLikeAccount::checkIfWalletIsEmpty() {
                return _keychain->isEmpty();
            }

            Future<AbstractAccount::AddressList> BitcoinLikeAccount::getFreshPublicAddresses() {
                auto keychain = getKeychain();
                return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
                    auto addrs = keychain->getFreshAddresses(BitcoinLikeKeychain::KeyPurpose::RECEIVE, keychain->getObservableRangeSize());
                    AbstractAccount::AddressList result(addrs.size());
                    auto i = 0;
                    for (auto& addr : addrs) {
                        result[i] = std::dynamic_pointer_cast<api::Address>(addr);
                        i += 1;
                    }
                    return result;
                });
            }

            Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> BitcoinLikeAccount::getUTXO() {
                auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
                return getUTXOCount().flatMap<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>(getContext(), [=](const int32_t& count) -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                    return self->getUTXO(0, count);
                });
            }

            FuturePtr<ledger::core::Amount> BitcoinLikeAccount::getBalance() {
                return FuturePtr<ledger::core::Amount>::failure(Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implemnt me"));
            }

            Future<std::vector<std::shared_ptr<api::Amount>>>
                BitcoinLikeAccount::getBalanceHistory(const std::string &start,
                    const std::string &end,
                    api::TimePeriod precision) {
                return Future<std::vector<std::shared_ptr<api::Amount>>>::failure(Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implemnt me"));
            }

            std::shared_ptr<BitcoinLikeAccount> BitcoinLikeAccount::getSelf() {
                return std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            }

            void BitcoinLikeAccount::startBlockchainObservation() {
                
            }

            void BitcoinLikeAccount::stopBlockchainObservation() {
                
            }

            bool BitcoinLikeAccount::isObservingBlockchain() {
                return false;
            }

            std::shared_ptr<api::OperationQuery> BitcoinLikeAccount::queryOperations() {
                throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me");
            }

            void BitcoinLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                const std::shared_ptr<api::StringCallback> &callback) {
                _broadcaster->broadcastRawTransaction(transaction, callback);
            }

            void BitcoinLikeAccount::broadcastTransaction(const std::shared_ptr<api::BitcoinLikeTransaction> &transaction,
                const std::shared_ptr<api::StringCallback> &callback) {
                broadcastRawTransaction(transaction->serialize(), callback);
            }

            std::shared_ptr<api::BitcoinLikeTransactionBuilder> BitcoinLikeAccount::buildTransaction() {
                auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
                auto getUTXO = [self]() -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                    return self->getUTXO();
                };
                auto getTransaction = [self](const std::string& hash) -> FuturePtr<BitcoinLikeNetwork::Transaction> {
                    return self->getTransaction(hash);
                };
                return std::make_shared<BitcoinLikeTransactionBuilder>(
                    getContext(),
                    getWallet()->getCurrency(),
                    logger(),
                    _picker->getBuildFunction(getUTXO, getTransaction, _keychain, _currentBlockHeight, logger())
                    );
            }


            FuturePtr<ledger::core::BitcoinLikeNetwork::Transaction> BitcoinLikeAccount::getTransaction(const std::string& hash) {
                return  FuturePtr<ledger::core::BitcoinLikeNetwork::Transaction>::failure(make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me"));
            }

            std::shared_ptr<api::BitcoinLikeAccount> BitcoinLikeAccount::asBitcoinLikeAccount() {
                return std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            }

            std::string BitcoinLikeAccount::getRestoreKey() {
                return _keychain->getRestoreKey();
            }

            Future<api::ErrorCode> BitcoinLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point & date) {
                return Future<api::ErrorCode>::failure(Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me"));
            }
        }
    }
}