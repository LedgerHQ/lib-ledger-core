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
#include <soci.h>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <api/TezosLikeAddress.hpp>
#include <api/TezosOperationTag.hpp>
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
#include <wallet/tezos/delegation/TezosLikeOriginatedAccount.h>
#include <events/Event.hpp>
#include <math/Base58.hpp>
#include <utils/Option.hpp>
#include <utils/DateUtils.hpp>
#include <collections/vector.hpp>
#include <database/query/ConditionQueryFilter.h>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <api/TezosLikeTransaction.hpp>
#include <common/AccountHelper.hpp>

using namespace soci;

namespace ledger {
    namespace core {
        Future<api::ErrorCode> TezosLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            auto log = logger();
            log->debug(" Start erasing data of account : {}", getAccountUid());

            std::lock_guard<std::mutex> lock(_synchronizationLock);
            _currentSyncEventBus = nullptr;

            soci::session sql(getWallet()->getDatabase()->getPool());

            //Update account's internal preferences (for synchronization)
            // Clear synchronizer state
            eraseSynchronizerDataSince(sql, date);

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
            auto self = std::static_pointer_cast<TezosLikeAccount>(shared_from_this());
            auto future = _synchronizer->synchronize(self)->getFuture();

            //Update current block height (needed to compute trust level)
            _explorer->getCurrentBlock().onComplete(getContext(),
                                                    [self] (const TryPtr<TezosLikeBlockchainExplorer::Block> &block) mutable {
                                                        if (block.isSuccess()) {
                                                            //TODO
                                                            self->_currentBlockHeight = block.getValue()->height;
                                                        }
                                                    });

            auto startTime = DateUtils::now();
            eventPublisher->postSticky(std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()), 0);
            future.flatMap<BlockchainExplorerAccountSynchronizationResult>(getContext(), [self] (const Try<BlockchainExplorerAccountSynchronizationResult> &result) {
                        // Synchronize originated accounts ...
                        // Notes: We should rid of this part by implementing support for fetching
                        // txs for multiple addresses
                        // Hint: we could add originated accounts to keychain as
                        // managedAccounts and getAllObservableAddresses will return them as well
                        if (self->_originatedAccounts.empty()) {
                            return Future<BlockchainExplorerAccountSynchronizationResult>::successful(result.getValue());
                        }
                        if (result.isFailure()) {
                            return Future<BlockchainExplorerAccountSynchronizationResult>::successful(BlockchainExplorerAccountSynchronizationResult{});
                        }

                        using TxsBulk = TezosLikeBlockchainExplorer::TransactionsBulk;

                        static std::function<Future<BlockchainExplorerAccountSynchronizationResult> (std::shared_ptr<TezosLikeAccount>, size_t, void*, BlockchainExplorerAccountSynchronizationResult)> getTxs =
                                [] (const std::shared_ptr<TezosLikeAccount> &account, size_t id, void *session, BlockchainExplorerAccountSynchronizationResult result) {
                                    std::vector<std::string> addresses{account->_originatedAccounts[id]->getAddress()};

                                    // Get offset to not start sync from beginning
                                    auto offset = session ? Future<std::vector<std::shared_ptr<api::Operation>>>::successful(std::vector<std::shared_ptr<api::Operation>>()) :
                                            std::dynamic_pointer_cast<OperationQuery>(
                                            account->_originatedAccounts[id]->queryOperations()->partial()
                                            )->execute();

                                    return offset.flatMap<BlockchainExplorerAccountSynchronizationResult>(account->getContext(), [=] (const std::vector<std::shared_ptr<api::Operation>> &ops) mutable {
                                        // For the moment we start synchro from the beginning
                                        auto getSession = session ? Future<void *>::successful(session) :
                                                          account->_explorer->startSession();
                                        return getSession.flatMap<BlockchainExplorerAccountSynchronizationResult>(account->getContext(), [=] (void *s) mutable {
                                            return account->_explorer->getTransactions(addresses, std::to_string(ops.size()), s)
                                                    .flatMap<BlockchainExplorerAccountSynchronizationResult>(account->getContext(), [=] (const std::shared_ptr<TxsBulk> &bulk) mutable {
                                                        auto uid = TezosLikeAccountDatabaseHelper::createOriginatedAccountUid(account->getAccountUid(), addresses[0]);
                                                        {
                                                            soci::session sql(account->getWallet()->getDatabase()->getPool());
                                                            soci::transaction tr(sql);
                                                            for (auto &tx : bulk->transactions) {
                                                                auto const flag = account->putTransaction(sql, tx, uid, addresses[0]);

                                                                if (::ledger::core::account::isInsertedOperation(flag)) {
                                                                    ++result.newOperations;
                                                                }

                                                            }
                                                            tr.commit();
                                                        }

                                                        if (bulk->hasNext) {
                                                            return getTxs(account, id, s, result);
                                                        }

                                                        if (id == account->_originatedAccounts.size() - 1) {
                                                            return Future<BlockchainExplorerAccountSynchronizationResult>::successful(result);
                                                        }

                                                        auto killSession = s ? Future<Unit>::successful(Unit()) :
                                                                           account->_explorer->killSession(s);
                                                        return killSession.flatMap<BlockchainExplorerAccountSynchronizationResult>(account->getContext(), [=](const auto&){
                                                            return getTxs(account, id + 1, nullptr, result);
                                                        });
                                                    }).recover(account->getContext(), [] (const Exception& ex) -> BlockchainExplorerAccountSynchronizationResult {
                                                        throw ex;
                                                    });
                                        });
                                    });
                        };
                        return getTxs(self, 0, nullptr, result.getValue());
            }).onComplete(getContext(), [eventPublisher, self, startTime](const auto &result) {
                api::EventCode code;
                auto payload = std::make_shared<DynamicObject>();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        DateUtils::now() - startTime).count();
                payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                if (result.isSuccess()) {
                    code = api::EventCode::SYNCHRONIZATION_SUCCEED;


                    auto const context = result.getValue();

                    payload->putInt(api::Account::EV_SYNC_LAST_BLOCK_HEIGHT, static_cast<int32_t>(self->_currentBlockHeight));
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

        void TezosLikeAccount::saveOptimisticCounter(const std::shared_ptr<BigInt>& counter, const std::string& txHash) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            self->getInternalPreferences()->editor()->putString("waiting_counter", counter->toString())->commit();
            self->getInternalPreferences()->editor()->putString("waiting_counter_last_update", DateUtils::toJSON(DateUtils::now()) )->commit();            
            std::cout << "broadcastTransaction: "<< counter->toString() << " / " << txHash << std::endl;
            //auto waitingTxs = self->getInternalPreferences()->getStringArray("waiting_counter_txs", {});
            //waitingTxs.push_back(txHash);
            //self->getInternalPreferences()->editor()->putStringArray("waiting_counter_txs", waitingTxs)->commit(); 
        }

        void TezosLikeAccount::_broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                       const std::shared_ptr<api::StringCallback> &callback, 
                                                       const std::shared_ptr<BigInt>& counter) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            _explorer->pushTransaction(transaction)
                    .map<std::string>(getContext(),
                                      [self, counter](const String &seq) -> std::string {
                                            auto txHash = seq.str();
                                            std::cout << "txHash=" << txHash << std::endl;
                                            const std::string optimisticStrategy = self->getWallet()->getConfiguration()->getString(
                                                api::TezosConfiguration::TEZOS_COUNTER_STRATEGY).value_or("");
                                            if (optimisticStrategy == "OPTIMISTIC") {
                                                self->saveOptimisticCounter(counter, txHash);                 
                                            }
                                            return txHash;
                                      })
                    .recoverWith(getMainExecutionContext(), [] (const Exception &e) {
                        // Avoid fmt error because of JSON string
                        const auto error = "Failed to broadcast raw transaction, reason " + e.getMessage();
                        return Future<std::string>::failure(
                                Exception(api::ErrorCode::INCOMPLETE_TRANSACTION, error)
                        );
                    })
                    .callback(getMainExecutionContext(), callback);
        }

        void TezosLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                       const std::shared_ptr<api::StringCallback> &callback) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            const std::string optimisticStrategy = self->getWallet()->getConfiguration()->getString(
                api::TezosConfiguration::TEZOS_COUNTER_STRATEGY).value_or("");
            if (optimisticStrategy == "OPTIMISTIC") {
                throw Exception(api::ErrorCode::API_ERROR, "broadcastRawTransaction not authorized with OPTIMISTIC COUNTER" );          
            }
            _broadcastRawTransaction(transaction, callback, nullptr);
        }

        void TezosLikeAccount::broadcastTransaction(const std::shared_ptr<api::TezosLikeTransaction> &transaction,
                                                    const std::shared_ptr<api::StringCallback> &callback) {
            auto counter = std::make_shared<BigInt>(transaction->getCounter()->toString(10));
            auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(transaction);
            if (tx->toReveal()) {
                ++(*counter);
            }
            _broadcastRawTransaction(transaction->serialize(), callback, counter);
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder> TezosLikeAccount::buildTransaction() {
            return buildTransaction(std::dynamic_pointer_cast<TezosLikeAddress>(getKeychain()->getAddress())->toString());
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder> TezosLikeAccount::buildTransaction(const std::string &senderAddress) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            auto buildFunction = [self, senderAddress](const TezosLikeTransactionBuildRequest &request,
                                                       const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer) {

                // Check if balance is sufficient
                auto currency = self->getWallet()->getCurrency();
                auto accountAddress = TezosLikeAddress::fromBase58(senderAddress, currency);
                return explorer->getBalance(std::vector<std::shared_ptr<TezosLikeAddress>>{accountAddress}).flatMapPtr<api::TezosLikeTransaction>(  
                        self->getMainExecutionContext(),
                        [self, request, explorer, accountAddress, currency, senderAddress](const std::shared_ptr<BigInt> &balance) {
                            // Check if all needed values are set
                            if (!request.transactionGasLimit || !request.storageLimit || !request.transactionFees
                                || (request.type != api::TezosOperationTag::OPERATION_TAG_DELEGATION && !request.value && !request.wipe)) {
                                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                                     "Missing mandatory informations (e.g. gasLimit, gasPrice or value).");
                            }
                            // Check if recepient is allocated or not
                            // because if not we have to add additional fees equal to storage_limit (in mXTZ)
                            auto getAllocationFee = [self, explorer, request]() -> Future<BigInt> {
                               if (request.type != api::TezosOperationTag::OPERATION_TAG_TRANSACTION) {
                                   return Future<BigInt>::successful(BigInt::ZERO);
                               }
                                // So here we are looking for unallocated / unfunded accounts
                                return explorer->isFunded(request.toAddress).map<BigInt>(self->getMainExecutionContext(), [request](bool funded) {
                                    // Base unit is uXTZ
                                    return funded ? BigInt::ZERO : *request.storageLimit * BigInt("1000");
                                });
                            };

                            return getAllocationFee()
                                    .flatMapPtr<api::TezosLikeTransaction>(self->getMainExecutionContext(), [=]
                                            (const BigInt &burned) {
                                        auto managerAddress = self->getKeychain()->getAddress()->toString();
                                        auto protocolUpdate = self->getWallet()
                                                ->getConfiguration()
                                                ->getString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE)
                                                .value_or("");
                                        auto tx = std::make_shared<TezosLikeTransactionApi>(currency, protocolUpdate);
                                        // Balance is used only for origination which is always performed from implicit accounts
                                        // In that case senderAddress == self->_keychain->getAddress() so safe to do so
                                        tx->setBalance(*balance);

                                        // Check whether we need a reveal operation
                                        // Note: we can't rely on DB + sent transactions, because
                                        // it is possible to have deleted accounts that hit 0 balance
                                        // during Babylon update (arf ...)
                                        auto setRevealStatus = [self, explorer, tx, senderAddress, managerAddress, request]() {
                                            // So here we are looking for unallocated accounts
                                            return explorer->getManagerKey(senderAddress.find("KT1") == 0 ? managerAddress : senderAddress).map<Unit>(self->getMainExecutionContext(), [tx, request] (const std::string &managerKey) -> Unit {
                                                tx->reveal(managerKey.empty());
                                                std::cout << "Set Reveal:"<< tx->toReveal() << std::endl;
                                                if (tx->toReveal() && (!request.revealGasLimit || !request.revealFees)) {
                                                    throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                                    "Missing mandatory informations (reveal gasPrice or reveal Fees).");
                                                }   
                                                return unit;
                                            });
                                        };

                                        return setRevealStatus().flatMapPtr<api::TezosLikeTransaction>(self->getMainExecutionContext(), [=] (const Unit &result) {
                                            // Check for balance
                                            // Multiply by 2 fees, since in case of reveal op, we input same fees as the ones used
                                            // for transaction op
                                            auto fees = burned +
                                                        (tx->toReveal() ? *request.transactionFees + *request.revealFees : *request.transactionFees);
                                            // If sender is KT account then the managing account is paying the fees ...
                                            if (senderAddress.find("KT1") == 0) {
                                                fees = fees - *request.transactionFees;
                                            }

                                            if (request.type != api::TezosOperationTag::OPERATION_TAG_DELEGATION) {
                                                auto maxPossibleAmountToSend = *balance - fees;
                                            
                                                auto amountToSend = request.wipe ? BigInt::ZERO : *request.value;
                                                if (maxPossibleAmountToSend < amountToSend) {
                                                    std::cout << maxPossibleAmountToSend.to_string() << "<" << amountToSend.to_string() << std::endl;
                                                    throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "Cannot gather enough funds.");
                                                }
                                                tx->setValue(request.wipe ? std::make_shared<BigInt>(maxPossibleAmountToSend) : request.value);
                                            }

                                            // Burned XTZs are not part of the fees
                                            // And if we have a reveal operation, it will be doubled automatically
                                            // since we serialize 2 ops with same fees
                                            tx->setTransactionFees(request.transactionFees);
                                            tx->setTransactionGasLimit(request.transactionGasLimit);
                                            tx->setRevealFees(request.revealFees);
                                            tx->setRevealGasLimit(request.revealGasLimit);
                                            tx->setStorage(request.storageLimit);
                                            
                                            auto getCurveHelper = [] (const std::string &xpubConfig) -> api::TezosCurve {
                                                if (xpubConfig == api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519) {
                                                    return api::TezosCurve::ED25519;
                                                } else if (xpubConfig == api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_SECP256K1) {
                                                    return api::TezosCurve::SECP256K1;
                                                }
                                                return api::TezosCurve::P256;
                                            };
                                            // Get sender's curve first
                                            // For KT accounts, it is always ED25519
                                            auto senderCurve = api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519;
                                            if (senderAddress.find("KT1") != 0) {
                                                senderCurve = self->getKeychain()->getConfiguration()
                                                        ->getString(api::TezosConfiguration::TEZOS_XPUB_CURVE)
                                                        .value_or(api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
                                            }
                                            tx->setSender(accountAddress, getCurveHelper(senderCurve));

                                            if (!request.toAddress.empty()) {
                                                // Get receiver's curve
                                                auto receiverCurve = api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519;
                                                auto receiverPrefix = request.toAddress.substr(0, 3);
                                                std::cout << "receiverPrefix=" << receiverPrefix << std::endl;
                                                if (receiverPrefix == "tz2") {
                                                    receiverCurve = api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_SECP256K1;
                                                } else if (receiverPrefix == "tz3") {
                                                    receiverCurve = api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_P256;
                                                }
                                                tx->setReceiver(TezosLikeAddress::fromBase58(request.toAddress, currency), getCurveHelper(receiverCurve));
                                            }

                                            tx->setSigningPubKey(self->getKeychain()->getPublicKey().getValue());
                                            tx->setManagerAddress(managerAddress,
                                                                  getCurveHelper(
                                                                          self->getKeychain()->getConfiguration()
                                                                          ->getString(api::TezosConfiguration::TEZOS_XPUB_CURVE)
                                                                          .value_or(api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519)));
                                            tx->setType(request.type);
                                            const auto counterAddress = protocolUpdate == api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON ?
                                                                        managerAddress : senderAddress;
                                            return explorer->getCounter(counterAddress
                                            ).flatMapPtr<Block>(self->getMainExecutionContext(), [self, tx, explorer, request] (const std::shared_ptr<BigInt> &explorerCounter) {
                                                if (!explorerCounter) {
                                                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Failed to retrieve counter from network.");
                                                }
                                                
                                                const std::string optimisticStrategy = self->getWallet()->getConfiguration()->getString(
                                                    api::TezosConfiguration::TEZOS_COUNTER_STRATEGY).value_or("");
                                                if (optimisticStrategy == "OPTIMISTIC") {
                                                    self->incrementOptimisticCounter(tx, explorerCounter);
                                                }
                                                else {
                                                    tx->setCounter(std::make_shared<BigInt>(++(*explorerCounter)));
                                                }
                                                return explorer->getCurrentBlock();
                                            }).flatMapPtr<api::TezosLikeTransaction>(self->getMainExecutionContext(), [self, explorer, tx, senderAddress] (const std::shared_ptr<Block> &block) {
                                                tx->setBlockHash(block->hash);
                                                if (senderAddress.find("KT1") == 0) {
                                                    // HACK: KT Operation we use forge endpoint
                                                    return explorer->forgeKTOperation(tx).mapPtr<api::TezosLikeTransaction>(self->getMainExecutionContext(), [tx] (const std::vector<uint8_t> &rawTx) {
                                                        tx->setRawTx(rawTx);
                                                        return tx;
                                                    });
                                                }
                                                return FuturePtr<api::TezosLikeTransaction>::successful(tx);
                                            });
                                        });
                                    });
                                    
                        });
            };
            return std::make_shared<TezosLikeTransactionBuilder>(senderAddress,
                                                                 getMainExecutionContext(),
                                                                 getWallet()->getCurrency(),
                                                                 _explorer,
                                                                 logger(),
                                                                 buildFunction,
                                                                 getWallet()
                                                                         ->getConfiguration()
                                                                         ->getString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE).value_or("")
            );
        }

        void TezosLikeAccount::incrementOptimisticCounter(std::shared_ptr<TezosLikeTransactionApi> tx, const std::shared_ptr<BigInt>& explorerCounter) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            std::cout << "get explorer counter =" << explorerCounter->toInt64() << std::endl;
            auto savedValue = self->getInternalPreferences()->getString("waiting_counter", "0");
            int64_t waitingCounter = BigInt::fromString(savedValue).toInt64();
            std::cout << "get waiting_counter =" << savedValue << std::endl;
            if (waitingCounter != 0) {
                //check counter validity
                savedValue = self->getInternalPreferences()->getString("waiting_counter_last_update", "");
                std::cout << "get waiting_counter_last_update =" << savedValue << std::endl;
                if (!savedValue.empty()) {
                    auto lastUpdate = DateUtils::fromJSON(savedValue);
                    auto timeout = BigInt::fromString(
                        self->getWallet()->getConfiguration()->getString(api::TezosConfiguration::TEZOS_OPTIMISTIC_COUNTER_TIMEOUT).value_or("300000"));                            
                      
                    int64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - lastUpdate).count();
                    std::cout << "duration since last update =" << duration << std::endl;
                    if ( duration > timeout.toInt64()) {
                        waitingCounter = 0;
                        //self->getInternalPreferences()->editor()->putStringArray("waiting_counter_txs", {})->commit();
                        //std::cout << "reset waiting_counter_txs" << std::endl;
                        self->getInternalPreferences()->editor()->putString("waiting_counter", "0")->commit();
                        std::cout << "reset waiting_counter" << std::endl;
                    }
                    //else {
                    //    //keep only not validated counters:
                    //    auto waitingTxs = self->getInternalPreferences()->getStringArray("waiting_counter_txs", {});
                    //    std::cout << "get waiting_counter_txs ="; 
                    //    for (auto& s: waitingTxs) std::cout << s << "-";
                    //    std::cout << std::endl;
                    //    auto waitingTxsSize = waitingCounter - explorerCounter->toInt64();
                    //    if(waitingTxsSize < waitingTxs.size()) {
                    //        waitingTxs = std::vector<std::string>(waitingTxs.end() - waitingTxsSize, waitingTxs.end());
                    //        self->getInternalPreferences()->editor()->putStringArray("waiting_counter_txs", waitingTxs)->commit();
                    //        std::cout << "set waiting_counter_txs ="; 
                    //        for (auto& s: waitingTxs) std::cout << s << "-";
                    //        std::cout << std::endl;
                    //      }
                    //}
                }
            }
            
            int64_t optimisticCounter = std::max(explorerCounter->toInt64(), waitingCounter) ;
            tx->setCounter(std::make_shared<BigInt>(optimisticCounter+1));
        }

        void TezosLikeAccount::addOriginatedAccounts(soci::session &sql, const std::vector<TezosLikeOriginatedAccountDatabaseEntry> &originatedEntries) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            for (auto &originatedEntry : originatedEntries) {
                auto newOriginatedAccount = std::make_shared<TezosLikeOriginatedAccount>(originatedEntry.uid,
                                                                                    originatedEntry.address,
                                                                                    self, originatedEntry.spendable,
                                                                                    originatedEntry.delegatable,
                                                                                    originatedEntry.publicKey);
                _originatedAccounts.push_back(newOriginatedAccount);
            }
        }

        void TezosLikeAccount::getFees(const std::shared_ptr<api::BigIntCallback> & callback) {
            getFees().mapPtr<api::BigInt>(getMainExecutionContext(), [] (const std::shared_ptr<BigInt> &fees) -> std::shared_ptr<api::BigInt>
            {
                if (!fees) {
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Failed to retrieve fees from network");
                }
                return std::make_shared<api::BigIntImpl>(*fees);
            }).callback(getMainExecutionContext(), callback);
        }

        FuturePtr<BigInt> TezosLikeAccount::getFees() {
            return _explorer->getFees();
        }

        void TezosLikeAccount::getCurrentDelegate(const std::shared_ptr<api::StringCallback> & callback) {
            getCurrentDelegate().mapPtr<std::string>(getMainExecutionContext(), [] (const std::shared_ptr<std::string> &delegate) -> std::shared_ptr<std::string>
            {
                if (!delegate) {
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Failed to retrieve current delegate from network");
                }
                return std::make_shared<std::string>(*delegate);
            }).callback(getMainExecutionContext(), callback);
        }

        FuturePtr<std::string> TezosLikeAccount::getCurrentDelegate() {
            return _explorer->getCurrentDelegate();
        }

        std::shared_ptr<api::Keychain> TezosLikeAccount::getAccountKeychain() {
            return _keychain;
        }
    }
}
