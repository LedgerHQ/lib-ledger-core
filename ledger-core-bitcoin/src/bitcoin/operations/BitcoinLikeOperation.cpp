/*
 *
 * BitcoinLikeOperation
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/07/2017.
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

#include <core/operation/OperationDatabaseHelper.hpp>

#include <bitcoin/operations/BitcoinLikeOperation.hpp>
#include <bitcoin/transactions/BitcoinLikeTransaction.hpp>

namespace ledger {
    namespace core {

        BitcoinLikeOperation::BitcoinLikeOperation(
            const std::shared_ptr<const AbstractWallet>& wallet,
            BitcoinLikeBlockchainExplorerTransaction const& tx)
                : _tx(std::make_shared<BitcoinLikeTransaction>(wallet->getCurrency())),
                  _explorerTx(tx)
        {}

        BitcoinLikeOperation::BitcoinLikeOperation(
            const std::shared_ptr<BitcoinLikeOperation> &operation,
            BitcoinLikeBlockchainExplorerTransaction const& tx) 
                : _tx(std::make_shared<BitcoinLikeTransaction>(operation, operation->getCurrency())),
                  _explorerTx(tx)
        {}
        
        std::shared_ptr<api::BitcoinLikeTransaction> BitcoinLikeOperation::getTransaction() {
            return _tx;
        }

        BitcoinLikeBlockchainExplorerTransaction& BitcoinLikeOperation::getExplorerTransaction() {
            return _explorerTx;
        }

        void BitcoinLikeOperation::setExplorerTransaction(BitcoinLikeBlockchainExplorerTransaction const& tx) {
            _explorerTx = tx;
        }

        void BitcoinLikeOperation::refreshUid(std::string const&) {
            uid = OperationDatabaseHelper::createUid(accountUid, _tx->getHash(), type);
        }

        bool BitcoinLikeOperation::isComplete() {
            return static_cast<bool>(_tx);
        }
    }
}