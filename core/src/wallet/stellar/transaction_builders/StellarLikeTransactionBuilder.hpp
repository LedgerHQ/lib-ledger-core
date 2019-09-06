/*
 *
 * StellarLikeTransactionBuilder.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/08/2019.
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

#ifndef LEDGER_CORE_STELLARLIKETRANSACTIONBUILDER_HPP
#define LEDGER_CORE_STELLARLIKETRANSACTIONBUILDER_HPP

#include <api/StellarLikeTransactionBuilder.hpp>
#include <async/Future.hpp>
#include <wallet/stellar/xdr/models.hpp>
#include "StellarLikeTransaction.hpp"

namespace ledger {
    namespace core {

        class StellarLikeAccount;
        class StellarLikeTransactionBuilder : public virtual api::StellarLikeTransactionBuilder,
                public std::enable_shared_from_this<StellarLikeTransactionBuilder> {
        public:
            explicit StellarLikeTransactionBuilder(const std::shared_ptr<StellarLikeAccount>& account);
            std::shared_ptr<api::StellarLikeTransactionBuilder>
            addNativePayment(const std::string &address, const std::shared_ptr<api::Amount> &amount) override;

            std::shared_ptr<api::StellarLikeTransactionBuilder>
            addCreateAccount(const std::string &address, const std::shared_ptr<api::Amount> &amount) override;

            std::shared_ptr<api::StellarLikeTransactionBuilder>
            setBaseFee(const std::shared_ptr<api::Amount> &baseFee) override;

            std::shared_ptr<api::StellarLikeTransactionBuilder>
            putSignature(const std::vector<uint8_t> &signature) override;

            std::shared_ptr<api::StellarLikeTransactionBuilder>
            setTextMemo(const std::string &text) override;

            std::shared_ptr<api::StellarLikeTransactionBuilder>
            setNumberMemo(const std::shared_ptr<api::BigInt> &number) override;

            std::shared_ptr<api::StellarLikeTransactionBuilder>
            setHashMemo(const std::vector<uint8_t> &hash) override;

            std::shared_ptr<api::StellarLikeTransactionBuilder>
            setReturnMemo(const std::vector<uint8_t> &value) override;

            std::shared_ptr<api::StellarLikeTransactionBuilder>
            setSequence(const std::shared_ptr<api::BigInt> &sequence) override;

            void build(const std::shared_ptr<api::StellarLikeTransactionCallback> &callback) override;
            FuturePtr<api::StellarLikeTransaction> build();

        private:
            std::shared_ptr<StellarLikeAccount> _account;
            stellar::xdr::TransactionEnvelope _envelope;
            Option<uint64_t> _baseFee;
            BigInt _balanceChange;

        };
    }
}


#endif //LEDGER_CORE_STELLARLIKETRANSACTIONBUILDER_HPP
