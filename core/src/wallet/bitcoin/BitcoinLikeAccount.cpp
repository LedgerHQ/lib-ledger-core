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
#include <utility>

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

        int BitcoinLikeAccount::putTransaction(soci::session &sql,
                                               const BitcoinLikeBlockchainExplorer::Transaction &transaction) {

            auto nodeIndex = std::const_pointer_cast<const BitcoinLikeKeychain>(_keychain)->getDerivationScheme().getPositionForLevel(DerivationSchemeLevel::NODE);
            std::list<std::pair<BitcoinLikeBlockchainExplorer::Input *, DerivationPath>> accountInputs;
            std::list<std::pair<BitcoinLikeBlockchainExplorer::Output *, DerivationPath>> accountOutputs;
            BigInt fees;
            BigInt sentAmount;
            BigInt receivedAmount;
            std::list<std::string> senders;
            std::list<std::string> recipients;

            int result = 0x00;

            // Find inputs
            for (auto& input : transaction.inputs) {

                if (input.address.nonEmpty())
                    senders.push_back(input.address.getValue());

                // Extend input with derivation paths

                if (input.address.nonEmpty() && input.value.nonEmpty()) {
                    auto path = _keychain->getAddressDerivationPath(input.address.getValue());
                    if (path.nonEmpty()) {
                        // This address is part of the account.
                        accountInputs.push_back(std::move(std::make_pair(const_cast<BitcoinLikeBlockchainExplorer::Input *>(&input), path.getValue())));
                        sentAmount = sentAmount + input.value.getValue();
                        if (_keychain->markPathAsUsed(path.getValue())) {
                            result = result | FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS;
                        } else {
                            result = result | FLAG_TRANSACTION_ON_USED_ADDRESS;
                        }
                    }
                }
                if (input.value.nonEmpty()) {
                    fees = fees + input.value.getValue();
                }
            }

            // Find outputs
            for (auto& output : transaction.outputs) {
                if (output.address.nonEmpty()) {
                    auto path = _keychain->getAddressDerivationPath(output.address.getValue());
                    if (path.nonEmpty()) {
                        accountOutputs.push_back(std::move(std::make_pair(const_cast<BitcoinLikeBlockchainExplorer::Output *>(&output), path.getValue())));
                        if (path.getValue()[nodeIndex] == 1) {
                            sentAmount = sentAmount - output.value;
                        } else {
                            recipients.push_back(output.address.getValue());
                        }
                    } else {
                        recipients.push_back(output.address.getValue());
                    }
                }
                fees = fees - output.value;
            }

            if (accountInputs.size() > 0) {
                // Create a send operation
                result = result | FLAG_TRANSACTION_CREATED_SENDING_OPERATION;
                BigInt amount;

                for (auto& accountInput : accountInputs) {

                }
            }

            if (accountOutputs.size() > 0) {
                // Check if this can be a reception

                /* Cases:
                 *  1 - If transaction is not a sending and we own one output, we consider the ouput as recipient (no matter if it's on a change address)
                 *  2 - If a transaction contains account outputs and no account inputs, we consider all our outputs in the recipients (no matter if there is change addresses)
                 *  3 - If a transaction contains both account inputs and outputs, we only consider
                 */

                BigInt amount;
                auto flag = 0;
                bool filterChangeAddresses = true;

                if (accountInputs.size() == 0) {
                    filterChangeAddresses = false;
                }

            }

            // Put the operation


            return result;
        }

        std::shared_ptr<const BitcoinLikeKeychain> BitcoinLikeAccount::getKeychain() const {
            return std::const_pointer_cast<const BitcoinLikeKeychain>(_keychain);
        }

        // REVIEW

        void BitcoinLikeAccount::getOperations(int32_t from, int32_t to, bool descending, bool complete,
                                               const std::shared_ptr<api::OperationListCallback> &callback) {

        }

        void BitcoinLikeAccount::getOperationsCount(const std::shared_ptr<api::I64Callback> &callback) {

        }

        void BitcoinLikeAccount::getOperation(const std::string &uid,
                                              const std::shared_ptr<api::OperationCallback> &callback) {

        }

        void BitcoinLikeAccount::getBalance(const std::shared_ptr<api::AmountCallback> &callback) {

        }

        bool BitcoinLikeAccount::isSynchronizing() {
            return false;
        }

        std::shared_ptr<api::EventBus> BitcoinLikeAccount::synchronize() {
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
    }
}