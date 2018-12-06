/*
 *
 * standard-into-type.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 22/11/2018.
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
#include <soci.h>
#include <utils/Try.hpp>

using namespace soci;
using namespace ledger::core;

#define OUT(Type) (static_cast<Type *>(_data))
#define GET(Type, type) \
     make_try<type>([&, this] () { \
        return _statement._lastRow->get##Type##ByPos(index); \
     }).getOrThrowException<soci_error>()

void proxy_standard_into_type_backend::define_by_pos(int &position, void *data, details::exchange_type type) {
    _position = position++;
    _data = data;
    _type = type;
}

void proxy_standard_into_type_backend::pre_fetch() {
    // Let's do nothing
}

void proxy_standard_into_type_backend::post_fetch(bool gotData, bool calledFromFetch, indicator *ind) {
    SP_PRINT("INTO POST FETCH " << _position << " " << _type);
    if (gotData == false && calledFromFetch == true) return ;

    auto index = _position - 1;

    if (_statement._lastRow->isNullAtPos(index)) {

        *ind = i_null;
        return ;
    }

    if (ind != nullptr)
        *ind = i_ok;
    switch (_type) {
        case details::x_char: throw soci_error("Unsupported type char in select statement");
        case details::x_stdstring: {
            OUT(std::string)->assign(GET(String, std::string));
            break;
        }
        case details::x_short: {
            *OUT(short) = GET(Short, short);
            break;
        }
        case details::x_integer: {
            *OUT(int32_t) = GET(Int, int32_t);
            break;
        }
        case details::x_long_long: {
            *OUT(int64_t) = GET(Long, int64_t);
            break;
        }
        case details::x_unsigned_long_long: {
            *OUT(uint64_t) = GET(Long, uint64_t);
            break;
        }
        case details::x_double: {
            *OUT(double) = GET(Double, double);
            break;
        }
        case details::x_stdtm: throw soci_error("Unsupported type timestamp in select statement");
        case details::x_statement: throw soci_error("Unsupported type statement in select statement");
        case details::x_rowid: {
            throw soci_error("Unsupported type statement in select statement");
            break;
        }
        case details::x_blob: {
            auto b = GET(Blob, std::shared_ptr<api::DatabaseBlob>);
            auto blob = static_cast<proxy_blob_backend *>(static_cast<soci::blob *>(_data)->get_backend());
            blob->setBlob(b);
            break;
        }
    }

}

void proxy_standard_into_type_backend::clean_up() {
    // Do nothing again
}

