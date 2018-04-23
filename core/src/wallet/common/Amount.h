/*
 *
 * Amount
 * ledger-core
 *
 * Created by Pierre Pollastri on 30/06/2017.
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
#ifndef LEDGER_CORE_AMOUNT_H
#define LEDGER_CORE_AMOUNT_H

#include <api/Amount.hpp>
#include <api/Currency.hpp>
#include <math/BigInt.h>
#include <api_impl/BigIntImpl.hpp>

namespace ledger {
    namespace core {
        class Amount : public api::Amount {
        public:
            Amount(const api::Currency& currency, int32_t unitIndex, const BigInt& value);
            Amount(const api::Currency& currency, int32_t unitIndex, BigInt&& value);
            std::shared_ptr<api::BigInt> toBigInt() override;
            api::Currency getCurrency() override;
            api::CurrencyUnit getUnit() override;
            std::shared_ptr<api::Amount> toUnit(const api::CurrencyUnit &unit) override;
            std::string toString() override;
            int64_t toLong() override;
            double toDouble() override;
            std::string format(const api::Locale &locale, const optional<api::FormatRules> &rules) override;
            std::shared_ptr<api::Amount> toMagnitude(int32_t magnitude) override;
            std::shared_ptr<ledger::core::BigInt> value() const;

        private:
            int32_t getMagnitude() const;

        private:
            BigInt _value;
            int32_t _unitIndex;
            api::Currency _currency;
        };
    }
}


#endif //LEDGER_CORE_AMOUNT_H
