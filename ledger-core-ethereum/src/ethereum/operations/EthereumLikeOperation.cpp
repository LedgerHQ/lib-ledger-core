/*
 *
 * EthereumLikeOperation
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include <session.h>

#include <core/api/OperationType.hpp>
#include <core/api/enum_from_string.hpp>
#include <core/wallet/AbstractAccount.hpp>

#include <ethereum/EthereumLikeTransaction.hpp>
#include <ethereum/InternalTransaction.hpp>
#include <ethereum/explorers/EthereumLikeBlockchainExplorer.hpp>
#include <ethereum/operations/EthereumLikeOperation.hpp>
#include <ethereum/operations/EthereumLikeOperationDatabaseHelper.hpp>

namespace ledger {
    namespace core {
        EthereumLikeOperation::EthereumLikeOperation(
            const std::shared_ptr<const AbstractWallet>& wallet,
            EthereumLikeBlockchainExplorerTransaction const& tx
        ): _internalTxsRetrieved(false) {
            setExplorerTransaction(tx);
            _tx = std::make_shared<EthereumLikeTransaction>(tx, wallet->getCurrency());
        }

        EthereumLikeOperation::EthereumLikeOperation(
            const std::shared_ptr<Operation>& operation,
            EthereumLikeBlockchainExplorerTransaction const& tx
        ): _internalTxsRetrieved(false) {
            setExplorerTransaction(tx);
            _tx = std::make_shared<EthereumLikeTransaction>(tx, operation->getCurrency());
        }

        std::shared_ptr<api::EthereumLikeTransaction> EthereumLikeOperation::getTransaction() const {
            return _tx;
        }

        const EthereumLikeBlockchainExplorerTransaction& EthereumLikeOperation::getExplorerTransaction() const
        {
            return _explorerTx;
        }

        EthereumLikeBlockchainExplorerTransaction& EthereumLikeOperation::getExplorerTransaction()
        {
            return _explorerTx;
        }

        void EthereumLikeOperation::setExplorerTransaction(EthereumLikeBlockchainExplorerTransaction const& tx)
        {
            _explorerTx = tx;
        }


        std::vector<std::shared_ptr<api::InternalTransaction>>
        EthereumLikeOperation::getInternalTransactions() {
            if (!_internalTxsRetrieved) {
                soci::session sql(_account->getWallet()->getDatabase()->getPool());
                soci::rowset<soci::row> internalTxRows = (sql.prepare << "SELECT type, value, sender, "
                        "receiver, gas_limit, gas_used, input_data "
                        "FROM internal_operations WHERE ethereum_operation_uid = :uid ",
                        soci::use(uid));
                for (auto &row : internalTxRows) {
                    InternalTx internalTx;
                    internalTx.type = api::from_string<api::OperationType>(row.get<std::string>(0));
                    internalTx.value = BigInt::fromHex(row.get<std::string>(1));
                    internalTx.from = row.get<std::string>(2);
                    internalTx.to = row.get<std::string>(3);
                    internalTx.gasLimit = BigInt::fromHex(row.get<std::string>(4));
                    internalTx.gasUsed = BigInt::fromHex(row.get<std::string>(5));
                    internalTx.inputData = hex::toByteArray(row.get<std::string>(6));
                    _internalTxs.push_back(std::make_shared<InternalTransaction>(internalTx));
                }
                _internalTxsRetrieved = true;
            }
            return _internalTxs;
        }

        void EthereumLikeOperation::refreshUid(std::string const&) {
            uid = EthereumLikeOperationDatabaseHelper::createUid(accountUid, _tx->getHash(), type);
        }

        bool EthereumLikeOperation::isComplete() {
            // uses bool operator here from shared_ptr
            return static_cast<bool>(_tx);
        }

        std::shared_ptr<api::EthereumLikeOperation> fromCoreOperation(const std::shared_ptr<api::Operation> & coreOperation) {
          return std::dynamic_pointer_cast<api::EthereumLikeOperation>(coreOperation);
        }
    }
}
