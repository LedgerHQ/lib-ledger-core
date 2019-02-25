/*
 *
 * BigIntImpl
 * ledger-core
 *
 * Created by Pierre Pollastri on 04/11/2016.
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
#ifndef LEDGER_CORE_BIGINTIMPL_HPP
#define LEDGER_CORE_BIGINTIMPL_HPP

#include "api/BigInt.hpp"
#include "math/BigInt.h"

namespace ledger { namespace core { namespace api {

        class BigIntImpl : public ledger::core::api::BigInt {

        public:
            BigIntImpl(const ledger::core::BigInt &v) : _bigi(v) {}

            virtual std::shared_ptr<ledger::core::api::BigInt>
            add(const std::shared_ptr<ledger::core::api::BigInt> &i) override;

            virtual std::shared_ptr<ledger::core::api::BigInt>
            subtract(const std::shared_ptr<ledger::core::api::BigInt> &i) override;

            virtual std::shared_ptr<ledger::core::api::BigInt>
            multiply(const std::shared_ptr<ledger::core::api::BigInt> &i) override;

            virtual std::shared_ptr<ledger::core::api::BigInt>
            divide(const std::shared_ptr<ledger::core::api::BigInt> &i) override;

            virtual std::vector<std::shared_ptr<ledger::core::api::BigInt>>
            divideAndRemainder(const std::shared_ptr<ledger::core::api::BigInt> &i) override;

            virtual std::shared_ptr<ledger::core::api::BigInt> pow(int32_t exponent) override;

            virtual std::string toDecimalString(int32_t precision, const std::string &decimalSeparator,
                                                const std::string &thousandSeparator) override;

            virtual int32_t intValue() override;

            virtual int32_t compare(const std::shared_ptr<ledger::core::api::BigInt> &i) override;

            virtual std::string toString(int32_t radix) override;

        private:
            const ledger::core::BigInt _bigi;
        };
    }
}
}

#endif //LEDGER_CORE_BIGINTIMPL_HPP
