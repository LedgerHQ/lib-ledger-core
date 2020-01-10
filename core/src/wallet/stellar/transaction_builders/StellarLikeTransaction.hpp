/*
 *
 * StellarLikeTransaction.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/08/2019.
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

#ifndef LEDGER_CORE_STELLARLIKETRANSACTION_HPP
#define LEDGER_CORE_STELLARLIKETRANSACTION_HPP

#include <api/StellarLikeTransaction.hpp>
#include <wallet/stellar/xdr/models.hpp>
#include <api/Currency.hpp>

namespace ledger {
    namespace core {
        class StellarLikeTransaction : public virtual api::StellarLikeTransaction {
        public:
            explicit StellarLikeTransaction(
                    const api::Currency& currency,
                    stellar::xdr::TransactionEnvelope&& envelope) : _envelope(envelope), _currency(currency) {};
            explicit StellarLikeTransaction(
                    const api::Currency& currency,
                    const stellar::xdr::TransactionEnvelope& envelope) : _envelope(envelope), _currency(currency) {};
            std::vector<uint8_t> toRawTransaction() override;

            std::vector<uint8_t> toSignatureBase() override;

            const stellar::xdr::TransactionEnvelope& envelope() const { return _envelope; }

            void putSignature(const std::vector<uint8_t> &signature,
                              const std::shared_ptr<api::Address>& address) override;


            std::shared_ptr<api::Address> getSourceAccount() override;

            std::shared_ptr<api::BigInt> getSourceAccountSequence() override;

            std::shared_ptr<api::Amount> getFee() override;

        private:
            stellar::xdr::TransactionEnvelope _envelope;
            api::Currency _currency;
        };
    }
}


#endif //LEDGER_CORE_STELLARLIKETRANSACTION_HPP
