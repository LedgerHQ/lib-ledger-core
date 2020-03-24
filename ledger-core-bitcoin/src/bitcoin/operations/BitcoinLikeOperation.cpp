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
            std::shared_ptr<AbstractAccount> account,
            BitcoinLikeBlockchainExplorerTransaction const& tx) :
                Operation(account),
                _explorerTx(tx),
                _tx(std::make_shared<BitcoinLikeTransaction>(tx, account->getWallet()->getCurrency()))
        {}

        BitcoinLikeOperation::BitcoinLikeOperation(const std::shared_ptr<BitcoinLikeOperation> &operation) :
                Operation(operation->getAccount()),
                _explorerTx(operation->_explorerTx),
                _tx(std::make_shared<BitcoinLikeTransaction>(operation->_explorerTx, operation->getCurrency()))
        {}

        std::shared_ptr<api::BitcoinLikeTransaction> BitcoinLikeOperation::getTransaction() {
            return _tx;
        }

        BitcoinLikeBlockchainExplorerTransaction& BitcoinLikeOperation::getExplorerTransaction() {
            return _explorerTx;
        }

        BitcoinLikeBlockchainExplorerTransaction const& BitcoinLikeOperation::getExplorerTransaction() const {
            return _explorerTx;
        }

        void BitcoinLikeOperation::setExplorerTransaction(BitcoinLikeBlockchainExplorerTransaction const& tx) {
            _explorerTx = tx;
            _tx = std::make_shared<BitcoinLikeTransaction>(_explorerTx, this->getCurrency());
        }

        void BitcoinLikeOperation::refreshUid(std::string const&) {
            uid = OperationDatabaseHelper::createUid(accountUid, _tx->getHash(), getOperationType());
        }

        bool BitcoinLikeOperation::isComplete() {
            return static_cast<bool>(_tx);
        }

        std::shared_ptr<api::BitcoinLikeOperation> fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) {
          return std::dynamic_pointer_cast<api::BitcoinLikeOperation>(coreOperation);
        }
    }
}
