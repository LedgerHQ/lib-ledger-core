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
#ifndef LEDGER_CORE_QUERYBUILDER_H
#define LEDGER_CORE_QUERYBUILDER_H

#include "QueryFilter.h"
#include <soci.h>
#include <list>
#include <tuple>

namespace ledger {
    namespace core {
        class QueryBuilder {
        public:
            QueryBuilder() {};
            QueryBuilder& select(const std::string& keys);
            QueryBuilder& select(std::string&& keys);
            QueryBuilder& from(const std::string& table);
            QueryBuilder& from(std::string&& table);
            QueryBuilder& where(const std::shared_ptr<QueryFilter>& filter);
            QueryBuilder& order(std::string&& keys, bool&& descending);
            soci::details::prepare_temp_type execute(soci::session& sql);

        private:
            std::string _keys;
            std::string _table;
            std::list<std::tuple<std::string, bool>> _order;
            std::shared_ptr<QueryFilter> _filter;
        };
    }
}


#endif //LEDGER_CORE_QUERYBUILDER_H
