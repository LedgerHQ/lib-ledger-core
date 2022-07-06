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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS ORnin
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

#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <api/TezosLikeAddress.hpp>
#include <api/TezosLikeTransaction.hpp>
#include <api/TezosOperationTag.hpp>
#include <async/Future.hpp>
#include <collections/vector.hpp>
#include <common/AccountHelper.hpp>
#include <database/query/ConditionQueryFilter.h>
#include <database/soci-date.h>
#include <database/soci-number.h>
#include <database/soci-option.h>
#include <debug/Benchmarker.h>
#include <events/Event.hpp>
#include <fmt/format.h>
#include <math/Base58.hpp>
#include <soci.h>
#include <utils/DateUtils.hpp>
#include <utils/Option.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/tezos/database/TezosLikeTransactionDatabaseHelper.h>
#include <wallet/tezos/delegation/TezosLikeOriginatedAccount.h>
#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>
#include <wallet/tezos/transaction_builders/TezosLikeTransactionBuilder.h>

using namespace soci;

namespace ledger {
    namespace core {

        static const BigInt minimalFees{100000};           // 100utz
        static const BigInt minimalNanotezPerByte{1000};   // 1000ntz
        static const BigInt minimalNanotezPerGazUnit{100}; // 100ntz
        static const std::size_t signatureSize{TezosLikeTransactionApi::SIGNATURE_SIZE_BYTES};

        Future<api::ErrorCode> TezosLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            auto log                  = logger();

            auto eraseDataBenchmarker = std::make_shared<Benchmarker>(
                fmt::format("erase_data_since/{}", tracePrefix()), log);
            eraseDataBenchmarker->start();
            log->debug("[{}] Start erasing data of account : {}", tracePrefix(), getAccountUid());

            std::lock_guard<std::mutex> lock(_synchronizationLock);
            _currentSyncEventBus = nullptr;

            soci::session sql(getWallet()->getDatabase()->getPool());

            // Update account's internal preferences (for synchronization)
            //  Clear synchronizer state
            eraseSynchronizerDataSince(sql, date);

            auto accountUid = getAccountUid();
            TezosLikeTransactionDatabaseHelper::eraseDataSince(sql, accountUid, date);

            log->debug("[{}] Finish erasing data of account : {}", tracePrefix(), accountUid);
            eraseDataBenchmarker->stop();
            return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
        }

        bool TezosLikeAccount::isSynchronizing() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            return _currentSyncEventBus != nullptr;
        }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-function-cognitive-complexity"
        std::shared_ptr<api::EventBus> TezosLikeAccount::synchronize() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            if (_currentSyncEventBus)
                return _currentSyncEventBus;
            auto eventPublisher  = std::make_shared<EventPublisher>(getContext());

            _currentSyncEventBus = eventPublisher->getEventBus();
            auto self            = std::static_pointer_cast<TezosLikeAccount>(shared_from_this());
            auto future          = _synchronizer->synchronizeAccount(self)->getFuture();

            // Update current block height (needed to compute trust level)
            _explorer->getCurrentBlock().onComplete(getContext(),
                                                    [self](const TryPtr<TezosLikeBlockchainExplorer::Block> &block) mutable {
                                                        if (block.isSuccess()) {
                                                            self->_currentBlockHeight = block.getValue()->height;
                                                            soci::session sql(self->getWallet()->getDatabase()->getPool());
                                                            BulkInsertDatabaseHelper::updateBlock(sql, *block.getValue());
                                                            self->_currentBlockHeight = block.getValue()->height;
                                                        }
                                                    });

            auto startTime = DateUtils::now();
            eventPublisher->postSticky(std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()), 0);
            future.flatMap<tezos::AccountSynchronizationContext>(getContext(), [self](const Try<tezos::AccountSynchronizationContext> &result) {
                      // Synchronize originated accounts ...
                      // Notes: We should rid of this part by implementing support for fetching
                      // txs for multiple addresses
                      // Hint: we could add originated accounts to keychain as
                      // managedAccounts and getAllObservableAddresses will return them as well
                      if (self->_originatedAccounts.empty()) {
                          return Future<tezos::AccountSynchronizationContext>::successful(result.getValue());
                      }
                      if (result.isFailure()) {
                          return Future<tezos::AccountSynchronizationContext>::successful(tezos::AccountSynchronizationContext{});
                      }
                      using TxsBulk = TezosLikeBlockchainExplorer::TransactionsBulk;

                      static std::function<Future<tezos::AccountSynchronizationContext>(std::shared_ptr<TezosLikeAccount>, size_t, void *, tezos::AccountSynchronizationContext)> getTxs =
                          [](const std::shared_ptr<TezosLikeAccount> &account, size_t id, void *session, tezos::AccountSynchronizationContext result) {
                              std::vector<std::string> addresses{account->_originatedAccounts[id]->getAddress()};

                              // Get offset to not start sync from beginning
                              auto offset = session ? Future<std::vector<std::shared_ptr<api::Operation>>>::successful(std::vector<std::shared_ptr<api::Operation>>()) : std::dynamic_pointer_cast<OperationQuery>(account->_originatedAccounts[id]->queryOperations()->partial())->execute();

                              return offset.flatMap<tezos::AccountSynchronizationContext>(account->getContext(), [=](const std::vector<std::shared_ptr<api::Operation>> &ops) mutable {
                                  // For the moment we start synchro from the beginning
                                  auto getSession = session ? Future<void *>::successful(session) : account->_explorer->startSession();
                                  return getSession.flatMap<tezos::AccountSynchronizationContext>(account->getContext(), [=](void *s) mutable {
                                      return account->_explorer->getTransactions(addresses, std::to_string(ops.size()), s)
                                          .flatMap<tezos::AccountSynchronizationContext>(account->getContext(), [=](const std::shared_ptr<TxsBulk> &bulk) mutable {
                                              auto uid = TezosLikeAccountDatabaseHelper::createOriginatedAccountUid(account->getAccountUid(), addresses[0]);
                                              {
                                                  std::vector<Operation> operations;
                                                  for (auto &tx : bulk->transactions) {
                                                      tx.originatedAccountUid     = uid;
                                                      tx.originatedAccountAddress = addresses[0];
                                                      account->interpretTransaction(tx, operations);
                                                  }
                                                  auto f = account->bulkInsert(operations);
                                                  if (f.isSuccess()) {
                                                      result.newOperations += operations.size();
                                                  }
                                              }

                                              if (bulk->hasNext) {
                                                  return getTxs(account, id, s, result);
                                              }

                                              if (id == account->_originatedAccounts.size() - 1) {
                                                  account->logger()->info("[{}] Stopping synchronization at originatedAccount index {} ({} originated account detected)",
                                                                          account->tracePrefix(),
                                                                          id,
                                                                          id + 1);
                                                  return Future<tezos::AccountSynchronizationContext>::successful(result);
                                              }
                                              return getTxs(account, id + 1, nullptr, result);
                                          })
                                          .recover(account->getContext(), [](const Exception &ex) -> tezos::AccountSynchronizationContext {
                                              throw ex;
                                          });
                                  });
                              });
                          };
                      return getTxs(self, 0, nullptr, result.getValue());
                  })
                .onComplete(getContext(), [eventPublisher, self, startTime](const auto &result) {
                    api::EventCode code;
                    auto payload  = std::make_shared<DynamicObject>();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        DateUtils::now() - startTime)
                                        .count();
                    payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                    if (result.isSuccess()) {
                        code = api::EventCode::SYNCHRONIZATION_SUCCEED;

                        self->getWallet()->invalidateBalanceCache(self->getIndex());
                        auto const context = result.getValue();

                        payload->putInt(api::Account::EV_SYNC_LAST_BLOCK_HEIGHT, static_cast<int32_t>(context.lastBlockHeight));
                        payload->putInt(api::Account::EV_SYNC_NEW_OPERATIONS, static_cast<int32_t>(context.newOperations));
                    } else {
                        code = api::EventCode::SYNCHRONIZATION_FAILED;
                        payload->putString(api::Account::EV_SYNC_ERROR_CODE,
                                           api::to_string(result.getFailure().getErrorCode()));
                        payload->putInt(api::Account::EV_SYNC_ERROR_CODE_INT, (int32_t)result.getFailure().getErrorCode());
                        payload->putString(api::Account::EV_SYNC_ERROR_MESSAGE, result.getFailure().getMessage());
                    }
                    eventPublisher->postSticky(std::make_shared<Event>(code, payload), 0);
                    std::lock_guard<std::mutex> lock(self->_synchronizationLock);
                    self->_currentSyncEventBus = nullptr;
                });
            return eventPublisher->getEventBus();
        }
#pragma clang diagnostic pop

        std::shared_ptr<TezosLikeAccount> TezosLikeAccount::getSelf() {
            return std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
        }

        std::string TezosLikeAccount::getRestoreKey() {
            return _keychain->getRestoreKey();
        }

        void TezosLikeAccount::saveOptimisticCounter(const std::shared_ptr<BigInt> &counter, const std::string &txHash) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            self->getInternalPreferences()->editor()->putString("waiting_counter", counter->toString())->commit();
            self->getInternalPreferences()->editor()->putString("waiting_counter_last_update", DateUtils::toJSON(DateUtils::now()))->commit();
            std::cout << "broadcastTransaction: " << counter->toString() << " / " << txHash << std::endl;
            // auto waitingTxs = self->getInternalPreferences()->getStringArray("waiting_counter_txs", {});
            // waitingTxs.push_back(txHash);
            // self->getInternalPreferences()->editor()->putStringArray("waiting_counter_txs", waitingTxs)->commit();
        }

        void TezosLikeAccount::_broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                        const std::shared_ptr<api::StringCallback> &callback,
                                                        const std::shared_ptr<BigInt> &counter,
                                                        const std::string &correlationId) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            _explorer->pushTransaction(transaction, correlationId)
                .map<std::string>(getContext(),
                                  [self, counter](const String &seq) -> std::string {
                                      auto txHash = seq.str();
                                      std::cout << "txHash=" << txHash << std::endl;
                                      const std::string optimisticStrategy = self->getWallet()->getConfiguration()->getString(
                                                                                                                      api::TezosConfiguration::TEZOS_COUNTER_STRATEGY)
                                                                                 .value_or("");
                                      if (optimisticStrategy == "OPTIMISTIC") {
                                          self->saveOptimisticCounter(counter, txHash);
                                      }
                                      return txHash;
                                  })
                .recoverWith(getMainExecutionContext(), [](const Exception &e) {
                    // Avoid fmt error because of JSON string
                    const auto error = "Failed to broadcast raw transaction, reason " + e.getMessage();
                    return Future<std::string>::failure(
                        Exception(api::ErrorCode::INCOMPLETE_TRANSACTION, error));
                })
                .callback(getMainExecutionContext(), callback);
        }

        void TezosLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                       const std::shared_ptr<api::StringCallback> &callback) {
            broadcastRawTransaction(transaction, callback, "");
        }

        void TezosLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                       const std::shared_ptr<api::StringCallback> &callback,
                                                       const std::string &correlationId) {
            auto self                            = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            const std::string optimisticStrategy = self->getWallet()->getConfiguration()->getString(
                                                                                            api::TezosConfiguration::TEZOS_COUNTER_STRATEGY)
                                                       .value_or("");
            if (optimisticStrategy == "OPTIMISTIC") {
                throw Exception(api::ErrorCode::API_ERROR, "broadcastRawTransaction not authorized with OPTIMISTIC COUNTER");
            }
            _broadcastRawTransaction(transaction, callback, nullptr, correlationId);
        }

        void TezosLikeAccount::broadcastTransaction(const std::shared_ptr<api::TezosLikeTransaction> &transaction,
                                                    const std::shared_ptr<api::StringCallback> &callback) {
            auto counter = std::make_shared<BigInt>(transaction->getCounter()->toString(10));
            auto tx      = std::dynamic_pointer_cast<TezosLikeTransactionApi>(transaction);
            if (tx->toReveal()) {
                ++(*counter);
            }
            logger()->info("[{}] {} receiving transaction", tracePrefix(), CORRELATIONID_PREFIX(transaction->getCorrelationId()));
            _broadcastRawTransaction(transaction->serialize(), callback, counter, transaction->getCorrelationId());
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder> TezosLikeAccount::buildTransaction() {
            return buildTransaction(std::dynamic_pointer_cast<TezosLikeAddress>(getKeychain()->getAddress())->toString());
        }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-function-cognitive-complexity"
        std::shared_ptr<api::TezosLikeTransactionBuilder> TezosLikeAccount::buildTransaction(const std::string &senderAddress) {
            auto self          = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            auto buildFunction = [self, senderAddress](const TezosLikeTransactionBuildRequest &request,
                                                       const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer) {
                // XTZ self-transactions seems to always fail and existing wallet forbid self-transactions
                // So that's why we prevent user from creating a self-transaction
                // TODO: Get reference to confirm this
                if (request.toAddress == senderAddress) {
                    throw make_exception(api::ErrorCode::INVALID_SELF_TX, "Cannot send funds to sending address!");
                }

                // Check if balance is sufficient
                auto currency       = self->getWallet()->getCurrency();
                auto accountAddress = TezosLikeAddress::fromBase58(senderAddress, currency);
                return explorer->getBalance(std::vector<std::shared_ptr<TezosLikeAddress>>{accountAddress}).flatMapPtr<api::TezosLikeTransaction>(self->getMainExecutionContext(), [self, request, explorer, accountAddress, currency, senderAddress](const std::shared_ptr<BigInt> &balance) {
                    // Check if all needed values are set
                    if (!request.transactionGasLimit || !request.storageLimit || !request.transactionFees || (request.type != api::TezosOperationTag::OPERATION_TAG_DELEGATION && !request.value && !request.wipe)) {
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
                        .flatMapPtr<api::TezosLikeTransaction>(self->getMainExecutionContext(), [=](const BigInt &burned) {
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
                                return explorer->getManagerKey(senderAddress.find("KT1") == 0 ? managerAddress : senderAddress).map<Unit>(self->getMainExecutionContext(), [tx, request](const std::string &managerKey) -> Unit {
                                    tx->reveal(managerKey.empty());
                                    std::cout << "Set Reveal:" << tx->toReveal() << std::endl;
                                    if (tx->toReveal() && (!request.revealGasLimit || !request.revealFees)) {
                                        throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                                             "Missing mandatory informations (reveal gasPrice or reveal Fees).");
                                    }
                                    return unit;
                                });
                            };

                            return setRevealStatus().flatMapPtr<api::TezosLikeTransaction>(self->getMainExecutionContext(), [=](const Unit &result) {

                                            //initialize the value
                                            //note that the value will be recalculated for the wipe mode after calculating fees
                                            if (request.type != api::TezosOperationTag::OPERATION_TAG_DELEGATION) {
                                                tx->setValue(request.wipe ? std::make_shared<BigInt>(BigInt::ZERO) : request.value);
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
                                                }).flatMapPtr<TezosLikeTransactionApi>(self->getMainExecutionContext(), [self, explorer, tx, senderAddress] (const std::shared_ptr<Block> &block) {
                                                tx->setBlockHash(block->hash);
                                                if (senderAddress.find("KT1") == 0) {
                                                    // HACK: KT Operation we use forge endpoint
                                                        return explorer->forgeKTOperation(tx).mapPtr<TezosLikeTransactionApi>(self->getMainExecutionContext(), [tx] (const std::vector<uint8_t> &rawTx) {
                                                            tx->setRawTx(rawTx);
                                                            return tx;
                                                        });
                                                }
                                                    return FuturePtr<TezosLikeTransactionApi>::successful(tx);
                                                }).flatMapPtr<TezosLikeTransactionApi>(self->getMainExecutionContext(), [self, request] (const std::shared_ptr<TezosLikeTransactionApi> &tx) {
                                                    if (request.transactionGasLimit->toInt() == 0) {
                                                        const auto& filledTx = tx;
                                                        return self->estimateGasLimit(filledTx).flatMapPtr<TezosLikeTransactionApi>(self->getMainExecutionContext(), [self, filledTx] (const std::shared_ptr<GasLimit> &gas) -> FuturePtr<TezosLikeTransactionApi> {
                                                            filledTx->setRevealGasLimit(std::make_shared<BigInt>(gas->reveal));
                                                            filledTx->setTransactionGasLimit(std::make_shared<BigInt>(gas->transaction));

                                                            auto computeFees = [](const std::size_t& size, const BigInt& gasLimit) -> BigInt {
                                                                const BigInt fullSize{static_cast<unsigned long long>(size + signatureSize)}; // Bytes
                                                                BigInt truncated = (minimalFees + minimalNanotezPerByte * fullSize + minimalNanotezPerGazUnit * gasLimit) / BigInt(1000); // utz
                                                                return ++truncated;
                                                            };

                                                            const std::size_t txSize = filledTx->serialize().size();
                                                            BigInt computedRevealFees = computeFees(txSize, gas->reveal);
                                                            BigInt computedTransactionFees = computeFees(txSize, gas->transaction);

                                                            const auto revealFees = std::make_shared<BigInt>(std::move(computedRevealFees));
                                                            filledTx->setRevealFees(revealFees);
                                                            const auto transactionFees = std::make_shared<BigInt>(std::move(computedTransactionFees));
                                                            filledTx->setTransactionFees(transactionFees);

                                                            self->logger()->info(
                                                                "Raw transaction fees specifications: txfees={} txgazlimit={} revealfees={} revealgazlimit={}",
                                                                transactionFees->toString(), gas->transaction.toString(),
                                                                revealFees->toString(), gas->reveal.toString()
                                                            );

                                                            return FuturePtr<TezosLikeTransactionApi>::successful(filledTx);
                                                        });
                                                    }
                                                    return FuturePtr<TezosLikeTransactionApi>::successful(tx);
                                                }).flatMapPtr<api::TezosLikeTransaction>(self->getMainExecutionContext(), [self, request, burned, senderAddress, balance] (const std::shared_ptr<TezosLikeTransactionApi> &tx) {
                                                    
                                                    auto fees = burned + BigInt(tx->getFees()->toLong());
                                                    // If sender is KT account then the managing account is paying the fees ...
                                                    if (senderAddress.find("KT1") == 0) {
                                                        fees = fees - BigInt(tx->getFees()->toLong());
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

                                                    std::shared_ptr<api::Amount> value = tx->getValue();

                                                    self->logger()->info(
                                                        "Transaction amount specifications: totalfees={} (including burned={}) value={} oldbalance={}", 
                                                        fees.toString(), burned.toString(),
                                                        value ? value->toString() : "NA", balance->toString()
                                                    );

                                                    return FuturePtr<api::TezosLikeTransaction>::successful(tx);
                                                }); });
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
                                                                     ->getString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE)
                                                                     .value_or(""));
        }
#pragma clang diagnostic pop

        std::string TezosLikeAccount::computeOperationUid(const std::shared_ptr<api::TezosLikeTransaction> &transaction) const {
            // The provided transaction has not necessarily been forged already, so we have to
            // compute the raw transaction first and parse it to make sure we can compute the
            // required operation values, including the operation index.
            std::shared_ptr<api::TezosLikeTransaction> parsedTx;
            {
                auto currency       = getWallet()->getCurrency();
                auto protocolUpdate = getWallet()
                                          ->getConfiguration()
                                          ->getString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE)
                                          .value_or("");

                parsedTx = TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
                    std::move(currency),
                    transaction->serialize(),
                    std::move(protocolUpdate));
            }

            std::string additional;
            api::OperationType opType;
            {
                std::string originatedAccountId;
                std::string originatedAccountAddress;
                if (!_originatedAccounts.empty()) {
                    auto originatedAccount   = std::dynamic_pointer_cast<TezosLikeOriginatedAccount>(_originatedAccounts[0]);
                    originatedAccountId      = originatedAccount->getAccountUid();
                    originatedAccountAddress = originatedAccount->getAddress();
                }

                auto sender = parsedTx->getSender();
                if (!sender) {
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "computeOperationUid: Sender must be provided");
                }
                auto receiver                      = parsedTx->getReceiver();

                const std::string &senderAddress   = sender->toBase58();
                const std::string &receiverAddress = receiver ? receiver->toBase58() : std::string();
                std::tie(opType, additional)       = getOperationTypeAndUidAdditional(
                          senderAddress,
                          receiverAddress,
                          originatedAccountId,
                          originatedAccountAddress);

                if (opType == api::OperationType::NONE) {
                    throw make_exception(
                        api::ErrorCode::RUNTIME_ERROR, "computeOperationUid: Failed to determine the operation type, sender=[%s] receiver=[%s] originatedAccId=[%s] originatedAccAddr=[%s]",
                        senderAddress,
                        receiverAddress,
                        originatedAccountId,
                        originatedAccountAddress);
                }
            }

            std::string txIdBase = fmt::format("{}", parsedTx->getCounter()->intValue());

            std::string &&txId   = Operation::computeTransactionId(txIdBase, parsedTx->getOperationTypeInTransaction(), additional);

            return OperationDatabaseHelper::createUid(getAccountUid(), txId, opType);
        }

        void TezosLikeAccount::incrementOptimisticCounter(std::shared_ptr<TezosLikeTransactionApi> tx, const std::shared_ptr<BigInt> &explorerCounter) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            std::cout << "get explorer counter =" << explorerCounter->toInt64() << std::endl;
            auto savedValue        = self->getInternalPreferences()->getString("waiting_counter", "0");
            int64_t waitingCounter = BigInt::fromString(savedValue).toInt64();
            std::cout << "get waiting_counter =" << savedValue << std::endl;
            if (waitingCounter != 0) {
                // check counter validity
                savedValue = self->getInternalPreferences()->getString("waiting_counter_last_update", "");
                std::cout << "get waiting_counter_last_update =" << savedValue << std::endl;
                if (!savedValue.empty()) {
                    auto lastUpdate = DateUtils::fromJSON(savedValue);
                    auto timeout    = BigInt::fromString(
                           self->getWallet()->getConfiguration()->getString(api::TezosConfiguration::TEZOS_OPTIMISTIC_COUNTER_TIMEOUT).value_or("300000"));

                    int64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - lastUpdate).count();
                    std::cout << "duration since last update =" << duration << std::endl;
                    if (duration > timeout.toInt64()) {
                        waitingCounter = 0;
                        // self->getInternalPreferences()->editor()->putStringArray("waiting_counter_txs", {})->commit();
                        // std::cout << "reset waiting_counter_txs" << std::endl;
                        self->getInternalPreferences()->editor()->putString("waiting_counter", "0")->commit();
                        std::cout << "reset waiting_counter" << std::endl;
                    }
                    // else {
                    //     //keep only not validated counters:
                    //     auto waitingTxs = self->getInternalPreferences()->getStringArray("waiting_counter_txs", {});
                    //     std::cout << "get waiting_counter_txs =";
                    //     for (auto& s: waitingTxs) std::cout << s << "-";
                    //     std::cout << std::endl;
                    //     auto waitingTxsSize = waitingCounter - explorerCounter->toInt64();
                    //     if(waitingTxsSize < waitingTxs.size()) {
                    //         waitingTxs = std::vector<std::string>(waitingTxs.end() - waitingTxsSize, waitingTxs.end());
                    //         self->getInternalPreferences()->editor()->putStringArray("waiting_counter_txs", waitingTxs)->commit();
                    //         std::cout << "set waiting_counter_txs =";
                    //         for (auto& s: waitingTxs) std::cout << s << "-";
                    //         std::cout << std::endl;
                    //       }
                    // }
                }
            }

            int64_t optimisticCounter = std::max(explorerCounter->toInt64(), waitingCounter);
            tx->setCounter(std::make_shared<BigInt>(optimisticCounter + 1));
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

        void TezosLikeAccount::getFees(const std::shared_ptr<api::BigIntCallback> &callback) {
            getFees().mapPtr<api::BigInt>(getMainExecutionContext(), [](const std::shared_ptr<BigInt> &fees) -> std::shared_ptr<api::BigInt> {
                         if (!fees) {
                             throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Failed to retrieve fees from network");
                         }
                         return std::make_shared<api::BigIntImpl>(*fees);
                     })
                .callback(getMainExecutionContext(), callback);
        }

        FuturePtr<BigInt> TezosLikeAccount::getFees() {
            return _explorer->getFees();
        }

        void TezosLikeAccount::getCurrentDelegate(const std::shared_ptr<api::StringCallback> &callback) {
            getCurrentDelegate().callback(getMainExecutionContext(), callback);
        }

        Future<std::string> TezosLikeAccount::getCurrentDelegate() {
            return _explorer->getCurrentDelegate(getKeychain()->getAddress()->toString());
        }

        FuturePtr<GasLimit> TezosLikeAccount::estimateGasLimit(const std::shared_ptr<TezosLikeTransactionApi> &tx, double adjustmentFactor) {
            return _explorer->getEstimatedGasLimit(tx).flatMapPtr<GasLimit>(
                                                          getMainExecutionContext(),
                                                          [adjustmentFactor](const std::shared_ptr<GasLimit> &consumedGas) {
                                                              BigInt adjustedGasReveal(static_cast<int64_t>(consumedGas->reveal.toInt64() * adjustmentFactor));
                                                              BigInt adjustedGasTransaction(static_cast<int64_t>(consumedGas->transaction.toInt64() * adjustmentFactor));
                                                              return Future<std::shared_ptr<GasLimit>>::successful(
                                                                  std::make_shared<GasLimit>(adjustedGasReveal, adjustedGasTransaction));
                                                          })
                .recover(getMainExecutionContext(), [=](const Exception &exception) {
                    logger()->info("[{}] unable to estimate gas limit dynamically: {} ==> using default gas limit", tracePrefix(), exception.getMessage());
                    return std::make_shared<GasLimit>(
                        api::TezosConfigurationDefaults::TEZOS_DEFAULT_GAS_LIMIT, api::TezosConfigurationDefaults::TEZOS_DEFAULT_GAS_LIMIT);
                });
        }

        std::shared_ptr<api::Keychain> TezosLikeAccount::getAccountKeychain() {
            return _keychain;
        }

        std::string TezosLikeAccount::tracePrefix() const {
            return fmt::format("{}/{}/{}", getWallet()->getPool()->getName(), getWallet()->getName(), getIndex());
        }

        const std::string &TezosLikeAccount::getAccountAddress() const {
            return _accountAddress;
        }
    } // namespace core
} // namespace ledger
