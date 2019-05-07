/*
 *
 * TezosLikeAccount
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#include "TezosLikeAccount.h"
#include "TezosLikeWallet.h"
#include <async/Future.hpp>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/tezos/database/TezosLikeTransactionDatabaseHelper.h>
#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>
#include <wallet/tezos/transaction_builders/TezosLikeTransactionBuilder.h>
#include <events/Event.hpp>
#include <math/Base58.hpp>
#include <utils/Option.hpp>
#include <utils/DateUtils.hpp>

#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>

namespace ledger {
    namespace core {

        TezosLikeAccount::TezosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                           int32_t index,
                                           const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                                           const std::shared_ptr<TezosLikeBlockchainObserver> &observer,
                                           const std::shared_ptr<TezosLikeAccountSynchronizer> &synchronizer,
                                           const std::shared_ptr<TezosLikeKeychain> &keychain) : AbstractAccount(wallet, index) {
            _explorer = explorer;
            _observer = observer;
            _synchronizer = synchronizer;
            _keychain = keychain;
            _accountAddress = keychain->getAddress()->toString();
        }


        FuturePtr<TezosLikeBlockchainExplorerTransaction> TezosLikeAccount::getTransaction(const std::string &hash) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            return async<std::shared_ptr<TezosLikeBlockchainExplorerTransaction>>(
                    [=]() -> std::shared_ptr<TezosLikeBlockchainExplorerTransaction> {
                        auto tx = std::make_shared<TezosLikeBlockchainExplorerTransaction>();
                        soci::session sql(self->getWallet()->getDatabase()->getPool());
                        if (!TezosLikeTransactionDatabaseHelper::getTransactionByHash(sql, hash, *tx)) {
                            throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction {} not found",
                                                 hash);
                        }
                        return tx;
                    });
        }

        void TezosLikeAccount::inflateOperation(Operation &out,
                                                const std::shared_ptr<const AbstractWallet> &wallet,
                                                const TezosLikeBlockchainExplorerTransaction &tx) {
            out.accountUid = getAccountUid();
            out.block = tx.block;
            out.tezosTransaction = Option<TezosLikeBlockchainExplorerTransaction>(tx);
            out.currencyName = getWallet()->getCurrency().name;
            out.walletType = getWalletType();
            out.walletUid = wallet->getWalletUid();
            out.date = tx.receivedAt;
            if (out.block.nonEmpty())
                out.block.getValue().currencyName = wallet->getCurrency().name;
            out.tezosTransaction.getValue().block = out.block;
        }

        int TezosLikeAccount::putTransaction(soci::session &sql,
                                             const TezosLikeBlockchainExplorerTransaction &transaction) {
            auto wallet = getWallet();
            if (wallet == nullptr) {
                throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
            }

            if (transaction.block.nonEmpty()) {
                putBlock(sql, transaction.block.getValue());
            }

            int result = FLAG_TRANSACTION_UPDATED;

            Operation operation;
            inflateOperation(operation, wallet, transaction);
            std::vector<std::string> senders{transaction.sender};
            operation.senders = std::move(senders);
            std::vector<std::string> receivers{transaction.receiver};
            operation.recipients = std::move(receivers);
            operation.fees = transaction.fees;
            operation.trust = std::make_shared<TrustIndicator>();
            operation.date = transaction.receivedAt;


            if (_accountAddress == transaction.sender) {
                operation.amount = transaction.value;
                operation.type = api::OperationType::SEND;
                operation.refreshUid();
                if (OperationDatabaseHelper::putOperation(sql, operation)) {
                    emitNewOperationEvent(operation);
                }
                result = FLAG_NEW_TRANSACTION;
            }

            if (_accountAddress == transaction.receiver) {
                operation.amount = transaction.value;
                operation.type = api::OperationType::RECEIVE;
                operation.refreshUid();
                if (OperationDatabaseHelper::putOperation(sql, operation)) {
                    emitNewOperationEvent(operation);
                }
                result = FLAG_NEW_TRANSACTION;
            }

            return result;
        }

        bool TezosLikeAccount::putBlock(soci::session &sql,
                                        const TezosLikeBlockchainExplorer::Block &block) {
            Block abstractBlock;
            abstractBlock.hash = block.hash;
            abstractBlock.currencyName = getWallet()->getCurrency().name;
            abstractBlock.height = block.height;
            abstractBlock.time = block.time;
            if (BlockDatabaseHelper::putBlock(sql, abstractBlock)) {
                emitNewBlockEvent(abstractBlock);
                return true;
            }
            return false;
        }

        std::shared_ptr<TezosLikeKeychain> TezosLikeAccount::getKeychain() const {
            return _keychain;
        }

        FuturePtr<Amount> TezosLikeAccount::getBalance() {
            std::vector<TezosLikeKeychain::Address> listAddresses{_keychain->getAddress()};
            auto currency = getWallet()->getCurrency();
            return _explorer->getBalance(listAddresses).mapPtr<Amount>(getContext(), [currency](
                    const std::shared_ptr<BigInt> &balance) -> std::shared_ptr<Amount> {
                return std::make_shared<Amount>(currency, 0, BigInt(balance->toString()));
            });
        }

        std::shared_ptr<api::OperationQuery> TezosLikeAccount::queryOperations() {
            auto query = std::make_shared<OperationQuery>(
                    api::QueryFilter::accountEq(getAccountUid()),
                    getWallet()->getDatabase(),
                    getWallet()->getContext(),
                    getWallet()->getMainExecutionContext()
            );
            query->registerAccount(shared_from_this());
            return query;
        }

        void TezosLikeAccount::getEstimatedGasLimit(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) {
            _explorer->getEstimatedGasLimit(address).mapPtr<api::BigInt>(getContext(), [] (const std::shared_ptr<BigInt> &gasLimit) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<api::BigIntImpl>(*gasLimit);
            }).callback(getContext(), callback);
        }

        void TezosLikeAccount::getStorage(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) {
            _explorer->getStorage(address).mapPtr<api::BigInt>(getContext(), [] (const std::shared_ptr<BigInt> &storage) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<api::BigIntImpl>(*storage);
            }).callback(getContext(), callback);
        }


        Future<AbstractAccount::AddressList> TezosLikeAccount::getFreshPublicAddresses() {
            auto keychain = getKeychain();
            return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
                AbstractAccount::AddressList result{keychain->getAddress()};
                return result;
            });
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        TezosLikeAccount::getBalanceHistory(const std::string &start,
                                            const std::string &end,
                                            api::TimePeriod precision) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
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

        Future<api::ErrorCode> TezosLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            auto log = logger();
            log->debug(" Start erasing data of account : {}", getAccountUid());
            soci::session sql(getWallet()->getDatabase()->getPool());
            //Update account's internal preferences (for synchronization)
            auto savedState = getInternalPreferences()->getSubPreferences("BlockchainExplorerAccountSynchronizer")->getObject<BlockchainExplorerAccountSynchronizationSavedState>("state");
            if (savedState.nonEmpty()) {
                //Reset batches to blocks mined before given date
                auto previousBlock = BlockDatabaseHelper::getPreviousBlockInDatabase(sql,
                                                                                     getWallet()->getCurrency().name,
                                                                                     date);
                for (auto &batch : savedState.getValue().batches) {
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
            auto accountUid = getAccountUid();
            sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ", soci::use(
                    accountUid), soci::use(date);
            log->debug(" Finish erasing data of account : {}", accountUid);
            return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

        }

        bool TezosLikeAccount::isSynchronizing() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            return _currentSyncEventBus != nullptr;
        }

        std::shared_ptr<api::EventBus> TezosLikeAccount::synchronize() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            if (_currentSyncEventBus)
                return _currentSyncEventBus;
            auto eventPublisher = std::make_shared<EventPublisher>(getContext());

            _currentSyncEventBus = eventPublisher->getEventBus();
            auto future = _synchronizer->synchronize(
                    std::static_pointer_cast<TezosLikeAccount>(shared_from_this()))->getFuture();
            auto self = std::static_pointer_cast<TezosLikeAccount>(shared_from_this());

            //Update current block height (needed to compute trust level)
            _explorer->getCurrentBlock().onComplete(getContext(),
                                                    [self](const TryPtr<TezosLikeBlockchainExplorer::Block> &block) mutable {
                                                        if (block.isSuccess()) {
                                                            //TODO
                                                        }
                                                    });

            auto startTime = DateUtils::now();
            eventPublisher->postSticky(std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()), 0);
            future.onComplete(getContext(), [eventPublisher, self, startTime](const Try<Unit> &result) {
                api::EventCode code;
                auto payload = std::make_shared<DynamicObject>();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        DateUtils::now() - startTime).count();
                payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                if (result.isSuccess()) {
                    code = api::EventCode::SYNCHRONIZATION_SUCCEED;
                } else {
                    code = api::EventCode::SYNCHRONIZATION_FAILED;
                    payload->putString(api::Account::EV_SYNC_ERROR_CODE,
                                       api::to_string(result.getFailure().getErrorCode()));
                    payload->putInt(api::Account::EV_SYNC_ERROR_CODE_INT, (int32_t) result.getFailure().getErrorCode());
                    payload->putString(api::Account::EV_SYNC_ERROR_MESSAGE, result.getFailure().getMessage());
                }
                eventPublisher->postSticky(std::make_shared<Event>(code, payload), 0);
                std::lock_guard<std::mutex> lock(self->_synchronizationLock);
                self->_currentSyncEventBus = nullptr;

            });
            return eventPublisher->getEventBus();
        }

        std::shared_ptr<TezosLikeAccount> TezosLikeAccount::getSelf() {
            return std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
        }

        void TezosLikeAccount::startBlockchainObservation() {
            _observer->registerAccount(getSelf());
        }

        void TezosLikeAccount::stopBlockchainObservation() {
            _observer->unregisterAccount(getSelf());
        }

        bool TezosLikeAccount::isObservingBlockchain() {
            return _observer->isRegistered(getSelf());
        }

        std::string TezosLikeAccount::getRestoreKey() {
            return _keychain->getRestoreKey();
        }

        void TezosLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                       const std::shared_ptr<api::StringCallback> &callback) {
            _explorer->pushTransaction(transaction).map<std::string>(getContext(),
                                                                     [](const String &seq) -> std::string {
                                                                         //TODO: optimistic update
                                                                         return seq.str();
                                                                     }).callback(getContext(), callback);
        }

        void TezosLikeAccount::broadcastTransaction(const std::shared_ptr<api::TezosLikeTransaction> &transaction,
                                                    const std::shared_ptr<api::StringCallback> &callback) {
            broadcastRawTransaction(transaction->serialize(), callback);
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder> TezosLikeAccount::buildTransaction() {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            auto buildFunction = [self](const TezosLikeTransactionBuildRequest &request,
                                        const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer) -> Future<std::shared_ptr<api::TezosLikeTransaction>> {
                auto currency = self->getWallet()->getCurrency();
                auto tx = std::make_shared<TezosLikeTransactionApi>(self->getWallet()->getCurrency());
                tx->setValue(request.value);
                tx->setFees(request.fees);
                auto address = self->getKeychain()->getAddress();
                auto accountAddress = std::dynamic_pointer_cast<TezosLikeAddress>(address);
                tx->setSender(accountAddress);
                tx->setReceiver(TezosLikeAddress::fromBase58(request.toAddress, currency));
                tx->setSigningPubKey(self->getKeychain()->getPublicKey(accountAddress->toString()).getValue());
                return explorer->getCounter(request.toAddress).mapPtr<api::TezosLikeTransaction>(self->getContext(), [self, tx] (const std::shared_ptr<BigInt> &nonce) {
                    tx->setCounter(nonce);
                    return tx;
                }).flatMapPtr<Block>(self->getContext(), [explorer] (const std::shared_ptr<api::TezosLikeTransaction> &transaction) {
                    return explorer->getCurrentBlock();
                }).mapPtr<api::TezosLikeTransaction>(self->getContext(), [tx] (const std::shared_ptr<Block> &block) {
                    tx->setBlockHash(block->hash);
                    return tx;
                });
            };
            return std::make_shared<TezosLikeTransactionBuilder>(getContext(),
                                                                 getWallet()->getCurrency(),
                                                                 _explorer,
                                                                 logger(),
                                                                 buildFunction);
        }

    }
}
