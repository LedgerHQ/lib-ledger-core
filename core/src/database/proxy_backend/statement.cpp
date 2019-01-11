/*
 *
 * statement.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/11/2018.
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
#include <utils/Exception.hpp>
#include <utils/Try.hpp>
#include <iostream>

using namespace soci;
using namespace ledger::core;



void proxy_statement_backend::alloc() {
 // Do nothing
    SP_PRINT("ALLOC")
}

void proxy_statement_backend::clean_up() {
    SP_PRINT("CLEAN UP")
    if (_stmt) {
        _stmt->close();
        _stmt = nullptr;
    }
    if (_results) {
        _results->close();
        _results = nullptr;
    }
}

bool proxy_statement_backend::reset_if_necessary() {
    if (_results != nullptr) {
        _results->close();
        _results = nullptr;
        _stmt->reset();
        return true;
    }
    return false;
}

void proxy_statement_backend::prepare(std::string const &query, details::statement_type eType) {
    SP_PRINT("PREPARE " << query)
    make_try<Unit>([&, this] () {
        if (!_stmt) {
            _stmt = _session.get_connection()->prepareStatement(query, eType == details::statement_type::st_repeatable_query);
        } else {
            throw soci_error("Statement already prepared");
        }
        return unit;
    }).getOrThrowException<soci_error>();
}

details::statement_backend::exec_fetch_result proxy_statement_backend::execute(int number) {
    SP_PRINT("EXECUTE")
    return make_try<details::statement_backend::exec_fetch_result>([&, this] {
        reset_if_necessary();
        _results = _stmt->execute();
        if (number == 0)
            return _results->getRowNumber() == 0 ? ef_no_data : ef_success;
        else
            return fetch(number);
    }).getOrThrowException<soci_error>();
}

details::statement_backend::exec_fetch_result proxy_statement_backend::fetch(int number) {
    SP_PRINT("FETCH " << number)
    if (number > 1)
        return batch_fetch(number);
    else
        return single_row_fetch();
}

details::statement_backend::exec_fetch_result proxy_statement_backend::batch_fetch(int number) {
    throw soci_error("Batch operation are not supported by proxy backend");
}

details::statement_backend::exec_fetch_result proxy_statement_backend::single_row_fetch() {
    SP_PRINT("FETCH SINGLE")
    if (_results)
        _lastRow = _results->getRow();
    else
        _lastRow = nullptr;
    if (_results && _results->hasNext())
        _results = _results->next();
    else {
        _results = nullptr;
    }
    return _lastRow ? ef_success : ef_no_data;
}

long long proxy_statement_backend::get_affected_rows() {
    SP_PRINT("GET AFFECTED ROWS")
    return _results ? _results->getUpdateCount() : 0;
}

int proxy_statement_backend::get_number_of_rows() {
    SP_PRINT("GET NUMBER OF ROWS returns " << (_results ? _results->getRowNumber() : 0))
    return _results ? _results->getRowNumber() : 0;
}

std::string proxy_statement_backend::get_parameter_name(int index) const {
    SP_PRINT("GET PARAMETER NAME " << index)
    return std::string();
}

std::string proxy_statement_backend::rewrite_for_procedure_call(std::string const &query) {
    SP_PRINT("REWRITE FOR PROCEDURE CALL " << query)
    return query;
}

int proxy_statement_backend::prepare_for_describe() {
    SP_PRINT("PREPARE FOR DESCRIBE")
    return make_try<int >([&, this] () {
        return _stmt->getColumnCount();
    }).getOrThrowException<soci_error>();
}

void proxy_statement_backend::describe_column(int colNum, data_type &dtype, std::string &columnName) {
    SP_PRINT("DESCRIBE COLUMN " << colNum)
    make_try<Unit>([&, this] () {
        auto col = _stmt->describeColumn(colNum);
        switch (col->getType()) {
            case api::DatabaseValueType::STRING:
                dtype = data_type::dt_string;
                break;
            case api::DatabaseValueType::DATE:
                dtype = data_type::dt_date;
                break;
            case api::DatabaseValueType::DOUBLE:
                dtype = data_type::dt_double;
                break;
            case api::DatabaseValueType::INTEGER:
                dtype = data_type::dt_integer;
                break;
            case api::DatabaseValueType::LONG_LONG:
                dtype = data_type::dt_long_long;
                break;
            case api::DatabaseValueType::UNSIGNED_LONG_LONG:
                dtype = data_type::dt_unsigned_long_long;
                break;
            case api::DatabaseValueType::BLOB:
                dtype = data_type::dt_string;
                break;
        }
        columnName = col->getName();
        return unit;
    }).getOrThrowException<soci_error>();
}

proxy_standard_into_type_backend *proxy_statement_backend::make_into_type_backend() {
    SP_PRINT("MAKE INTO TYPE BACKEND")
    return new proxy_standard_into_type_backend(*this);
}

proxy_standard_use_type_backend *proxy_statement_backend::make_use_type_backend() {
    SP_PRINT("MAKE USE TYPE BACKEND")
    return new proxy_standard_use_type_backend(*this);
}

details::vector_into_type_backend *proxy_statement_backend::make_vector_into_type_backend() {
    SP_PRINT("MAKE VECTOR INTO TYPE BACKEND")
    throw soci_error("Unsupported feature used, bulk operation on SOCI");
}

details::vector_use_type_backend *proxy_statement_backend::make_vector_use_type_backend() {
    SP_PRINT("MAKE VECTOR USE TYPE BACKEND")
    throw soci_error("Unsupported feature used, bulk operation on SOCI");
}


