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
                    fmt::print("HAS {}\n", input.address.getValue());
                    senders.push_back(input.address.getValue());
                }
                // Extend input with derivation paths

                if (input.address.nonEmpty() && input.value.nonEmpty()) {
                    auto path = _keychain->getAddressDerivationPath(input.address.getValue());
                    if (path.nonEmpty()) {
                        // This address is part of the account.
                        sentAmount += input.value.getValue().toUint64();
                        accountInputs.push_back(std::move(std::make_pair(const_cast<BitcoinLikeBlockchainExplorer::Input *>(&input), path.getValue())));
                        if (_keychain->markPathAsUsed(path.getValue())) {
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
                        accountOutputs.push_back(std::move(std::make_pair(const_cast<BitcoinLikeBlockchainExplorer::Output *>(&output), path.getValue())));
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
                OperationDatabaseHelper::putOperation(sql, operation);
            }

            if (accountOutputs.size() > 0) {

                BigInt amount;
                auto flag = 0;
                bool filterChangeAddresses = true;

                if (accountInputs.size() == 0) {
                    filterChangeAddresses = false;
                }

                operation.amount.assignI64(receivedAmount);
                operation.type = api::OperationType::RECEIVE;
                operation.refreshUid();
                OperationDatabaseHelper::putOperation(sql, operation);
            }

            return result;
        }

        void
        BitcoinLikeAccount::computeOperationTrust(Operation &operation, const std::shared_ptr<const AbstractWallet> &wallet,
                                                  const BitcoinLikeBlockchainExplorer::Transaction &tx) {
            operation.trust->setTrustLevel(api::TrustLevel::TRUSTED);
        }

        std::shared_ptr<const BitcoinLikeKeychain> BitcoinLikeAccount::getKeychain() const {
            return std::const_pointer_cast<const BitcoinLikeKeychain>(_keychain);
        }

        // REVIEW

        void BitcoinLikeAccount::getBalance(const std::shared_ptr<api::AmountCallback> &callback) {

        }

        bool BitcoinLikeAccount::isSynchronizing() {
            return false;
        }

        std::shared_ptr<api::EventBus> BitcoinLikeAccount::synchronize() {
            _synchronizer->synchronize(std::static_pointer_cast<BitcoinLikeAccount>(shared_from_this()));
            return nullptr;
        }

        void BitcoinLikeAccount::computeFees(const std::shared_ptr<api::Amount> &amount, int32_t priority,
                                             const std::vector<std::string> &recipients,
                                             const std::vector<std::vector<uint8_t>> &data,
                                             const std::shared_ptr<api::AmountCallback> &callback) {

        }

        void BitcoinLikeAccount::getUTXO(int32_t from, int32_t to,
                                         const std::shared_ptr<api::BitcoinLikeOutputListCallback> &callback) {

        }

        void BitcoinLikeAccount::getUTXOCount(const std::shared_ptr<api::I32Callback> &callback) {

        }

        bool BitcoinLikeAccount::putBlock(soci::session &sql, const BitcoinLikeBlockchainExplorer::Block block) {
            return false;
        }

        FuturePtr<api::Amount> BitcoinLikeAccount::getBalance() {
            PromisePtr<api::Amount> p;
            return p.getFuture();
        }

        Future<int32_t> BitcoinLikeAccount::getUTXOCount() {
            Promise<int32_t> p;
            return p.getFuture();
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

    }
}