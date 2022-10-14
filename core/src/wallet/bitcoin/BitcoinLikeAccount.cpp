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

#include <algorithm>
#include <api/BitcoinLikeInput.hpp>
#include <api/BitcoinLikeOutputListCallback.hpp>
#include <api/EventCode.hpp>
#include <api/I32Callback.hpp>
#include <api/StringCallback.hpp>
#include <collections/functional.hpp>
#include <database/query/QueryBuilder.h>
#include <database/soci-date.h>
#include <database/soci-number.h>
#include <database/soci-option.h>
#include <events/Event.hpp>
#include <events/EventPublisher.hpp>
#include <memory>
#include <numeric>
#include <spdlog/logger.h>
#include <utils/Cached.h>
#include <utils/DateUtils.hpp>
#include <wallet/bitcoin/api_impl/BitcoinLikeOutputApi.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <wallet/bitcoin/database/BitcoinLikeBlockDatabaseHelper.h>
#include <wallet/bitcoin/database/BitcoinLikeOperationDatabaseHelper.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/bitcoin/database/BitcoinLikeUTXODatabaseHelper.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeStrategyUtxoPicker.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include <wallet/common/Operation.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>

namespace {
    using namespace ledger::core;
    /// Constructor of BitcoinLikeBlockchainExplorerTransaction that initializes some fields
    /// with a just signed transaction.
    ///
    /// This is used for optimistic updates
    BitcoinLikeBlockchainExplorerTransaction initOptimisticUpdate(
        const std::string &txHash,
        const std::shared_ptr<api::BitcoinLikeTransaction> &justSentTx) {
        BitcoinLikeBlockchainExplorerTransaction txExplorer;
        txExplorer.hash          = txHash;
        txExplorer.lockTime      = justSentTx->getLockTime();
        txExplorer.receivedAt    = std::chrono::system_clock::now();
        txExplorer.version       = justSentTx->getVersion();
        txExplorer.confirmations = 0;
        return txExplorer;
    }

    /// Fill the inputs of an optimistic update transaction before it is inserted in
    /// database
    ///
    /// Throw if the previous inputs cannot be found.
    void inflateOptimisticUpdateInputs(
        BitcoinLikeBlockchainExplorerTransaction &toFillTx,
        const std::shared_ptr<api::BitcoinLikeTransaction> &justSentTx,
        const std::string &accountUid,
        soci::session &sql) {
        auto inputCount = justSentTx->getInputs().size();
        for (auto index = 0; index < inputCount; index++) {
            auto input = justSentTx->getInputs()[index];
            BitcoinLikeBlockchainExplorerInput in;
            in.index               = index;
            auto prevTxHash        = input->getPreviousTxHash().value_or("");
            auto prevTxOutputIndex = input->getPreviousOutputIndex().value_or(0);
            BitcoinLikeBlockchainExplorerTransaction prevTx;
            if (!BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(
                    sql, prevTxHash, accountUid, prevTx) ||
                prevTxOutputIndex >= prevTx.outputs.size()) {
                throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND,
                                     "Transaction {} not found while broadcasting",
                                     prevTxHash);
            }

            // With the "foreign outputs" being merged in database,
            // we MUST manually scan the "prevTx" to find the output with the matching
            // index.
            //
            // It should not affect performance too much, as prevTx.outputs has
            // (n_keychain_in_tx+1) elements, where n_keychain_in_tx is the number of
            // _owned_ addresses in the outputs of prevTx. This assertion is true
            // because of the "foreign outputs" merging feature.
            auto isSameIndex =
                [prevTxOutputIndex](const BitcoinLikeBlockchainExplorerOutput &output) {
                    return output.index == prevTxOutputIndex;
                };
            auto prevTxOutput = std::find_if(prevTx.outputs.cbegin(),
                                             prevTx.outputs.cend(), isSameIndex);
            if (prevTxOutput == std::end(prevTx.outputs)) {
                throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND,
                                     "Output {}/{} not found in database", prevTxHash,
                                     prevTxOutputIndex);
            }

            in.value                 = prevTxOutput->value;
            in.signatureScript       = hex::toString(input->getScriptSig());
            in.previousTxHash        = prevTxHash;
            in.previousTxOutputIndex = prevTxOutputIndex;
            in.sequence              = input->getSequence();
            in.address               = prevTxOutput->address.getValueOr("");
            toFillTx.inputs.push_back(in);
        }
    }

    /// Fill the outputs of an optimistic update transaction before it is inserted
    /// in database
    void inflateOptimisticUpdateOutputs(
        BitcoinLikeBlockchainExplorerTransaction &toFillTx,
        const std::shared_ptr<api::BitcoinLikeTransaction> &justSentTx) {
        auto outputCount = justSentTx->getOutputs().size();
        for (auto index = 0; index < outputCount; index++) {
            auto output = justSentTx->getOutputs()[index];
            BitcoinLikeBlockchainExplorerOutput out;
            out.value           = BigInt(output->getValue()->toString());
            out.time            = DateUtils::toJSON(std::chrono::system_clock::now());
            out.transactionHash = output->getTransactionHash();
            out.index           = output->getOutputIndex();
            out.script          = hex::toString(output->getScript());
            out.address         = output->getAddress().value_or("");
            toFillTx.outputs.push_back(out);
        }
    }

    /// Fill the fees of an optimistic update transaction
    /// from its current inputs and outputs
    void inflateOptimisticUpdateFees(
        BitcoinLikeBlockchainExplorerTransaction &toFillTx) {
        auto outputValue = std::accumulate(
            toFillTx.outputs.cbegin(),
            toFillTx.outputs.cend(),
            BigInt::ZERO,
            [](BigInt acc, BitcoinLikeBlockchainExplorerOutput new_val) {
                return std::move(acc) + new_val.value;
            });
        auto inputValue = std::accumulate(
            toFillTx.inputs.cbegin(),
            toFillTx.inputs.cend(),
            BigInt::ZERO,
            [](BigInt acc, BitcoinLikeBlockchainExplorerInput new_val) {
                return std::move(acc) + new_val.value.getValueOr(BigInt::ZERO);
            });
        toFillTx.fees = inputValue - outputValue;
    }
} // namespace

namespace ledger {
    namespace core {

        BitcoinLikeAccount::BitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index, const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer, const std::shared_ptr<BitcoinLikeAccountSynchronizer> &synchronizer, const std::shared_ptr<BitcoinLikeKeychain> &keychain)
            : AbstractAccount(wallet, index) {
            _explorer     = explorer;
            _synchronizer = synchronizer;
            _keychain     = keychain;
            _keychain->getAllObservableAddresses(0, 40);
            _picker             = std::make_shared<BitcoinLikeStrategyUtxoPicker>(getWallet()->getPool()->getThreadPoolExecutionContext(), getWallet()->getCurrency());
            _currentBlockHeight = 0;
        }

        void
        BitcoinLikeAccount::inflateOperation(Operation &out,
                                             const BitcoinLikeBlockchainExplorerTransaction &tx) {
            out.accountUid         = getAccountUid();
            out.block              = tx.block;
            out.bitcoinTransaction = Option<BitcoinLikeBlockchainExplorerTransaction>(tx);
            // Set accountUid of bitcoin outputs
            for (auto &output : out.bitcoinTransaction.getValue().outputs) {
                if (output.address.hasValue() && getKeychain()->contains(output.address.getValue())) {
                    output.accountUid = getAccountUid();
                }
                output.blockHeight = tx.block.map<uint64_t>([](const auto &b) {
                    return b.height;
                });
            }
            out.currencyName = getWallet()->getCurrency().name;
            out.walletType   = getWalletType();
            out.walletUid    = getWallet()->getWalletUid();
            out.date         = tx.receivedAt;
            if (out.block.nonEmpty())
                out.block.getValue().currencyName = getWallet()->getCurrency().name;
            out.bitcoinTransaction.getValue().block = out.block;
        }

        void BitcoinLikeAccount::interpretTransaction(const BitcoinLikeBlockchainExplorerTransaction &transaction,
                                                      std::vector<Operation> &out,
                                                      bool needExtendKeychain) {
            auto nodeIndex = std::const_pointer_cast<const BitcoinLikeKeychain>(_keychain)->getFullDerivationScheme().getPositionForLevel(DerivationSchemeLevel::NODE);
            std::list<std::pair<BitcoinLikeBlockchainExplorerInput *, DerivationPath>> accountInputs;
            std::list<std::pair<BitcoinLikeBlockchainExplorerOutput *, DerivationPath>> accountOutputs;
            uint64_t fees           = transaction.fees.getValue().toUint64();
            uint64_t sentAmount     = 0L;
            uint64_t receivedAmount = 0L;
            std::vector<std::string> senders;
            senders.reserve(transaction.inputs.size());
            std::vector<std::string> recipients;
            recipients.reserve(transaction.outputs.size());
            int result = FLAG_TRANSACTION_IGNORED;

            // Find inputs
            for (auto &input : transaction.inputs) {
                if (input.address.nonEmpty()) {
                    senders.push_back(input.address.getValue());
                }
                // Extend input with derivation paths

                if (input.address.nonEmpty() && input.value.nonEmpty()) {
                    auto path = _keychain->getAddressDerivationPath(input.address.getValue());
                    if (path.nonEmpty()) {
                        // This address is part of the account.
                        sentAmount += input.value.getValue().toUint64();
                        accountInputs.push_back(std::make_pair(const_cast<BitcoinLikeBlockchainExplorerInput *>(&input), DerivationPath(path.getValue())));
                        if (_keychain->markPathAsUsed(DerivationPath(path.getValue()), needExtendKeychain)) {
                            result = result | FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS;
                        } else {
                            result = result | FLAG_TRANSACTION_ON_USED_ADDRESS;
                        }
                    }
                }
            }

            // Find outputs
            auto hasSpentNothing = sentAmount == 0L;
            auto outputCount     = transaction.outputs.size();
            for (auto index = 0; index < outputCount; index++) {
                auto &output = transaction.outputs[index];
                if (output.address.nonEmpty()) {
                    auto path = _keychain->getAddressDerivationPath(output.address.getValue());
                    if (path.nonEmpty()) {
                        DerivationPath p(path.getValue());
                        accountOutputs.push_back(std::make_pair(const_cast<BitcoinLikeBlockchainExplorerOutput *>(&output), p));
                        if (p.getNonHardenedChildNum(nodeIndex) == 1) {
                            if (hasSpentNothing) {
                                receivedAmount += output.value.toUint64();
                            }
                            if ((recipients.size() == 0 && index + 1 >= outputCount) || hasSpentNothing) {
                                recipients.push_back(output.address.getValue());
                            }
                        } else {
                            receivedAmount += output.value.toUint64();
                            recipients.push_back(output.address.getValue());
                        }
                        if (_keychain->markPathAsUsed(DerivationPath(path.getValue()), needExtendKeychain)) {
                            result = result | FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS;
                        } else {
                            result = result | FLAG_TRANSACTION_ON_USED_ADDRESS;
                        }
                    } else {
                        recipients.push_back(output.address.getValue());
                    }
                }
            }
            std::stringstream snds;
            strings::join(senders, snds, ",");

            Operation operation;
            inflateOperation(operation, transaction);
            operation.senders    = std::move(senders);
            operation.recipients = std::move(recipients);
            operation.fees       = std::move(BigInt().assignI64(fees));
            operation.trust      = std::make_shared<TrustIndicator>();
            operation.date       = transaction.receivedAt;

            // Compute trust
            computeOperationTrust(operation, transaction);

            if (accountInputs.size() > 0) {
                // Create a send operation
                result = result | FLAG_TRANSACTION_CREATED_SENDING_OPERATION;

                for (auto &accountOutput : accountOutputs) {
                    if (accountOutput.second.getNonHardenedChildNum(nodeIndex) == 1)
                        sentAmount -= accountOutput.first->value.toInt64();
                }
                sentAmount -= fees;

                operation.amount.assignI64(sentAmount);
                operation.type = api::OperationType::SEND;
                operation.refreshUid();
                out.push_back(operation);
            }

            if (accountOutputs.size() > 0) {
                // Receive
                BigInt amount;
                auto flag                  = 0;
                bool filterChangeAddresses = true;

                if (accountInputs.size() == 0) {
                    filterChangeAddresses = false;
                }

                BigInt finalAmount;
                auto accountOutputCount = 0;
                for (auto &o : accountOutputs) {
                    if (filterChangeAddresses && o.second.getNonHardenedChildNum(nodeIndex) == 1)
                        continue;
                    finalAmount = finalAmount + o.first->value;
                    accountOutputCount += 1;
                }
                if (accountOutputCount > 0) {
                    operation.amount = finalAmount;
                    operation.type   = api::OperationType::RECEIVE;
                    operation.refreshUid();
                    out.push_back(operation);
                }
            }
        }

        Try<int> BitcoinLikeAccount::bulkInsert(const std::vector<Operation> &ops) {
            return Try<int>::from([&]() {
                soci::session sql(getWallet()->getDatabase()->getPool());
                soci::transaction tr(sql);
                BitcoinLikeOperationDatabaseHelper::bulkInsert(sql, ops);
                tr.commit();
                // Emit
                emitNewOperationsEvent(ops);
                return ops.size();
            });
        }

        void BitcoinLikeAccount::computeOperationTrust(Operation &operation,
                                                       const BitcoinLikeBlockchainExplorerTransaction &tx) {
            if (tx.block.nonEmpty()) {
                auto txBlockHeight = tx.block.getValue().height;
                if (_currentBlockHeight > txBlockHeight + 5) {
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

        std::vector<std::shared_ptr<api::Address>> BitcoinLikeAccount::fromBitcoinAddressesToAddresses(const std::vector<std::shared_ptr<BitcoinLikeAddress>> &addresses) {
            AbstractAccount::AddressList result;
            result.reserve(addresses.size());
            for (auto &addr : addresses) {
                result.push_back(std::dynamic_pointer_cast<api::Address>(addr));
            }
            return result;
        }

        std::shared_ptr<BitcoinLikeKeychain> BitcoinLikeAccount::getKeychain() const {
            return _keychain;
        }

        inline bool BitcoinLikeAccount::allowP2TR() const {
            return getWallet()->getConfig()->getBoolean(api::Configuration::ALLOW_P2TR).value_or(false);
        }

        bool BitcoinLikeAccount::isSynchronizing() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            return _currentSyncEventBus != nullptr;
        }

        std::shared_ptr<api::EventBus> BitcoinLikeAccount::synchronize() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);
            auto span = getTracer()->startSpan("BitcoinLikeAccount::synchronize");
            if (_currentSyncEventBus)
                return _currentSyncEventBus;
            auto eventPublisher  = std::make_shared<EventPublisher>(getContext());
            auto wasEmpty        = checkIfWalletIsEmpty();
            _currentSyncEventBus = eventPublisher->getEventBus();
            auto future          = _synchronizer->synchronize(std::static_pointer_cast<BitcoinLikeAccount>(shared_from_this()))->getFuture();
            auto self            = std::static_pointer_cast<BitcoinLikeAccount>(shared_from_this());

            // Update current block height (needed to compute trust level)
            _explorer->getCurrentBlock().onComplete(getContext(), [self](const TryPtr<BitcoinLikeBlockchainExplorer::Block> &block) mutable {
                if (block.isSuccess()) {
                    self->_currentBlockHeight = block.getValue()->height;
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    BulkInsertDatabaseHelper::updateBlock(sql, *block.getValue());
                }
            });

            auto startTime = DateUtils::now();
            eventPublisher->postSticky(std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()), 0);
            future.onComplete(getContext(), [eventPublisher, self, wasEmpty, startTime, span](auto const &result) {
                auto span2   = self->getTracer()->startSpan("BitcoinLikeAccount::synchronize.onComplete");
                auto isEmpty = self->checkIfWalletIsEmpty();
                api::EventCode code;
                auto payload  = std::make_shared<DynamicObject>();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - startTime).count();
                payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                if (result.isSuccess()) {
                    code = !isEmpty && wasEmpty ? api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT
                                                : api::EventCode::SYNCHRONIZATION_SUCCEED;
                    self->getWallet()->invalidateBalanceCache(self->getIndex());

                    auto const context = result.getValue();

                    payload->putInt(api::Account::EV_SYNC_LAST_BLOCK_HEIGHT, static_cast<int32_t>(context.lastBlockHeight));
                    payload->putInt(api::Account::EV_SYNC_NEW_OPERATIONS, static_cast<int32_t>(context.newOperations));

                    if (context.reorgBlockHeight) {
                        payload->putInt(api::Account::EV_SYNC_REORG_BLOCK_HEIGHT, static_cast<int32_t>(context.reorgBlockHeight.getValue()));
                    }
                } else {
                    code = api::EventCode::SYNCHRONIZATION_FAILED;
                    payload->putString(api::Account::EV_SYNC_ERROR_CODE, api::to_string(result.getFailure().getErrorCode()));
                    payload->putInt(api::Account::EV_SYNC_ERROR_CODE_INT, (int32_t)result.getFailure().getErrorCode());
                    payload->putString(api::Account::EV_SYNC_ERROR_MESSAGE, result.getFailure().getMessage());
                }
                eventPublisher->postSticky(std::make_shared<Event>(code, payload), 0);
                std::lock_guard<std::mutex> lock(self->_synchronizationLock);
                self->_currentSyncEventBus = nullptr;
                span2->close();
                span->close();
            });
            return eventPublisher->getEventBus();
        }

        void BitcoinLikeAccount::getUTXO(int32_t from, int32_t to, const std::shared_ptr<api::BitcoinLikeOutputListCallback> &callback) {
            getUTXO(from, to).callback(getMainExecutionContext(), callback);
        }

        Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>
        BitcoinLikeAccount::getUTXO(int32_t from, int32_t to) {
            auto self = getSelf();
            return async<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>([=]() -> std::vector<std::shared_ptr<api::BitcoinLikeOutput>> {
                auto keychain         = self->getKeychain();
                auto currency         = self->getWallet()->getCurrency();
                const auto dustAmount = BitcoinLikeTransactionApi::computeBasicTransactionDustAmount(currency, keychain->getKeychainEngine());
                soci::session sql(self->getWallet()->getDatabase()->getReadonlyPool());
                std::vector<BitcoinLikeBlockchainExplorerOutput> utxo;

                utils::cache_type<bool, std::string> cache{};
                std::function<bool(const std::string &)> filter = utils::cached(cache, utils::to_function([&keychain](const std::string addr) -> bool { // NOLINT(performance-unnecessary-value-param)
                                                                                    return keychain->contains(addr);
                                                                                }));

                BitcoinLikeUTXODatabaseHelper::queryUTXO(sql, self->getAccountUid(), from, to - from, dustAmount, utxo, filter);
                return functional::map<BitcoinLikeBlockchainExplorerOutput, std::shared_ptr<api::BitcoinLikeOutput>>(utxo, [&currency](const BitcoinLikeBlockchainExplorerOutput &output) -> std::shared_ptr<api::BitcoinLikeOutput> {
                    return std::make_shared<BitcoinLikeOutputApi>(output, currency);
                });
            });
        }

        void BitcoinLikeAccount::getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) {
            getUTXOCount().callback(getMainExecutionContext(), callback);
        }

        bool BitcoinLikeAccount::putBlock(soci::session &sql, const BitcoinLikeBlockchainExplorer::Block &block) {
            Block abstractBlock;
            abstractBlock.hash         = block.hash;
            abstractBlock.currencyName = getWallet()->getCurrency().name;
            abstractBlock.height       = block.height;
            abstractBlock.time         = block.time;
            if (BlockDatabaseHelper::putBlock(sql, abstractBlock)) {
                emitNewBlockEvent(abstractBlock);
                return true;
            }
            return false;
        }

        Future<int32_t> BitcoinLikeAccount::getUTXOCount() {
            auto self = getSelf();
            return async<int32_t>([=]() -> int32_t {
                auto keychain = self->getKeychain();
                soci::session sql(self->getWallet()->getDatabase()->getReadonlyPool());
                const auto dustAmount = BitcoinLikeTransactionApi::computeBasicTransactionDustAmount(self->getWallet()->getCurrency(), keychain->getKeychainEngine());
                utils::cache_type<bool, std::string> cache{};
                std::function<bool(const std::string &)> filter = utils::cached(cache, utils::to_function([&keychain](const std::string addr) -> bool { // NOLINT(performance-unnecessary-value-param)
                                                                                    return keychain->contains(addr);
                                                                                }));

                return static_cast<int32_t>(BitcoinLikeUTXODatabaseHelper::UTXOcount(sql, self->getAccountUid(), dustAmount, filter));
            });
        }

        std::shared_ptr<api::OperationQuery> BitcoinLikeAccount::queryOperations() {
            auto query = std::make_shared<OperationQuery>(
                api::QueryFilter::accountEq(getAccountUid()),
                getWallet()->getDatabase(),
                getWallet()->getPool()->getThreadPoolExecutionContext(),
                getWallet()->getMainExecutionContext());
            query->registerAccount(shared_from_this());
            return query;
        }

        bool BitcoinLikeAccount::checkIfWalletIsEmpty() {
            return _keychain->isEmpty();
        }

        Future<AbstractAccount::AddressList> BitcoinLikeAccount::getFreshPublicAddresses() {
            auto keychain = getKeychain();
            return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
                return fromBitcoinAddressesToAddresses(
                    keychain->getFreshAddresses(
                        BitcoinLikeKeychain::KeyPurpose::RECEIVE,
                        keychain->getObservableRangeSize()));
            });
        }

        Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> BitcoinLikeAccount::getUTXO() {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return getUTXOCount()
                .flatMap<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>(
                    getWallet()->getPool()->getThreadPoolExecutionContext(),
                    [=](const int32_t &count) -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                        return self->getUTXO(0, count);
                    });
        }

        void BitcoinLikeAccount::getMaxSpendable(api::BitcoinLikePickingStrategy strategy, optional<int32_t> maxUtxos, const std::shared_ptr<api::AmountCallback> &callback) {
            getMaxSpendable(strategy, maxUtxos).callback(getMainExecutionContext(), callback);
        }

        FuturePtr<ledger::core::Amount> BitcoinLikeAccount::getMaxSpendable(api::BitcoinLikePickingStrategy strategy, optional<int32_t> maxUtxos) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return FuturePtr<Amount>::async(getWallet()->getPool()->getThreadPoolExecutionContext(), [=]() -> std::shared_ptr<Amount> {
                const auto &uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                std::vector<BitcoinLikeBlockchainExplorerOutput> utxos;
                BigInt sum(0);
                auto keychain = self->getKeychain();
                const auto dustAmount = BitcoinLikeTransactionApi::computeBasicTransactionDustAmount(self->getWallet()->getCurrency(), keychain->getKeychainEngine());
                utils::cache_type<bool, std::string> cache{};
                std::function<bool(const std::string &)> filter = utils::cached(cache, utils::to_function([&keychain](std::string addr) -> bool { // NOLINT(performance-unnecessary-value-param)
                                                                                    return keychain->contains(addr);
                                                                                }));
                BitcoinLikeUTXODatabaseHelper::queryUTXO(sql, uid, 0, std::numeric_limits<int32_t>::max(), dustAmount, utxos, filter);
                switch (strategy) {
                case api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST:
                case api::BitcoinLikePickingStrategy::MERGE_OUTPUTS:
                case api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE: {
                    for (const auto &utxo : utxos) {
                        sum = sum + utxo.value;
                    }
                } break;
                case api::BitcoinLikePickingStrategy::HIGHEST_FIRST_LIMIT_UTXO:
                case api::BitcoinLikePickingStrategy::LIMIT_UTXO: {
                    std::sort(utxos.begin(), utxos.end(), [](auto &lhs, auto &rhs) {
                        return lhs.value.toInt64() > rhs.value.toInt64();
                    });
                    for (int i = 0; i < utxos.size() && i < maxUtxos.value_or(0); ++i) {
                        sum = sum + utxos[i].value;
                    }
                } break;
                default: {
                    throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "unknown strategy");
                }
                }

                Amount balance(self->getWallet()->getCurrency(), 0, sum);
                return std::make_shared<Amount>(balance);
            });
        }

        FuturePtr<ledger::core::Amount> BitcoinLikeAccount::getBalance() {
            auto cachedBalance = getWallet()->getBalanceFromCache(getIndex());
            if (cachedBalance.hasValue()) {
                return FuturePtr<Amount>::successful(std::make_shared<Amount>(cachedBalance.getValue()));
            }
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return FuturePtr<Amount>::async(getWallet()->getPool()->getThreadPoolExecutionContext(), [=]() -> std::shared_ptr<Amount> {
                const auto &uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getReadonlyPool());
                std::vector<BitcoinLikeBlockchainExplorerOutput> utxos;
                BigInt sum(0);
                auto keychain = self->getKeychain();
                const auto dustAmount = BitcoinLikeTransactionApi::computeBasicTransactionDustAmount(self->getWallet()->getCurrency(), keychain->getKeychainEngine());
                utils::cache_type<bool, std::string> cache{};
                std::function<bool(const std::string &)> filter = utils::cached(cache, utils::to_function([&keychain](const std::string addr) -> bool { // NOLINT(performance-unnecessary-value-param)
                                                                                    return keychain->contains(addr);
                                                                                }));
                BitcoinLikeUTXODatabaseHelper::queryUTXO(sql, uid, 0, std::numeric_limits<int32_t>::max(), dustAmount, utxos, filter);
                for (const auto &utxo : utxos) {
                    sum = sum + utxo.value;
                }
                Amount balance(self->getWallet()->getCurrency(), 0, sum);
                self->getWallet()->updateBalanceCache(self->getIndex(), balance);
                return std::make_shared<Amount>(balance);
            });
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        BitcoinLikeAccount::getBalanceHistory(const std::string &start,
                                              const std::string &end,
                                              api::TimePeriod precision) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return Future<std::vector<std::shared_ptr<api::Amount>>>::async(getWallet()->getPool()->getThreadPoolExecutionContext(), [=]() -> std::vector<std::shared_ptr<api::Amount>> {
                auto startDate = DateUtils::fromJSON(start);
                auto endDate   = DateUtils::fromJSON(end);
                if (startDate >= endDate) {
                    throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT,
                                         "Start date should be strictly lower than end date");
                }

                const auto &uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getReadonlyPool());
                std::vector<Operation> operations;

                auto keychain = self->getKeychain();

                utils::cache_type<bool, std::string> cache{};
                std::function<bool(const std::string &)> filter = utils::cached(cache, utils::to_function([&keychain](const std::string addr) -> bool { // NOLINT(performance-unnecessary-value-param)
                                                                                    return keychain->contains(addr);
                                                                                }));

                // Get operations related to an account
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
                        case api::OperationType::NONE:
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

        std::shared_ptr<BitcoinLikeAccount> BitcoinLikeAccount::getSelf() {
            return std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
        }

        Future<std::string> BitcoinLikeAccount::broadcastTransaction(const std::vector<uint8_t> &transaction) {
            return _explorer->pushTransaction(transaction).map<std::string>(getMainExecutionContext(), [](const String &hash) -> std::string {
                return hash.str();
            });
        }

        // This method is used to get a block height only used to know which
        // update/fork (activated at certain block height) is used.
        // This explains why we take LLONG_MAX is we have no block in DB for
        // certain currency
        // WARNING: please don't use this method if you need an accurate value
        // of last block
        static uint64_t getLastBlockFromDB(soci::session &sql, const std::string &currencyName) {
            // Get last block from DB
            auto lastBlock = BlockDatabaseHelper::getLastBlock(sql, currencyName);
            // If we can not retrieve a last block for currency,
            // we set the returned block height to LLONG_MAX in order to
            // activate/apply last BIP/update/fork for this currency.
            return lastBlock.hasValue() ? static_cast<uint64_t>(lastBlock->height) : LLONG_MAX;
        }

        void BitcoinLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                         const std::shared_ptr<api::StringCallback> &callback) {
            broadcastRawTransaction(transaction, callback, "");
        }

        void BitcoinLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &transaction,
                                                         const std::shared_ptr<api::StringCallback> &callback,
                                                         const std::string &correlationId) {
            auto self = getSelf();
            _explorer->pushTransaction(transaction, correlationId).map<std::string>(getContext(), [self, transaction, correlationId](const String &seq) -> std::string {
                                                                      // Store newly broadcasted tx in db
                                                                      // First parse it
                                                                      auto txHash           = seq.str();
                                                                      auto optimisticUpdate = Try<Unit>::from([&]() -> Unit {
                                                                          // Get last block from DB or cache
                                                                          self->logger()->debug("{} getting last block from DB or cache", CORRELATIONID_PREFIX(correlationId));
                                                                          uint64_t lastBlockHeight = 0;
                                                                          soci::session sql(self->getWallet()->getDatabase()->getReadonlyPool());
                                                                          auto cachedBlock = self->getWallet()->getPool()->getBlockFromCache(self->getWallet()->getCurrency().name);
                                                                          if (cachedBlock.hasValue()) {
                                                                              lastBlockHeight = cachedBlock.getValue().height;
                                                                          } else {
                                                                              lastBlockHeight = getLastBlockFromDB(sql, self->getWallet()->getCurrency().name);
                                                                          }

                                                                          self->logger()->debug("{} reparsing broadcasted transaction", CORRELATIONID_PREFIX(correlationId));
                                                                          auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(self->getWallet()->getCurrency(), transaction, lastBlockHeight);

                                                                          self->logger()->debug("{} creating the optimistic update transaction", CORRELATIONID_PREFIX(correlationId));
                                                                          // Get a BitcoinLikeBlockchainExplorerTransaction from a BitcoinLikeTransaction
                                                                          auto txExplorer = initOptimisticUpdate(txHash, tx);

                                                                          // Inputs
                                                                          inflateOptimisticUpdateInputs(txExplorer, tx, self->getAccountUid(), sql);

                                                                          // Outputs
                                                                          inflateOptimisticUpdateOutputs(txExplorer, tx);

                                                                          // Fees
                                                                          inflateOptimisticUpdateFees(txExplorer);

                                                                          // Store in DB
                                                                          self->logger()->debug("{} storing the optimistic update transaction", CORRELATIONID_PREFIX(correlationId));
                                                                          std::vector<Operation> operations;
                                                                          self->interpretTransaction(txExplorer, operations);
                                                                          self->logger()->debug("{} inserting the associated operations", CORRELATIONID_PREFIX(correlationId));
                                                                          self->bulkInsert(operations);
                                                                          self->emitEventsNow();
                                                                          return unit;
                                                                      });

                                                                      // Failing optimistic update should not throw an exception
                                                                      // because the tx was successfully broadcasted to the network,
                                                                      // and the update will occur at next synchro ...
                                                                      // But still let's log that !
                                                                      if (optimisticUpdate.isFailure()) {
                                                                          self->logger()->warn("{} Optimistic update failed for broadcasted transaction [hash: {}] with errcode {}: {}",
                                                                                               CORRELATIONID_PREFIX(correlationId), txHash, api::to_string(optimisticUpdate.getFailure().getErrorCode()), optimisticUpdate.getFailure().getMessage());
                                                                      } else {
                                                                          self->getWallet()->invalidateBalanceCache(self->getIndex());
                                                                      }

                                                                      return txHash;
                                                                  })
                .callback(getMainExecutionContext(), callback);
        }

        void BitcoinLikeAccount::broadcastTransaction(const std::shared_ptr<api::BitcoinLikeTransaction> &transaction,
                                                      const std::shared_ptr<api::StringCallback> &callback) {
            logger()->info("{} received raw transaction to broadcast", CORRELATIONID_PREFIX(transaction->getCorrelationId()));
            broadcastRawTransaction(transaction->serialize(), callback, transaction->getCorrelationId());
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder> BitcoinLikeAccount::buildTransaction(bool partial) {
            auto self    = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            auto getUTXO = [=]() -> Future<std::vector<BitcoinLikeUtxo>> {
                return Future<std::vector<BitcoinLikeUtxo>>::async(getContext(), [=]() {
                    auto keychain = self->getKeychain();
                    soci::session session(self->getWallet()->getDatabase()->getPool());

                    const auto dustAmount = BitcoinLikeTransactionApi::computeBasicTransactionDustAmount(self->getWallet()->getCurrency(), keychain->getKeychainEngine());
                    return BitcoinLikeUTXODatabaseHelper::queryAllUtxos(session, self->getAccountUid(), self->getWallet()->getCurrency(), dustAmount);
                });
            };
            auto getTransaction = [self](const std::string &hash) -> FuturePtr<BitcoinLikeBlockchainExplorerTransaction> {
                return self->getTransaction(hash);
            };

            uint64_t lastBlockHeight = 0;
            auto cachedBlock         = self->getWallet()->getPool()->getBlockFromCache(self->getWallet()->getCurrency().name);
            if (cachedBlock.hasValue()) {
                lastBlockHeight = cachedBlock.getValue().height;
            } else {
                soci::session sql(self->getWallet()->getDatabase()->getReadonlyPool());
                lastBlockHeight = getLastBlockFromDB(sql, self->getWallet()->getCurrency().name);
            }

            return std::make_shared<BitcoinLikeTransactionBuilder>(
                getMainExecutionContext(),
                getWallet()->getCurrency(),
                logger(),
                _picker->getBuildFunction(getUTXO,
                                          getTransaction,
                                          _explorer,
                                          _keychain,
                                          lastBlockHeight,
                                          logger(),
                                          partial),
                allowP2TR());
        }

        const std::shared_ptr<BitcoinLikeBlockchainExplorer> &BitcoinLikeAccount::getExplorer() const {
            return _explorer;
        }

        FuturePtr<ledger::core::BitcoinLikeBlockchainExplorerTransaction> BitcoinLikeAccount::getTransaction(const std::string &hash) {
            auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            return async<std::shared_ptr<BitcoinLikeBlockchainExplorerTransaction>>([=]() -> std::shared_ptr<BitcoinLikeBlockchainExplorerTransaction> {
                auto tx = std::make_shared<BitcoinLikeBlockchainExplorerTransaction>();
                soci::session sql(self->getWallet()->getDatabase()->getReadonlyPool());
                if (!BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, hash, self->getAccountUid(), *tx)) {
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

        Future<api::ErrorCode> BitcoinLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            auto log = logger();

            log->debug(" Start erasing data of account : {}", getAccountUid());

            std::lock_guard<std::mutex> lock(_synchronizationLock);
            _currentSyncEventBus = nullptr;

            soci::session sql(getWallet()->getDatabase()->getPool());

            // Update account's internal preferences (for synchronization)
            //  Clear synchronizer state
            eraseSynchronizerDataSince(sql, date);

            auto accountUid = getAccountUid();
            BitcoinLikeTransactionDatabaseHelper::eraseDataSince(sql, accountUid, date);

            return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
        }

        void BitcoinLikeAccount::getFees(const std::shared_ptr<api::BigIntListCallback> &callback) {
            return _explorer->getFees().callback(getMainExecutionContext(), callback);
        }

        Future<AbstractAccount::AddressList> BitcoinLikeAccount::getAddresses(int64_t from, int64_t to) {
            auto keychain = getKeychain();
            return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
                return fromBitcoinAddressesToAddresses(keychain->getAllObservableAddresses(from, to));
            });
        }

        void BitcoinLikeAccount::getAddresses(int64_t from, int64_t to, const std::shared_ptr<api::AddressListCallback> &callback) {
            return getAddresses(from, to).callback(getMainExecutionContext(), callback);
        }

        AbstractAccount::AddressList BitcoinLikeAccount::getAllAddresses() {
            auto keychain = getKeychain();
            return fromBitcoinAddressesToAddresses(keychain->getAllAddresses());
        }

        std::shared_ptr<api::Keychain> BitcoinLikeAccount::getAccountKeychain() {
            return _keychain;
        }
    } // namespace core
} // namespace ledger
