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
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <database/query/QueryBuilder.h>
#include <api/EventCode.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/bitcoin/database/BitcoinLikeUTXODatabaseHelper.h>
#include <api/I32Callback.hpp>
#include <collections/functional.hpp>
#include <wallet/bitcoin/api_impl/BitcoinLikeOutputApi.h>
#include <api/BitcoinLikeOutputListCallback.hpp>
#include <api/BitcoinLikeInput.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/bitcoin/database/BitcoinLikeBlockDatabaseHelper.h>
#include <events/EventPublisher.hpp>
#include <events/Event.hpp>
#include <api/StringCallback.hpp>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeStrategyUtxoPicker.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeStrategyUtxoPicker.h>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <wallet/NetworkTypes.hpp>
#include <spdlog/logger.h>
#include <utils/DateUtils.hpp>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>


namespace ledger {
    namespace core {

        BitcoinLikeAccount::BitcoinLikeAccount(
            const std::shared_ptr<AbstractWallet>& wallet,
            int32_t index,
            const std::shared_ptr<TransactionBroadcaster<BitcoinLikeNetwork>>& broadcaster,
            const std::shared_ptr<BitcoinLikeBlockchainObserver>& observer,
            const std::shared_ptr<AccountSynchronizer>& synchronizer,
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
        BitcoinLikeAccount::inflateOperation(Operation &out,
                                             const std::shared_ptr<const AbstractWallet>& wallet,
                                             const BitcoinLikeNetwork::Transaction &tx) {
            out.accountUid = getAccountUid();
            out.block = tx.block;
            out.bitcoinTransaction = Option<BitcoinLikeNetwork::Transaction>(tx);
            out.currencyName = getWallet()->getCurrency().name;
            out.walletType = getWalletType();
            out.walletUid = wallet->getWalletUid();
            out.date = tx.receivedAt;
            if (out.block.nonEmpty())
                out.block.getValue().currencyName = wallet->getCurrency().name;
            out.bitcoinTransaction.getValue().block = out.block;
        }

        void
        BitcoinLikeAccount::computeOperationTrust(Operation &operation, const std::shared_ptr<const AbstractWallet> &wallet,
                                                  const BitcoinLikeNetwork::Transaction &tx) {
            if (tx.block.nonEmpty()) {
                auto txBlockHeight = tx.block.getValue().height;
                if (_currentBlockHeight > txBlockHeight + 5 ) {
                    operation.trust->setTrustLevel(api::TrustLevel::TRUSTED);
                } else if (_currentBlockHeight > txBlockHeight) {
                    operation.trust->setTrustLevel(api::TrustLevel::UNTRUSTED);
                } else if (_currentBlockHeight == txBlockHeight) {
                    operation.trust->setTrustLevel(api::TrustLevel::PENDING);
                }

            } else {
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
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            if (_currentSyncEventBus)
                return _currentSyncEventBus;
            auto eventPublisher = std::make_shared<EventPublisher>(getContext());
            auto wasEmpty = checkIfWalletIsEmpty();
            _currentSyncEventBus = eventPublisher->getEventBus();
            auto future = _synchronizer->synchronize()->getFuture();
            auto self = std::static_pointer_cast<BitcoinLikeAccount>(shared_from_this());

            auto startTime = DateUtils::now();
            eventPublisher->postSticky(std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()), 0);
            future.onComplete(getContext(), [eventPublisher, self, wasEmpty, startTime] (const Try<Unit>& result) {
                auto isEmpty = self->checkIfWalletIsEmpty();
                api::EventCode code;
                auto payload = std::make_shared<DynamicObject>();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - startTime).count();
                payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                if (result.isSuccess()) {
                    code = !isEmpty && wasEmpty ? api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT
                                                : api::EventCode::SYNCHRONIZATION_SUCCEED;
                } else {
                    code = api::EventCode::SYNCHRONIZATION_FAILED;
                    payload->putString(api::Account::EV_SYNC_ERROR_CODE, api::to_string(result.getFailure().getErrorCode()));
                    payload->putInt(api::Account::EV_SYNC_ERROR_CODE_INT, (int32_t)result.getFailure().getErrorCode());
                    payload->putString(api::Account::EV_SYNC_ERROR_MESSAGE, result.getFailure().getMessage());
                }
                eventPublisher->postSticky(std::make_shared<Event>(code, payload), 0);
                std::lock_guard<std::mutex> lock(self->_synchronizationLock);
                self->_currentSyncEventBus = nullptr;

            });
            return eventPublisher->getEventBus();
        }

        void BitcoinLikeAccount::getUTXO(int32_t from, int32_t to,
                                         const std::shared_ptr<api::BitcoinLikeOutputListCallback> &callback) {
            getUTXO(from, to).callback(getMainExecutionContext(), callback);
        }

        Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>
        BitcoinLikeAccount::getUTXO(int32_t from, int32_t to) {
            auto self = getSelf();
            return async<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>([=] () -> std::vector<std::shared_ptr<api::BitcoinLikeOutput>> {
                auto keychain = self->getKeychain();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                std::vector<BitcoinLikeNetwork::Output> utxo;
                BitcoinLikeUTXODatabaseHelper::queryUTXO(sql, self->getAccountUid(), from, to - from, utxo, [&keychain] (const std::string& addr) {
                    return keychain->contains(addr);
                });
                auto currency = self->getWallet()->getCurrency();
                return functional::map<BitcoinLikeNetwork::Output, std::shared_ptr<api::BitcoinLikeOutput>>(utxo, [&currency] (const BitcoinLikeNetwork::Output& output) -> std::shared_ptr<api::BitcoinLikeOutput> {
                    return std::make_shared<BitcoinLikeOutputApi>(output, currency);
                });
            });
        }


        void BitcoinLikeAccount::getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) {
            getUTXOCount().callback(getMainExecutionContext(), callback);
        }

        Future<int32_t> BitcoinLikeAccount::getUTXOCount() {
            auto self = getSelf();
            return async<int32_t>([=] () -> int32_t {
                auto keychain = self->getKeychain();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                return (int32_t) BitcoinLikeUTXODatabaseHelper::UTXOcount(sql, self->getAccountUid(), [keychain] (const std::string& addr) -> bool {
                    return keychain->contains(addr);
                });
            });
        }

        std::shared_ptr<api::OperationQuery> BitcoinLikeAccount::queryOperations() {
            auto query = std::make_shared<OperationQuery>(
                    api::QueryFilter::accountEq(getAccountUid()),
                    getWallet()->getDatabase(),
                    getWallet()->getContext(),
                    getWallet()->getMainExecutionContext()
            );
            query->registerAccount(shared_from_this());
            return query;
        }

        bool BitcoinLikeAccount::checkIfWalletIsEmpty() {
            return _keychain->isEmpty();
        }

        Future<AbstractAccount::AddressList> BitcoinLikeAccount::getFreshPublicAddresses() {
            auto keychain = getKeychain();
            return async<AbstractAccount::AddressList>([=] () -> AbstractAccount::AddressList {
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
            return getUTXOCount().flatMap<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>(getContext(), [=] (const int32_t& count) -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                return self->getUTXO(0, count);
            });
        }

        FuturePtr<ledger::core::Amount> BitcoinLikeAccount::getBalance() {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return async<std::shared_ptr<Amount>>([=] () -> std::shared_ptr<Amount> {
                const int32_t BATCH_SIZE = 100;
                const auto& uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                std::vector<BitcoinLikeNetwork::Output> utxos;
                auto offset = 0;
                std::size_t count = 0;
                BigInt sum(0);
                auto keychain = self->getKeychain();
                std::function<bool (const std::string&)> filter = [&keychain] (const std::string addr) -> bool {
                    return keychain->contains(addr);
                };
                for (; (count = BitcoinLikeUTXODatabaseHelper::queryUTXO(sql, uid, offset, BATCH_SIZE, utxos, filter)) == BATCH_SIZE; offset += count) {}
                for (const auto& utxo : utxos) {
                    sum = sum + utxo.value;
                }
                return std::make_shared<Amount>(self->getWallet()->getCurrency(), 0, sum);
            });
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        BitcoinLikeAccount::getBalanceHistory(const std::string &start,
                                              const std::string &end,
                                              api::TimePeriod precision) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return async<std::vector<std::shared_ptr<api::Amount>>>([=]() -> std::vector<std::shared_ptr<api::Amount>> {

                auto startDate = DateUtils::fromJSON(start);
                auto endDate = DateUtils::fromJSON(end);
                if (startDate >= endDate) {
                    throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT,
                                         "Start date should be strictly greater than end date");
                }

                const auto &uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                std::vector<Operation> operations;

                auto keychain = self->getKeychain();
                std::function<bool(const std::string &)> filter = [&keychain](const std::string addr) -> bool {
                    return keychain->contains(addr);
                };

                //Get operations related to an account
                OperationDatabaseHelper::queryOperations(sql, uid, operations, filter);

                auto lowerDate = startDate;
                auto upperDate = DateUtils::incrementDate(startDate, precision);

                std::vector<std::shared_ptr<api::Amount>> amounts;
                std::size_t operationsCount = 0;
                BigInt sum;
                while (lowerDate <= endDate && operationsCount < operations.size()) {

                    auto operation = operations[operationsCount];
                    while (operation.date > upperDate && lowerDate < endDate) {
                        lowerDate = DateUtils::incrementDate(lowerDate, precision);
                        upperDate = DateUtils::incrementDate(upperDate, precision);
                        amounts.emplace_back(
                                std::make_shared<ledger::core::Amount>(self->getWallet()->getCurrency(), 0, sum));
                    }

                    if (operation.date <= upperDate) {
                        switch (operation.type) {
                            case api::OperationType::RECEIVE: {
                                sum = sum + operation.amount;
                                break;
                            }
                            case api::OperationType::SEND: {
                                sum = sum - (operation.amount + operation.fees.getValueOr(BigInt::ZERO));
                                break;
                            }
                        }
                    }
                    operationsCount += 1;
                }

                while (lowerDate < endDate) {
                    lowerDate = DateUtils::incrementDate(lowerDate, precision);
                    amounts.emplace_back(
                            std::make_shared<ledger::core::Amount>(self->getWallet()->getCurrency(), 0, sum));
                }

                return amounts;
            });
        }

        std::shared_ptr<BitcoinLikeAccount> BitcoinLikeAccount::getSelf() {
            return std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
        }

        void BitcoinLikeAccount::startBlockchainObservation() {
            _observer->registerAccount(getSelf());
        }

        void BitcoinLikeAccount::stopBlockchainObservation() {
            _observer->unregisterAccount(getSelf());
        }

        bool BitcoinLikeAccount::isObservingBlockchain() {
            return _observer->isRegistered(getSelf());
        }

        Future<std::string> BitcoinLikeAccount::broadcastTransaction(const std::vector<uint8_t> &transaction) {
            _broadcaster->broadcastTransaction(transaction, callback);
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
            auto getUTXO = [self] () -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                return self->getUTXO();
            };
            auto getTransaction = [self] (const std::string& hash) -> FuturePtr<BitcoinLikeNetwork::Transaction> {
                return self->getTransaction(hash);
            };
            return std::make_shared<BitcoinLikeTransactionBuilder>(
                    getContext(),
                    getWallet()->getCurrency(),
                    logger(),
                    _picker->getBuildFunction(getUTXO, getTransaction, _keychain, _currentBlockHeight,logger())
            );
        }

        
        FuturePtr<ledger::core::BitcoinLikeNetwork::Transaction> BitcoinLikeAccount::getTransaction(const std::string& hash) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return async<std::shared_ptr<BitcoinLikeNetwork::Transaction>>([=] () -> std::shared_ptr<BitcoinLikeNetwork::Transaction> {
                auto tx = std::make_shared<BitcoinLikeNetwork::Transaction>();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                if (!BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, hash, *tx)) {
                    throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction {} not found", hash);
                }
                return tx;
            });
        }

        std::shared_ptr<api::BitcoinLikeAccount> BitcoinLikeAccount::asBitcoinLikeAccount() {
            return std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
        }

        std::string BitcoinLikeAccount::getRestoreKey() {
            return _keychain->getRestoreKey();
        }

        Future<api::ErrorCode> BitcoinLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point & date) {
            auto log = logger();

            log->debug(" Start erasing data of account : {}", getAccountUid());
            soci::session sql(getWallet()->getDatabase()->getPool());
            //Update account's internal preferences (for synchronization)
            auto savedState = getInternalPreferences()->getSubPreferences("BlockchainExplorerAccountSynchronizer")->getObject<BlockchainExplorerAccountSynchronizationSavedState>("state");
            if (savedState.nonEmpty()) {
                //Reset batches to blocks mined before given date
                auto previousBlock = BlockDatabaseHelper::getPreviousBlockInDatabase(sql, getWallet()->getCurrency().name, date);
                for (auto& batch : savedState.getValue().batches) {
                    if (previousBlock.nonEmpty() && batch.blockHeight > previousBlock.getValue().height) {
                        batch.blockHeight = (uint32_t) previousBlock.getValue().height;
                        batch.blockHash = previousBlock.getValue().blockHash;
                    } else if (!previousBlock.nonEmpty()) {//if no previous block, sync should go back from genesis block
                        batch.blockHeight = 0;
                        batch.blockHash = "";
                    }
                }
                getInternalPreferences()->getSubPreferences("BlockchainExplorerAccountSynchronizer")->editor()->putObject<BlockchainExplorerAccountSynchronizationSavedState>("state", savedState.getValue())->commit();
            }
            sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ", soci::use(getAccountUid()), soci::use(date);
            log->debug(" Finish erasing data of account : {}", getAccountUid());
            return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
        }

    }
}