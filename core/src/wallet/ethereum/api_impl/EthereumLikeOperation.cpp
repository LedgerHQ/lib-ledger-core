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


#include "EthereumLikeOperation.h"
#include <wallet/ethereum/api_impl/EthereumLikeTransactionApi.h>
#include <wallet/ethereum/api_impl/InternalTransaction.h>
#include <session.h>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/ethereum/api_impl/InternalTransaction.h>
#include <wallet/ethereum/explorers/EthereumLikeBlockchainExplorer.h>
#include <api/OperationType.hpp>

namespace ledger {
    namespace core {

        EthereumLikeOperation::EthereumLikeOperation(const std::shared_ptr<OperationApi>& baseOp)
        : _initialized(false)
        {
            _transaction = std::make_shared<EthereumLikeTransactionApi>(baseOp);
            _backend = baseOp;
        }

        std::shared_ptr<api::EthereumLikeTransaction> EthereumLikeOperation::getTransaction() {
            return _transaction;
        }

        std::vector<std::shared_ptr<api::InternalTransaction>>
        EthereumLikeOperation::getInternalTransactions() {
            if (_initialized) {
                soci::session sql(_backend->getAccount()->getWallet()->getDatabase()->getPool());
                auto uid = _backend->getUid();
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
            }
            return _internalTxs;
        }
    }
}