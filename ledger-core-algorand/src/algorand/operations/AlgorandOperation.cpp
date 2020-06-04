/*
 * AlgorandOperation
 *
 * Created by Rémi Barjon on 12/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include <algorand/operations/AlgorandOperation.hpp>

#include "../transactions/api_impl/AlgorandTransactionImpl.hpp"

#include <core/operation/OperationDatabaseHelper.hpp>

#include <fmt/format.h>

namespace ledger {
namespace core {
namespace algorand {

    Operation::Operation(const std::shared_ptr<AbstractAccount>& account,
                         const model::Transaction& txn)
        : ::ledger::core::Operation(account)
        , algorandType()
    {
        setTransaction(txn);
    }

    api::AlgorandOperationType Operation::getAlgorandOperationType() const
    {
        return algorandType;
    }

    std::shared_ptr<api::AlgorandTransaction> Operation::getTransaction() const
    {
        return transaction;
    }

    void Operation::refreshUid(const std::string&)
    {
        const auto& txn = getTransactionData();
        const auto id =
            fmt::format("{}+{}", *txn.header.id, api::to_string(algorandType));
        uid = OperationDatabaseHelper::createUid(accountUid, id, getOperationType());
    }

    const model::Transaction& Operation::getTransactionData() const
    {
        return transaction->getTransactionData();
    }

    void Operation::setTransaction(const model::Transaction& txn)
    {
        transaction = std::make_shared<AlgorandTransactionImpl>(txn);
        setAlgorandOperationType();
    }

    void Operation::setAlgorandOperationType()
    {
        const auto& txn = transaction->getTransactionData();
        if (txn.header.type == model::constants::pay) {
            const auto& details = boost::get<model::PaymentTxnFields>(txn.details);
            if (details.closeAddr) {
                algorandType = api::AlgorandOperationType::ACCOUNT_CLOSE;
            } else {
                algorandType = api::AlgorandOperationType::PAYMENT;
            }
        } else if (txn.header.type == model::constants::keyreg) {
            const auto& details = boost::get<model::KeyRegTxnFields>(txn.details);
            if (details.nonParticipation && !(*details.nonParticipation)) {
                algorandType = api::AlgorandOperationType::ACCOUNT_REGISTER_ONLINE;
            } else {
                algorandType = api::AlgorandOperationType::ACCOUNT_REGISTER_OFFLINE;
            }
        } else if (txn.header.type == model::constants::acfg) {
            const auto& details = boost::get<model::AssetConfigTxnFields>(txn.details);
            if (!details.assetId) {
                algorandType = api::AlgorandOperationType::ASSET_CREATE;
            } else if (!details.assetParams) {
                algorandType = api::AlgorandOperationType::ASSET_DESTROY;
            } else {
                algorandType = api::AlgorandOperationType::ASSET_RECONFIGURE;
            }
        } else if (txn.header.type == model::constants::axfer) {
            const auto& details = boost::get<model::AssetTransferTxnFields>(txn.details);
            if (details.assetSender) {
                algorandType = api::AlgorandOperationType::ASSET_REVOKE;
            } else if (details.assetCloseTo) {
                algorandType = api::AlgorandOperationType::ASSET_OPT_OUT;
            } else if (!details.assetAmount) {
                algorandType = api::AlgorandOperationType::ASSET_OPT_IN;
            } else {
                algorandType = api::AlgorandOperationType::ASSET_TRANSFER;
            }
        } else if (txn.header.type == model::constants::afreeze) {
            algorandType = api::AlgorandOperationType::ASSET_FREEZE;
        } else {
            algorandType = api::AlgorandOperationType::UNSUPPORTED;
        }
    }

} // namespace algorand
} // namespace core
} // namespace ledger

