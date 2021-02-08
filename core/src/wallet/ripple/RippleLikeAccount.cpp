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
#include <wallet/pool/WalletPool.hpp>
#include <events/Event.hpp>
#include <math/Base58.hpp>
#include <utils/Option.hpp>
#include <utils/DateUtils.hpp>
#include <api/RippleConfiguration.hpp>
#include <api/RippleConfigurationDefaults.hpp>

#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <wallet/ripple/database/RippleLikeOperationDatabaseHelper.hpp>

namespace ledger {
    namespace core {

        RippleLikeAccount::RippleLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                             int32_t index,
                                             const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer,
                                             const std::shared_ptr<RippleLikeAccountSynchronizer> &synchronizer,
                                             const std::shared_ptr<RippleLikeKeychain> &keychain) : AbstractAccount(
                wallet, index) {
            _explorer = explorer;
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

        Try<int> RippleLikeAccount::bulkInsert(const std::vector<Operation> &operations) {
            return Try<int>::from([&] () {
                soci::session sql(getWallet()->getDatabase()->getPool());
                soci::transaction tr(sql);
                RippleLikeOperationDatabaseHelper::bulkInsert(sql, operations);
                tr.commit();
                // Emit
                emitNewOperationsEvent(operations);
                return operations.size();
            });
        }

        void RippleLikeAccount::interpretTransaction(
                const ledger::core::RippleLikeBlockchainExplorerTransaction &transaction,
                std::vector<Operation> &out) {
            auto wallet = getWallet();
            if (wallet == nullptr) {
                throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
            }

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
                setOperationAmount(operation, transaction);
                operation.type = api::OperationType::SEND;
                operation.refreshUid();
                out.push_back(operation);
            }

            if (_accountAddress == transaction.receiver) {
                setOperationAmount(operation, transaction);
                operation.type = api::OperationType::RECEIVE;
                operation.refreshUid();
                out.push_back(operation);
            }
        }

        void RippleLikeAccount::setOperationAmount(
            Operation& operation,
            RippleLikeBlockchainExplorerTransaction const& transaction
        ) const {
            if (transaction.status == 1) {
                operation.amount = transaction.value;
            } else {
                // if the status of the transaction is not correct, we set the operation’s amount to
                // zero as it’s failed (yet fees were still paid)
                operation.amount = BigInt::ZERO;
            }
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
            auto cachedBalance = getWallet()->getBalanceFromCache(getIndex());
            if (cachedBalance.hasValue()) {
                return FuturePtr<Amount>::successful(std::make_shared<Amount>(cachedBalance.getValue()));
            }
            std::vector<RippleLikeKeychain::Address> listAddresses{_keychain->getAddress()};
            auto currency = getWallet()->getCurrency();
            auto self = getSelf();
            return _explorer->getBalance(listAddresses).mapPtr<Amount>(getMainExecutionContext(), [self, currency](
                    const std::shared_ptr<BigInt> &balance) -> std::shared_ptr<Amount> {
                Amount b(currency, 0, BigInt(balance->toString()));
                self->getWallet()->updateBalanceCache(self->getIndex(), b);
                return std::make_shared<Amount>(b);
            });
        }

        std::shared_ptr<api::OperationQuery> RippleLikeAccount::queryOperations() {
            auto query = std::make_shared<OperationQuery>(
                    api::QueryFilter::accountEq(getAccountUid()),
                    getWallet()->getDatabase(),
                    getWallet()->getPool()->getThreadPoolExecutionContext(),
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
                                         "Start date should be strictly lower than end date");
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

            std::lock_guard<std::mutex> lock(_synchronizationLock);
            _currentSyncEventBus = nullptr;

            soci::session sql(getWallet()->getDatabase()->getPool());

            //Update account's internal preferences (for synchronization)
            // Clear synchronizer state
            eraseSynchronizerDataSince(sql, date);
            
            auto accountUid = getAccountUid();
            RippleLikeTransactionDatabaseHelper::eraseDataSince(sql, accountUid, date);

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
            future.onComplete(getContext(), [eventPublisher, self, startTime](const auto &result) {
                api::EventCode code;
                auto payload = std::make_shared<DynamicObject>();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        DateUtils::now() - startTime).count();
                payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                if (result.isSuccess()) {
                    code = api::EventCode::SYNCHRONIZATION_SUCCEED;

                    auto const context = result.getValue();

                    payload->putInt(api::Account::EV_SYNC_LAST_BLOCK_HEIGHT, static_cast<int32_t>(context.lastBlockHeight));
                    payload->putInt(api::Account::EV_SYNC_NEW_OPERATIONS, static_cast<int32_t>(context.newOperations));
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

        std::string RippleLikeAccount::getRestoreKey() {
            return _keychain->getRestoreKey();
        }

        void RippleLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                        const std::shared_ptr<api::StringCallback> &callback) {
            broadcastRawTransaction(transaction).callback(getMainExecutionContext(), callback);
        }

        RippleLikeBlockchainExplorerTransaction RippleLikeAccount::getXRPLikeBlockchainExplorerTxFromRawTx(const std::shared_ptr<RippleLikeAccount> &account,
                                                                                                               const std::string &txHash,
                                                                                                               const std::vector<uint8_t> &rawTx) {
            auto tx = RippleLikeTransactionBuilder::parseRawTransaction(account->getWallet()->getCurrency(), rawTx, true);
            RippleLikeBlockchainExplorerTransaction txExplorer;
            // It is an optimistic so it should be successful (but tx could fail but it will be updated when sync again )
            auto sender = account->getKeychain()->getAddress()->toString();
            txExplorer.status = 1;
            txExplorer.hash = txHash;
            txExplorer.value = BigInt(tx->getValue()->toString());
            txExplorer.fees = BigInt(tx->getFees()->toString());
            txExplorer.sequence = BigInt(tx->getSequence()->toString(10));
            txExplorer.sender = sender;
            txExplorer.receiver = tx->getReceiver()->toBase58();
            txExplorer.receivedAt = std::chrono::system_clock::now();
            txExplorer.memos = tx->getMemos();
            txExplorer.destinationTag = Option<int64_t>(tx->getDestinationTag());
            return txExplorer;
        }

        Future<std::string> RippleLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction) {
            auto self = getSelf();
            return _explorer->pushTransaction(transaction).map<std::string>(getContext(),
                [self, transaction](const String &seq) -> std::string {
                    auto txHash = seq.str();
                    //optimisticUpdate
                    auto txExplorer = getXRPLikeBlockchainExplorerTxFromRawTx(self, txHash, transaction);
                    //Store in DB
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    std::vector<Operation> operations;
                    self->interpretTransaction(txExplorer, operations);
                    self->bulkInsert(operations);
                    return txHash;
                });
        }

        void RippleLikeAccount::broadcastTransaction(const std::shared_ptr<api::RippleLikeTransaction> &transaction,
                                                     const std::shared_ptr<api::StringCallback> &callback) {
            broadcastRawTransaction(transaction->serialize(), callback);
        }

        Future<std::string> RippleLikeAccount::broadcastTransaction(const std::shared_ptr<api::RippleLikeTransaction> &transaction) {
            return broadcastRawTransaction(transaction->serialize());
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

                if (request.destinationTag.hasValue()) {
                    tx->setDestinationTag(request.destinationTag.getValue());
                }

                return explorer->getSequence(accountAddress->toString()).flatMapPtr<api::RippleLikeTransaction>(self->getMainExecutionContext(), [self, tx, explorer] (const std::shared_ptr<BigInt> &sequence) -> FuturePtr<api::RippleLikeTransaction> {
                    tx->setSequence(*sequence);

                    return explorer->getLedgerSequence().mapPtr<api::RippleLikeTransaction>(self->getMainExecutionContext(), [self, tx] (const std::shared_ptr<BigInt>& ledgerSequence) -> std::shared_ptr<api::RippleLikeTransaction> {
                        // according to this <https://xrpl.org/reliable-transaction-submission.html#lastledgersequence>,
                        // we should set the LastLedgerSequence value on every transaction; advised to
                        // use the ledger index of the latest valid ledger + 4
                        auto offset = self->getWallet()->getConfiguration()->getInt(
                            api::RippleConfiguration::RIPPLE_LAST_LEDGER_SEQUENCE_OFFSET
                        ).value_or(api::RippleConfigurationDefaults::RIPPLE_DEFAULT_LAST_LEDGER_SEQUENCE_OFFSET);

                        tx->setLedgerSequence(*ledgerSequence + BigInt(offset));

                        return tx;
                    });
                });
            };

            return std::make_shared<RippleLikeTransactionBuilder>(getMainExecutionContext(),
                                                                  getWallet()->getCurrency(),
                                                                  _explorer,
                                                                  logger(),
                                                                  buildFunction);
        }

        void RippleLikeAccount::getFees(const std::shared_ptr<api::AmountCallback> & callback) {
            getFees().callback(getMainExecutionContext(), callback);
        }

        FuturePtr<api::Amount> RippleLikeAccount::getFees() {
            auto self = shared_from_this();
            return _explorer->getFees().mapPtr<api::Amount>(getMainExecutionContext(), [self] (const std::shared_ptr<BigInt> &fees) {
                // Fees in drops
                return std::make_shared<Amount>(self->getWallet()->getCurrency(), 0, *fees);
            });
        }

        void RippleLikeAccount::getBaseReserve(const std::shared_ptr<api::AmountCallback> & callback) {
            getBaseReserve().callback(getMainExecutionContext(), callback);
        }

        FuturePtr<api::Amount> RippleLikeAccount::getBaseReserve() {
            auto self = shared_from_this();
            return _explorer->getBaseReserve().mapPtr<api::Amount>(getMainExecutionContext(), [self] (const std::shared_ptr<BigInt> &reserve) {
                // Reserve in XRPs
                return std::make_shared<Amount>(self->getWallet()->getCurrency(), 0, *reserve);
            });
        }

        void RippleLikeAccount::isAddressActivated(const std::string &address,
                                                   const std::shared_ptr<api::BoolCallback> &isActivated) {
            isAddressActivated(address).callback(getMainExecutionContext(), isActivated);
        }

        Future<bool> RippleLikeAccount::isAddressActivated(const std::string &address) {
            auto xrpAddress = RippleLikeAddress::parse(address, getWallet()->getCurrency());
            if (!xrpAddress) {
                return Future<bool>::successful(false);
            }
            auto self = getSelf();
            return _explorer->getBaseReserve().flatMap<bool>(getMainExecutionContext(), [self, xrpAddress](const std::shared_ptr<BigInt> &reserve) -> Future<bool> {
                if (!reserve) {
                    return Future<bool>::successful(false);
                }
                return self->_explorer->getBalance(
                        std::vector<RippleLikeKeychain::Address>{
                                std::dynamic_pointer_cast<RippleLikeAddress>(xrpAddress)
                        }).map<bool>(self->getMainExecutionContext(), [reserve](const std::shared_ptr<BigInt> &balance) -> bool {
                    if (!balance) {
                        return false;
                    }
                    return reserve->compare(*balance) < 0;
                });
            });
        }

        std::shared_ptr<api::Keychain> RippleLikeAccount::getAccountKeychain() {
            return _keychain;
        }
    }
}
