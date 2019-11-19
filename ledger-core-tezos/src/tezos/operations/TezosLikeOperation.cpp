/*
 *
 * TezosLikeOperation
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


#include <tezos/transactions/TezosLikeTransaction.hpp>
#include <tezos/operations/TezosLikeOperation.hpp>

#include <core/operation/OperationDatabaseHelper.hpp>

namespace ledger {
    namespace core {
        TezosLikeOperation::TezosLikeOperation(
                const std::shared_ptr<const AbstractWallet> & wallet, 
                TezosLikeBlockchainExplorerTransaction const& tx) {
            setExplorerTransaction(tx);
            
            _tx = std::make_shared<TezosLikeTransaction>(tx, wallet->getCurrency());
        }

        std::shared_ptr<api::TezosLikeTransaction> TezosLikeOperation::getTransaction() const {
            return _tx;
        }

        const TezosLikeBlockchainExplorerTransaction& TezosLikeOperation::getExplorerTransaction() const
        {
            return _explorerTx;
        }

        TezosLikeBlockchainExplorerTransaction& TezosLikeOperation::getExplorerTransaction()
        {
            return _explorerTx;
        }
        
        void TezosLikeOperation::setExplorerTransaction(TezosLikeBlockchainExplorerTransaction const& tx)
        {
            _explorerTx = tx;
        }

        void TezosLikeOperation::refreshUid() {
          uid = OperationDatabaseHelper::createUid(
              accountUid,
              _tx->getHash(),
              getOperationType()
          );
        }

        bool TezosLikeOperation::isComplete() {
            return true;
        }
    }
}