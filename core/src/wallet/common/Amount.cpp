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
#include "Amount.h"
#include <utils/Exception.hpp>

namespace ledger {
    namespace core {

        Amount::Amount(const api::Currency &currency, int32_t unitIndex, const BigInt &value) {
            _currency = currency;
            _unitIndex = unitIndex;
            _value = value;
        }

        std::shared_ptr<api::BigInt> Amount::toBigInt() {
            return std::make_shared<api::BigIntImpl>(_value);
        }

        api::Currency Amount::getCurrency() {
            return _currency;
        }

        api::CurrencyUnit Amount::getUnit() {
            return _currency.units[_unitIndex];
        }

        std::shared_ptr<api::Amount> Amount::toUnit(const api::CurrencyUnit &unit) {
            auto index = 0;
            for (auto& u : _currency.units) {
                if (u.code == unit.code && u.name == unit.name && u.numberOfDecimal == unit.numberOfDecimal &&
                    u.symbol == unit.symbol) {
                    return std::make_shared<Amount>(
                            _currency,
                            index,
                            _value
                    );
                }
                index += 1;
            }
            throw make_exception(api::ErrorCode::CURRENCY_UNIT_NOT_FOUND, "Cannot find currency {}", unit.name);
        }

        std::string Amount::toString() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::string Amount::toString()");
        }

        int64_t Amount::toLong() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "int64_t Amount::toLong()");
        }

        double Amount::toDouble() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "double Amount::toDouble()");
        }

        std::string Amount::format(const api::Locale &locale, const optional<api::FormatRules> &rules) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::string Amount::format(const api::Locale &locale, const optional<api::FormatRules> &rules)");
        }
    }
}