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

namespace ledger {
    namespace core {

        int BitcoinLikeAccount::putTransaction(soci::session &sql,
                                               const BitcoinLikeBlockchainExplorer::Transaction &transaction) {

//            std::list<BitcoinLikeBlockchainExplorer::Input&> accountInputs;
//            std::list<BitcoinLikeBlockchainExplorer::Output&> accountOutputs;
//            BigInt fees;
//
//            // Find inputs
//            for (auto& input : transaction.inputs) {
//                if (input.address.nonEmpty()) {
//                    //if (_keychain->)
//                }
//                if (input.value.nonEmpty()) {
//                    fees = fees + input.value.getValue();
//                }
//            }
//
//            // Find outputs
//            for (auto& output : transaction.outputs) {
//                if (output.address.nonEmpty()) {
//
//                }
//                fees = fees - output.value;
//            }
//
//            // Put the operation
            return 0;
        }

        std::shared_ptr<const BitcoinLikeKeychain> BitcoinLikeAccount::getKeychain() const {
            return std::const_pointer_cast<const BitcoinLikeKeychain>(_keychain);
        }

    }
}