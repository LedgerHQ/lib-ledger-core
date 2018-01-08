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
#include <fmt/format.h>

namespace ledger {
    namespace core {

        soci::details::prepare_temp_type QueryBuilder::execute(soci::session &sql) {
            std::stringstream query;
            query << "SELECT " << _keys << " FROM " << _table<< " AS " << _output;
            if (_outerJoin.nonEmpty()) {
                query << " LEFT OUTER JOIN " << std::get<0>(_outerJoin.getValue()) << " ON " << std::get<1>(_outerJoin.getValue());
            }

            if (_filter) {
                query << " WHERE ";
                std::string sFilter = _filter->getHead()->toString();
                query << sFilter;
            }

            std::cout<<query.str()<<std::endl;

            if (_order.size() > 0) {
                query << " ORDER BY ";
                for (auto it = _order.begin(); it != _order.end(); it++) {
                    auto& order = *it;
                    query << std::get<0>(order) << (std::get<1>(order) ? " DESC" : " ASC");

                    if (std::distance(it, _order.end()) > 1) {
                        query << ",";
                    }
                }
            }

            if (_limit.nonEmpty()) {
                query << " LIMIT " << _limit.getValue();
            }
            if (_offset.nonEmpty()) {
                query << " OFFSET " << _offset.getValue();
            }


            soci::details::prepare_temp_type statement = sql.prepare << query.str();
            if (_filter) {
                _filter->getHead()->bindValue(statement);
            }
            return statement;
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

        QueryBuilder &QueryBuilder::to(const std::string &output) {
            _output = std::move(output);
            return *this;
        }

        QueryBuilder &QueryBuilder::to(std::string &&output) {
            _output = output;
            return *this;
        }

        QueryBuilder &QueryBuilder::where(const std::shared_ptr<api::QueryFilter> &filter) {
            _filter = std::dynamic_pointer_cast<QueryFilter>(filter);
            return *this;
        }

        QueryBuilder &QueryBuilder::order(std::string &&keys, bool&& descending) {
            _order.push_back(std::move(std::make_tuple(keys, descending)));
            return *this;
        }

        QueryBuilder &QueryBuilder::limit(int32_t limit) {
            _limit = limit;
            return *this;
        }

        QueryBuilder &QueryBuilder::offset(int32_t offset) {
            _offset = offset;
            return *this;
        }

        QueryBuilder& QueryBuilder::outerJoin(const std::string &table, const std::string &condition) {
            _outerJoin = Option<LeftOuterJoin>(std::make_tuple(table, condition));
            return *this;
        }

    }
}