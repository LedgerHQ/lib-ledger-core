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
#include <spdlog/logger.h>
#include <utils/DateUtils.hpp>
namespace ledger {
    namespace core {

        BitcoinLikeAccount::BitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                               const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer,
                                               const std::shared_ptr<BitcoinLikeBlockchainObserver> &observer,
                                               const std::shared_ptr<BitcoinLikeAccountSynchronizer> &synchronizer,
                                               const std::shared_ptr<BitcoinLikeKeychain> &keychain)
                : AbstractAccount(wallet, index) {
            _explorer = explorer;
            _observer = observer;
            _synchronizer = synchronizer;
            _keychain = keychain;
            _keychain->getAllObservableAddresses(0, 40);
            _picker = std::make_shared<BitcoinLikeStrategyUtxoPicker>(getContext(), getWallet()->getCurrency());
            _currentBlockHeight = 0;
        }

        void
        BitcoinLikeAccount::inflateOperation(Operation &out,
                                             const std::shared_ptr<const AbstractWallet>& wallet,
                                             const BitcoinLikeBlockchainExplorer::Transaction &tx) {
            out.accountUid = getAccountUid();
            out.block = tx.block;
            out.bitcoinTransaction = Option<BitcoinLikeBlockchainExplorer::Transaction>(tx);
            out.currencyName = getWallet()->getCurrency().name;
            out.walletType = getWalletType();
            out.walletUid = wallet->getWalletUid();
            out.date = tx.receivedAt;
            if (out.block.nonEmpty())
                out.block.getValue().currencyName = wallet->getCurrency().name;
            out.bitcoinTransaction.getValue().block = out.block;
        }

        int BitcoinLikeAccount::putTransaction(soci::session &sql,
                                               const BitcoinLikeBlockchainExplorer::Transaction &transaction) {
            auto wallet = getWallet();
            if (wallet == nullptr) {
                throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
            }
            if (transaction.block.nonEmpty())
                putBlock(sql, transaction.block.getValue());
            auto nodeIndex = std::const_pointer_cast<const BitcoinLikeKeychain>(_keychain)->getFullDerivationScheme().getPositionForLevel(DerivationSchemeLevel::NODE);
            std::list<std::pair<BitcoinLikeBlockchainExplorer::Input *, DerivationPath>> accountInputs;
            std::list<std::pair<BitcoinLikeBlockchainExplorer::Output *, DerivationPath>> accountOutputs;
            uint64_t fees = 0L;
            uint64_t sentAmount = 0L;
            uint64_t receivedAmount = 0L;
            std::vector<std::string> senders;
            senders.reserve(transaction.inputs.size());
            std::vector<std::string> recipients;
            recipients.reserve(transaction.outputs.size());
            int result = 0x00;

            // Find inputs
            for (auto& input : transaction.inputs) {

                if (input.address.nonEmpty()) {
                    senders.push_back(input.address.getValue());
                }
                // Extend input with derivation paths

                if (input.address.nonEmpty() && input.value.nonEmpty()) {
                    auto path = _keychain->getAddressDerivationPath(input.address.getValue());
                    if (path.nonEmpty()) {
                        // This address is part of the account.
                        sentAmount += input.value.getValue().toUint64();
                        accountInputs.push_back(std::make_pair(const_cast<BitcoinLikeBlockchainExplorer::Input *>(&input), DerivationPath(path.getValue())));
                        if (_keychain->markPathAsUsed(DerivationPath(path.getValue()))) {
                            result = result | FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS;
                        } else {
                            result = result | FLAG_TRANSACTION_ON_USED_ADDRESS;
                        }
                    }
                }
                if (input.value.nonEmpty()) {
                    fees += input.value.getValue().toUint64();
                }
            }

            // Find outputs
            auto outputCount = transaction.outputs.size();
            for (auto index = 0; index < outputCount; index++) {
                auto& output = transaction.outputs[index];
                if (output.address.nonEmpty()) {
                    auto path = _keychain->getAddressDerivationPath(output.address.getValue());
                    if (path.nonEmpty()) {
                        DerivationPath p(path.getValue());
                        accountOutputs.push_back(std::make_pair(const_cast<BitcoinLikeBlockchainExplorer::Output *>(&output), p));
                        if (p.getNonHardenedChildNum(nodeIndex) == 1) {
                            if (sentAmount == 0L) {
                                receivedAmount +=  output.value.toUint64();
                            }
                            if (recipients.size() == 0 && index + 1 >= outputCount) {
                                recipients.push_back(output.address.getValue());
                            }
                        } else {
                            receivedAmount += output.value.toUint64();
                            recipients.push_back(output.address.getValue());
                        }
                        if (_keychain->markPathAsUsed(DerivationPath(path.getValue()))) {
                            result = result | FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS;
                        } else {
                            result = result | FLAG_TRANSACTION_ON_USED_ADDRESS;
                        }
                    } else {
                        recipients.push_back(output.address.getValue());
                    }
                }
                fees = fees - output.value.toUint64();
            }
            std::stringstream snds;
            strings::join(senders, snds, ",");

            Operation operation;
            inflateOperation(operation, wallet, transaction);
            operation.senders = std::move(senders);
            operation.recipients = std::move(recipients);
            operation.fees = std::move(BigInt().assignI64(fees));
            operation.trust = std::make_shared<TrustIndicator>();
            operation.date = transaction.receivedAt;

            // Compute trust
            computeOperationTrust(operation, wallet, transaction);

            if (accountInputs.size() > 0) {
                // Create a send operation
                result = result | FLAG_TRANSACTION_CREATED_SENDING_OPERATION;

                for (auto& accountOutput : accountOutputs) {
                    if (accountOutput.second.getNonHardenedChildNum(nodeIndex) == 1)
                        sentAmount -= accountOutput.first->value.toInt64();
                }
                sentAmount -= fees;

                operation.amount.assignI64(sentAmount);
                operation.type = api::OperationType::SEND;
                operation.refreshUid();
                if (OperationDatabaseHelper::putOperation(sql, operation))
                    emitNewOperationEvent(operation);
            }

            if (accountOutputs.size() > 0) {
                // Receive
                BigInt amount;
                auto flag = 0;
                bool filterChangeAddresses = true;

                if (accountInputs.size() == 0) {
                    filterChangeAddresses = false;
                }

                BigInt finalAmount;
                auto accountOutputCount = 0;
                for (auto& o : accountOutputs) {
                    if (filterChangeAddresses && o.second.getNonHardenedChildNum(nodeIndex) == 1)
                        continue;
                    finalAmount = finalAmount + o.first->value;
                    accountOutputCount += 1;
                }
                if (accountOutputCount > 0) {
                    operation.amount = finalAmount;
                    operation.type = api::OperationType::RECEIVE;
                    operation.refreshUid();
                    if (OperationDatabaseHelper::putOperation(sql, operation))
                        emitNewOperationEvent(operation);
                }

                //Update account_uid column of bitcoin_outputs table
                for (auto& o : accountOutputs) {
                    if (o.first->address.nonEmpty()) {
                        int count = 0;
                        sql << "SELECT COUNT(*) FROM bitcoin_outputs WHERE address = :address", soci::use(o.first->address.getValue()), soci::into(count);
                        if (count > 0) {
                            sql << "UPDATE bitcoin_outputs SET account_uid = :accountUid WHERE address = :address",
                                    soci::use(getAccountUid()), soci::use(o.first->address.getValue());
                        }
                    }
                }

            }

            return result;
        }

        void
        BitcoinLikeAccount::computeOperationTrust(Operation &operation, const std::shared_ptr<const AbstractWallet> &wallet,
                                                  const BitcoinLikeBlockchainExplorer::Transaction &tx) {
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
            auto future = _synchronizer->synchronize(std::static_pointer_cast<BitcoinLikeAccount>(shared_from_this()))->getFuture();
            auto self = std::static_pointer_cast<BitcoinLikeAccount>(shared_from_this());

            //Update current block height (needed to compute trust level)
            _explorer->getCurrentBlock().onComplete(getContext(), [self] (const TryPtr<BitcoinLikeBlockchainExplorer::Block>& block) mutable {
                if (block.isSuccess()) {
                    self->_currentBlockHeight = block.getValue()->height;
                }
            });

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
                std::vector<BitcoinLikeBlockchainExplorer::Output> utxo;
                BitcoinLikeUTXODatabaseHelper::queryUTXO(sql, self->getAccountUid(), from, to - from, utxo, [&keychain] (const std::string& addr) {
                    return keychain->contains(addr);
                });
                auto currency = self->getWallet()->getCurrency();
                return functional::map<BitcoinLikeBlockchainExplorer::Output, std::shared_ptr<api::BitcoinLikeOutput>>(utxo, [&currency] (const BitcoinLikeBlockchainExplorer::Output& output) -> std::shared_ptr<api::BitcoinLikeOutput> {
                    return std::make_shared<BitcoinLikeOutputApi>(output, currency);
                });
            });
        }


        void BitcoinLikeAccount::getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) {
            getUTXOCount().callback(getMainExecutionContext(), callback);
        }

        bool BitcoinLikeAccount::putBlock(soci::session &sql, const BitcoinLikeBlockchainExplorer::Block& block) {
            Block abstractBlock;
            abstractBlock.hash = block.hash;
            abstractBlock.currencyName = getWallet()->getCurrency().name;
            abstractBlock.height = block.height;
            abstractBlock.time = block.time;
            if (BlockDatabaseHelper::putBlock(sql, abstractBlock)) {
                BitcoinLikeBlockDatabaseHelper::putBlock(sql, block);
                emitNewBlockEvent(abstractBlock);
                return true;
            }
            return false;
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
                std::vector<BitcoinLikeBlockchainExplorer::Output> utxos;
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

        Future<std::vector<std::shared_ptr<api::Amount>>> BitcoinLikeAccount::getBalanceHistory(const std::string & start,
                                                                                           const std::string & end,
                                                                                           api::TimePeriod precision) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return async<std::vector<std::shared_ptr<api::Amount>>>([=] () -> std::vector<std::shared_ptr<api::Amount>> {

                const int32_t BATCH_SIZE = 100;
                const auto& uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                std::vector<Operation> operations;
                auto offset = 0;
                std::size_t count = 0;

                auto keychain = self->getKeychain();
                std::function<bool (const std::string&)> filter = [&keychain] (const std::string addr) -> bool {
                    return keychain->contains(addr);
                };

                //Query all utxos in date range
                for (; (count = OperationDatabaseHelper::queryOperations(sql, uid, operations, filter)) == BATCH_SIZE; offset += count) {}

                auto startDate = DateUtils::fromJSON(start);
                auto endDate = DateUtils::fromJSON(end);
                auto upperDate = DateUtils::incrementDate(startDate, precision);

                std::vector<std::shared_ptr<api::Amount>> amounts;
                std::size_t operationsCount = 0;
                bool increment = true;
                BigInt sum(0);
                while (upperDate <= endDate) {

                    if(operationsCount < operations.size()) {
                        auto operation = operations[operationsCount];
                        if (operation.date <= upperDate) {
                            increment = false;
                            switch (operation.type) {
                                case api::OperationType::RECEIVE:
                                {
                                    sum = sum + operation.amount;
                                    break;
                                }
                                case api::OperationType::SEND:
                                {
                                    sum = sum - (operation.amount + operation.fees.getValueOr(BigInt::ZERO));
                                    break;
                                }
                            }
                            operationsCount += 1;
                        }
                    }

                    if(increment) {
                        //Save amount
                        amounts.emplace_back(std::make_shared<ledger::core::Amount>(self->getWallet()->getCurrency(), 0, sum));
                        //Increment Date
                        upperDate = DateUtils::incrementDate(upperDate, precision);
                    }

                    increment = true;
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
            return _explorer->pushTransaction(transaction).map<std::string>(getContext(), [] (const String& hash) -> std::string {
                return hash.str();
            });
        }

        void BitcoinLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                         const std::shared_ptr<api::StringCallback> &callback) {
            _explorer->pushTransaction(transaction).map<std::string>(getContext(), [] (const String& seq) -> std::string {
                return seq.str();
            }).callback(getContext(), callback);
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
            auto getTransaction = [self] (const std::string& hash) -> FuturePtr<BitcoinLikeBlockchainExplorer::Transaction> {
                return self->getTransaction(hash);
            };
            return std::make_shared<BitcoinLikeTransactionBuilder>(
                    getContext(),
                    getWallet()->getCurrency(),
                    logger(),
                    _picker->getBuildFunction(getUTXO, getTransaction, _explorer, _keychain, logger())
            );
        }

        const std::shared_ptr<BitcoinLikeBlockchainExplorer> &BitcoinLikeAccount::getExplorer() const {
            return _explorer;
        }

        FuturePtr<ledger::core::BitcoinLikeBlockchainExplorer::Transaction> BitcoinLikeAccount::getTransaction(const std::string& hash) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return async<std::shared_ptr<BitcoinLikeBlockchainExplorer::Transaction>>([=] () -> std::shared_ptr<BitcoinLikeBlockchainExplorer::Transaction> {
                auto tx = std::make_shared<BitcoinLikeBlockchainExplorer::Transaction>();
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

    }
}