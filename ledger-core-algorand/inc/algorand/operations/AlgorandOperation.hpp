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

#pragma once

#include <algorand/model/transactions/AlgorandTransaction.hpp>

#include <algorand/api/AlgorandOperation.hpp>
#include <algorand/api/AlgorandOperationType.hpp>
#include <algorand/api/AlgorandTransaction.hpp>

#include <core/operation/Operation.hpp>
#include <core/wallet/AbstractAccount.hpp>

namespace ledger {
namespace core {
namespace algorand {

    class Account;
    class AlgorandTransactionImpl;

    class Operation : public ::ledger::core::api::AlgorandOperation
                    , public ::ledger::core::Operation
    {
    public:
        explicit Operation() = default;
        explicit Operation(const std::shared_ptr<AbstractAccount>& account);
        explicit Operation(const std::shared_ptr<AbstractAccount>& account,
                           const model::Transaction& txn);

        // From ledger::core::api::AlgorandOperation
        std::shared_ptr<api::AlgorandTransaction> getTransaction() const override;
        api::AlgorandOperationType getAlgorandOperationType() const override;
        std::string getRewards() const override;
        std::string getAssetAmount() const override;

        // From ledger::core::api::Operation
        void refreshUid(const std::string& additional = "") override;

        const model::Transaction& getTransactionData() const;
        void setTransaction(const model::Transaction& txn);
        void setAlgorandOperationType(api::AlgorandOperationType t);

    private:
        void inflate();
        void inflateFromAccount();
        void inflateFromTransaction();
        void inflateDate();
        void inflateBlock();
        void inflateAmountAndFees();
        void inflateSenders();
        void inflateRecipients();
        void inflateType();
        void inflateAlgorandOperationType();

        const Account& getAlgorandAccount() const;

    private:
        std::shared_ptr<AlgorandTransactionImpl> transaction;
        api::AlgorandOperationType algorandType;
        uint64_t rewards;
        uint64_t assetAmount;
    };

} // namespace algorand
} // namespace core
} // namespace ledger

