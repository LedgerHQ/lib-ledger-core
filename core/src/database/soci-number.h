/*
 *
 * soci
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/06/2017.
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
#ifndef LEDGER_CORE_SOCI_NUMBER_H
#define LEDGER_CORE_SOCI_NUMBER_H

#include <soci.h>
#include <boost/lexical_cast.hpp>
#include <utils/Exception.hpp>
#include <math/BigInt.h>

namespace soci {

    template <>
    struct type_conversion<ledger::core::BigInt> {
        typedef long long base_type;
        static void from_base(base_type const & in, indicator ind, ledger::core::BigInt& out) {
            out = std::move(ledger::core::BigInt(in));
        }

        static void to_base(ledger::core::BigInt const & in, base_type & out, indicator & ind) {
            out = (base_type)in.toUint64();
        }

    };

    template<typename T>
    T get_number(const row& row, std::size_t pos) {
        auto prop = row.get_properties(pos);
        switch (prop.get_data_type()) {
            case dt_string:
            std::cout << pos << " " << row.get<std::string>(pos) << std::endl;
                return boost::lexical_cast<T>(row.get<std::string>(pos));
            case dt_date: throw ledger::core::Exception(ledger::core::api::ErrorCode::RUNTIME_ERROR, "SQL date cannot be casted to number");
            case dt_double: return (T) row.get<double>(pos);
            case dt_integer: return (T) row.get<int>(pos);
            case dt_long_long: return (T) row.get<long long>(pos);
            case dt_unsigned_long_long: return (T) row.get<unsigned long long>(pos);
        }
    };

}


#endif //LEDGER_CORE_SOCI_NUMBER_H
