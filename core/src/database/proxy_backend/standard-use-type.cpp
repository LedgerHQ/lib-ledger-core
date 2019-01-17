/*
 *
 * standard-use_type.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/11/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "soci-proxy.h"
#include <utils/Try.hpp>

using namespace ledger::core;
using namespace soci;

#define BIND(T, value) \
    if (_position.isLeft()) { \
        statement_._stmt->bind##T(_position.getLeft(), (value)); \
    }

void
proxy_standard_use_type_backend::bind_by_pos(int &position, void *data, details::exchange_type type, bool /*readOnly*/) {
    SP_PRINT("SUTB BIND BY POS " << position << " type " << type)
    _position = position++;
    _data = data;
    _type = type;
}

void proxy_standard_use_type_backend::bind_by_name(std::string const &name, void *data, details::exchange_type type,
                                                   bool /*readOnly*/) {
    SP_PRINT("SUTB BIND BY NAME" << name)
    _position = name;
    _data = data;
    _type = type;
}

void proxy_standard_use_type_backend::pre_use(indicator const *ind) {
    SP_PRINT("SUTB PRE USE")
    make_try<Unit>([&] () {
        if (ind && *ind == i_null) {
            if (_position.isLeft()) {
                statement_._stmt->bindNull(_position.getLeft());
            }
            return unit;
        }
        switch (_type) {
            case details::x_char: throw soci_error("Unsupported type char.");
            case details::x_stdstring: {
                auto str = static_cast<std::string *>(_data);
                BIND(String, *str);
                break;
            }
            case details::x_short: {
                auto value = static_cast<short*>(_data);
                BIND(Short, (int16_t)(*value));
                break;
            }
            case details::x_integer: {
                auto i = static_cast<int *>(_data);
                BIND(Int, (int32_t)(*i));
                break;
            }
            case details::x_long_long: {
                auto ll = static_cast<int64_t *>(_data);
                BIND(Long, (long long)(*ll));
                break;
            }
            case details::x_unsigned_long_long: {
                auto ull = static_cast<unsigned long long*>(_data);
                BIND(Long, (int64_t)(*ull));
                break;
            }
            case details::x_double: {
                auto d = static_cast<double *>(_data);
                BIND(Double, *d);
                break;
            }
            case details::x_stdtm: throw soci_error("Unsupported type timestamp.");
            case details::x_statement: throw soci_error("Unsupported type statement in SQL query.");
            case details::x_blob: {
                auto blob = static_cast<soci::blob *>(_data);
                auto backend = dynamic_cast<proxy_blob_backend *>(blob->get_backend())->getBlob();
                BIND(Blob, backend);
                break;
            }
            case details::x_rowid: throw soci_error("Unsupported type row id in SQL query");
        }
        return unit;
    });
}

void proxy_standard_use_type_backend::post_use(bool gotData, indicator *ind) {
    SP_PRINT("SUTB POST USE")
    // Do nothing
}

void proxy_standard_use_type_backend::clean_up() {
    SP_PRINT("SUTB CLEAN UP")
    // Nothing to be done here
}
