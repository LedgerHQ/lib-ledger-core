/*
 *
 * QueryBuilder
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/06/2017.
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
#include "QueryBuilder.h"

namespace ledger {
    namespace core {

        soci::details::prepare_temp_type QueryBuilder::execute(soci::session &sql) {
            return sql.prepare << "SELECT * FROM operations";
        }

        QueryBuilder &QueryBuilder::select(const std::string &keys) {
            _keys = std::move(keys);
            return *this;
        }

        QueryBuilder &QueryBuilder::select(std::string &&keys) {
            _keys = keys;
            return *this;
        }

        QueryBuilder &QueryBuilder::from(const std::string &table) {
            _table = std::move(table);
            return *this;
        }

        QueryBuilder &QueryBuilder::from(std::string &&table) {
            _table = table;
            return *this;
        }

        QueryBuilder &QueryBuilder::where(const std::shared_ptr<QueryFilter> &filter) {
            _filter = filter;
            return *this;
        }

        QueryBuilder &QueryBuilder::order(std::string &&keys, bool&& descending) {
            _order.push_back(std::move(std::make_tuple(keys, descending)));
            return *this;
        }


    }
}