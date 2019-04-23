/*
 *
 * RippleLikeAccount
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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


#include "RippleLikeAccount.h"
#include "RippleLikeWallet.h"
#include <async/Future.hpp>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>
#include <wallet/ripple/database/RippleLikeAccountDatabaseHelper.h>
#include <wallet/ripple/explorers/RippleLikeBlockchainExplorer.h>
#include <wallet/ripple/transaction_builders/RippleLikeTransactionBuilder.h>
#include <wallet/ripple/database/RippleLikeTransactionDatabaseHelper.h>
#include <wallet/ripple/api_impl/RippleLikeTransactionApi.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>
#include <events/Event.hpp>
#include <math/Base58.hpp>
#include <utils/Option.hpp>
#include <utils/DateUtils.hpp>

#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>

namespace ledger {
    namespace core {

        RippleLikeAccount::RippleLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                             int32_t index,
                                             const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer,
                                             const std::shared_ptr<RippleLikeBlockchainObserver> &observer,
                                             const std::shared_ptr<RippleLikeAccountSynchronizer> &synchronizer,
                                             const std::shared_ptr<RippleLikeKeychain> &keychain) : AbstractAccount(
                wallet, index) {
            _explorer = explorer;
            _observer = observer;
            _synchronizer = synchronizer;
            _keychain = keychain;
            _accountAddress = keychain->getAddress()->toString();
            _currentLedgerSequence = 0;
        }


        FuturePtr<RippleLikeBlockchainExplorerTransaction> RippleLikeAccount::getTransaction(const std::string &hash) {
            auto self = std::dynamic_pointer_cast<RippleLikeAccount>(shared_from_this());
            return async<std::shared_ptr<RippleLikeBlockchainExplorerTransaction>>(
                    [=]() -> std::shared_ptr<RippleLikeBlockchainExplorerTransaction> {
                        auto tx = std::make_shared<RippleLikeBlockchainExplorerTransaction>();
                        soci::session sql(self->getWallet()->getDatabase()->getPool());
                        if (!RippleLikeTransactionDatabaseHelper::getTransactionByHash(sql, hash, *tx)) {
                            throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction {} not found",
                                                 hash);
                        }
                        return tx;
                    });
        }

        void RippleLikeAccount::inflateOperation(Operation &out,
                                                 const std::shared_ptr<const AbstractWallet> &wallet,
                                                 const RippleLikeBlockchainExplorerTransaction &tx) {
            out.accountUid = getAccountUid();
            out.block = tx.block;
            out.rippleTransaction = Option<RippleLikeBlockchainExplorerTransaction>(tx);
            out.currencyName = getWallet()->getCurrency().name;
            out.walletType = getWalletType();
            out.walletUid = wallet->getWalletUid();
            out.date = tx.receivedAt;
            if (out.block.nonEmpty())
                out.block.getValue().currencyName = wallet->getCurrency().name;
            out.rippleTransaction.getValue().block = out.block;
        }

        int RippleLikeAccount::putTransaction(soci::session &sql,
                                              const RippleLikeBlockchainExplorerTransaction &transaction) {
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

        bool RippleLikeAccount::putBlock(soci::session &sql,
                                         const RippleLikeBlockchainExplorer::Block &block) {
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

        std::shared_ptr<RippleLikeKeychain> RippleLikeAccount::getKeychain() const {
            return _keychain;
        }

        FuturePtr<Amount> RippleLikeAccount::getBalance() {
            std::vector<RippleLikeKeychain::Address> listAddresses{_keychain->getAddress()};
            auto currency = getWallet()->getCurrency();
            return _explorer->getBalance(listAddresses).mapPtr<Amount>(getContext(), [currency](
                    const std::shared_ptr<BigInt> &balance) -> std::shared_ptr<Amount> {
                return std::make_shared<Amount>(currency, 0, BigInt(balance->toString()));
            });
        }

        std::shared_ptr<api::OperationQuery> RippleLikeAccount::queryOperations() {
            auto query = std::make_shared<OperationQuery>(
                    api::QueryFilter::accountEq(getAccountUid()),
                    getWallet()->getDatabase(),
                    getWallet()->getContext(),
                    getWallet()->getMainExecutionContext()
            );
            query->registerAccount(shared_from_this());
            return query;
        }

        Future<AbstractAccount::AddressList> RippleLikeAccount::getFreshPublicAddresses() {
            auto keychain = getKeychain();
            return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
                AbstractAccount::AddressList result{keychain->getAddress()};
                return result;
            });
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        RippleLikeAccount::getBalanceHistory(const std::string &start,
                                             const std::string &end,
                                             api::TimePeriod precision) {
            auto self = std::dynamic_pointer_cast<RippleLikeAccount>(shared_from_this());
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
                            default:
                                break;
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

        Future<api::ErrorCode> RippleLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            auto log = logger();
            log->debug(" Start erasing data of account : {}", getAccountUid());
            soci::session sql(getWallet()->getDatabase()->getPool());
            //Update account's internal preferences (for synchronization)
            auto savedState = getInternalPreferences()->getSubPreferences(
                    "BlockchainExplorerAccountSynchronizer")->getObject<BlockchainExplorerAccountSynchronizationSavedState>(
                    "state");
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
                getInternalPreferences()->getSubPreferences(
                        "BlockchainExplorerAccountSynchronizer")->editor()->putObject<BlockchainExplorerAccountSynchronizationSavedState>(
                        "state", savedState.getValue())->commit();
            }
            auto accountUid = getAccountUid();
            sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ", soci::use(
                    accountUid), soci::use(date);
            log->debug(" Finish erasing data of account : {}", accountUid);
            return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

        }

        bool RippleLikeAccount::isSynchronizing() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            return _currentSyncEventBus != nullptr;
        }

        std::shared_ptr<api::EventBus> RippleLikeAccount::synchronize() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            if (_currentSyncEventBus)
                return _currentSyncEventBus;
            auto eventPublisher = std::make_shared<EventPublisher>(getContext());

            _currentSyncEventBus = eventPublisher->getEventBus();
            auto future = _synchronizer->synchronize(
                    std::static_pointer_cast<RippleLikeAccount>(shared_from_this()))->getFuture();
            auto self = std::static_pointer_cast<RippleLikeAccount>(shared_from_this());

            //Update current block height (needed to compute trust level)
            _explorer->getCurrentBlock().onComplete(getContext(),
                                                    [self](const TryPtr<RippleLikeBlockchainExplorer::Block> &block) mutable {
                                                        if (block.isSuccess()) {
                                                            self->_currentLedgerSequence = block.getValue()->height;
                                                        }
                                                    });

            auto startTime = DateUtils::now();
            eventPublisher->postSticky(
                    std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()),
                    0);
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

        std::shared_ptr<RippleLikeAccount> RippleLikeAccount::getSelf() {
            return std::dynamic_pointer_cast<RippleLikeAccount>(shared_from_this());
        }

        void RippleLikeAccount::startBlockchainObservation() {
            _observer->registerAccount(getSelf());
        }

        void RippleLikeAccount::stopBlockchainObservation() {
            _observer->unregisterAccount(getSelf());
        }

        bool RippleLikeAccount::isObservingBlockchain() {
            return _observer->isRegistered(getSelf());
        }

        std::string RippleLikeAccount::getRestoreKey() {
            return _keychain->getRestoreKey();
        }

        void RippleLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                        const std::shared_ptr<api::StringCallback> &callback) {
            _explorer->pushTransaction(transaction).map<std::string>(getContext(),
                                                                     [](const String &seq) -> std::string {
                                                                         //TODO: optimistic update
                                                                         return seq.str();
                                                                     }).callback(getContext(), callback);
        }

        void RippleLikeAccount::broadcastTransaction(const std::shared_ptr<api::RippleLikeTransaction> &transaction,
                                                     const std::shared_ptr<api::StringCallback> &callback) {
            broadcastRawTransaction(transaction->serialize(), callback);
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder> RippleLikeAccount::buildTransaction() {

            auto self = std::dynamic_pointer_cast<RippleLikeAccount>(shared_from_this());

            // TODO: rm this
            auto getTransaction = [self](
                    const std::string &hash) -> FuturePtr<RippleLikeBlockchainExplorerTransaction> {
                return self->getTransaction(hash);
            };

            auto buildFunction = [self](const RippleLikeTransactionBuildRequest &request,
                                        const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer) -> Future<std::shared_ptr<api::RippleLikeTransaction>> {
                auto currency = self->getWallet()->getCurrency();
                auto tx = std::make_shared<RippleLikeTransactionApi>(self->getWallet()->getCurrency());
                tx->setValue(request.value);
                tx->setFees(request.fees);
                auto address = self->getKeychain()->getAddress();
                auto accountAddress = std::dynamic_pointer_cast<RippleLikeAddress>(address);
                tx->setSender(accountAddress);
                tx->setReceiver(RippleLikeAddress::fromBase58(request.toAddress, currency));
                BigInt ledgerSequence((int64_t)self->_currentLedgerSequence);
                tx->setLedgerSequence(ledgerSequence);
                tx->setSigningPubKey(self->getKeychain()->getPublicKey(accountAddress->toString()).getValue());

                for (auto& memo : request.memos) {
                    tx->addMemo(memo);
                }

                return explorer->getSequence(accountAddress->toString()).mapPtr<api::RippleLikeTransaction>(self->getContext(), [self, tx] (const std::shared_ptr<BigInt> &sequence) -> std::shared_ptr<api::RippleLikeTransaction> {
                    tx->setSequence(BigInt(sequence->toString()) + BigInt("1"));
                    return tx;
                });
            };

            return std::make_shared<RippleLikeTransactionBuilder>(getContext(),
                                                                  getWallet()->getCurrency(),
                                                                  _explorer,
                                                                  logger(),
                                                                  buildFunction);
        }

    }
}
