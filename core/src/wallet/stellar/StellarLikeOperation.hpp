/*
 *
 * StellarLikeOperation.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2019.
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

#ifndef LEDGER_CORE_STELLARLIKEOPERATION_HPP
#define LEDGER_CORE_STELLARLIKEOPERATION_HPP

#include <api/StellarLikeOperation.hpp>
#include <wallet/common/api_impl/OperationApi.h>
#include <api/StellarLikeOperationRecord.hpp>
#include <wallet/stellar/transaction_builders/StellarLikeTransaction.hpp>

namespace ledger {
    namespace core {
        class StellarLikeOperation : public api::StellarLikeOperation {
        public:
            explicit StellarLikeOperation(const std::shared_ptr<OperationApi>& api);

            api::StellarLikeOperationRecord getRecord() override;

            std::shared_ptr<api::StellarLikeTransaction> getTransaction() override;

        private:
            api::StellarLikeOperationRecord _record;
            stellar::xdr::TransactionEnvelope _envelope;
            api::Currency _currency;
        };
    }
}


#endif //LEDGER_CORE_STELLARLIKEOPERATION_HPP
