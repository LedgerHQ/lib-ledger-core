/*
 *
 * MemoryDatabaseProxy.cpp
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

 #include "MemoryDatabaseProxy.h"
#include <api/DatabaseConnectionPool.hpp>
#include <api/DatabaseConnection.hpp>
#include <api/DatabaseStatement.hpp>
#include <api/DatabaseBlob.hpp>
#include <api/DatabaseResultSet.hpp>

#include <sqlite3.h>
//#include <soci-sqlite3.h>
#include <database/proxy_backend/soci-proxy.h>
#include <soci.h>
#include <soci-backend.h>
#include <sstream>
#include <list>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <iterator>

#define BIND(Type) \
    auto pos = sqlite3_bind_parameter_index(_stmt, name.c_str()); \
    if (pos == 0) { \
        std::ostringstream ss; \
        ss << "Cannot bind to (by name) " << name; \
        throw soci::soci_error(ss.str()); \
    } \
    bind##Type(pos, value);

#define GET(Type) boost::lexical_cast<Type>(std::get<1>(_columns[pos]))

using namespace ledger::core;

typedef std::tuple<std::string, api::DatabaseValueType> type_tuple;

const std::vector<type_tuple> SQLITE3_TYPES {
        type_tuple("int8", api::DatabaseValueType::LONG_LONG),
        type_tuple("bigint", api::DatabaseValueType::LONG_LONG),
        type_tuple("unsigned big int", api::DatabaseValueType::LONG_LONG),
        type_tuple("int", api::DatabaseValueType::INTEGER),
        type_tuple("float", api::DatabaseValueType::DOUBLE),
        type_tuple("double", api::DatabaseValueType::DOUBLE),
        type_tuple("text", api::DatabaseValueType::STRING),
        type_tuple("blob", api::DatabaseValueType::BLOB)
};

class Blob : public api::DatabaseBlob {
public:
    explicit Blob(const std::vector<uint8_t>& buffer) : _buffer(buffer) {}
    Blob() = default;

    std::vector<uint8_t> read(int64_t offset, int64_t length) override {
        if (_buffer.size() <= offset)
            return std::vector<uint8_t>();
        auto endIterator = (offset + length) >= _buffer.size() ? _buffer.end() : _buffer.begin() + offset + length;
        return std::vector<uint8_t>(_buffer.begin() + offset, endIterator);
    }

    int64_t write(int64_t offset, const std::vector<uint8_t> &data) override {
        _buffer.insert(_buffer.begin() + offset, data.begin(), data.end());
        return static_cast<int64_t>(_buffer.size());
    }

    int64_t append(const std::vector<uint8_t> &data) override {
        _buffer.insert(_buffer.end(), data.begin(),data.end());
        return static_cast<int64_t>(_buffer.size());
    }

    int64_t trim(int64_t newLen) override {
        _buffer.resize(static_cast<unsigned long>(newLen));
        return size();
    }

    int64_t size() override {
        return static_cast<int64_t>(_buffer.size());
    }

    const std::vector<uint8_t>& getBuffer() const {
        return _buffer;
    }

private:
    std::vector<uint8_t> _buffer;
};

class Column : public api::DatabaseColumn {
public:
    Column(api::DatabaseValueType type, const std::string& name) : _type(type), _name(name) {};
    api::DatabaseValueType getType() override {
        return _type;
    }

    std::string getName() override {
        return _name;
    }

private:
    api::DatabaseValueType _type;
    std::string _name;
};

class ResultRow : public api::DatabaseResultRow {
public:
    ResultRow(sqlite3_stmt* stmt) {
        auto count = sqlite3_column_count(stmt);
        for (auto i = 0; i < count; i++) {
            auto size = sqlite3_column_bytes(stmt, i);
            auto data = static_cast<const uint8_t  *>(sqlite3_column_blob(stmt, i));
            std::string name(sqlite3_column_name(stmt, i));
            if (data != nullptr)
                _columns.emplace_back(name, std::string((const char *)data, (size_t) size), false);
            else
                _columns.emplace_back(name, std::string(), true);
        }
    }

    bool isNullAtPos(int32_t pos) override {
        return std::get<2>(_columns[pos]);
    }

    std::string getColumnName(int32_t pos) override {
        return std::get<0>(_columns[pos]);
    }

    int16_t getShortByPos(int32_t pos) override {
        return GET(short);
    }

    int32_t getIntByPos(int32_t pos) override {
        return GET(int);
    }

    float getFloatByPos(int32_t pos) override {
        return GET(float);
    }

    double getDoubleByPos(int32_t pos) override {
        return GET(double);
    }

    int64_t getLongByPos(int32_t pos) override {
        return GET(int64_t);
    }

    std::string getStringByPos(int32_t pos) override {
        return std::get<1>(_columns[pos]);
    }

    std::shared_ptr<api::DatabaseBlob> getBlobByPos(int32_t pos) override {
        const auto& v = std::get<1>(_columns[pos]);
        std::vector<uint8_t> buffer(v.begin(), v.end());
        return std::make_shared<Blob>(buffer);
    }

private:
    std::vector<std::tuple<std::string, std::string, bool>> _columns;
};

class Results : public api::DatabaseResultSet, public std::enable_shared_from_this<Results> {
public:

    Results(sqlite3* db, sqlite3_stmt* st) : _stmt(st), _changes(sqlite3_changes(db)) {
        // We read all results, this is not really efficient but this wrapper is only for tests
       read_all(db);
    }

    void read_all(sqlite3* db) {
    iterate:
        int last_result = sqlite3_step(_stmt);
        if (last_result == SQLITE_ROW) {
            _rows.push_back(std::make_shared<ResultRow>(_stmt));
            goto iterate;
        }
        else if (last_result != SQLITE_DONE) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
        final:
        if (_rows.size() > 0) {
            _rows.emplace_front(*_rows.begin());
        }
        _it = _rows.begin();
    }

    std::shared_ptr<api::DatabaseResultRow> getRow() override {
        return *_it;
    }

    int32_t getUpdateCount() override {
        return _changes;
    }

    int32_t available() override {
        return std::min(1, (int) std::distance(_it, _rows.end()));
    }

    bool hasNext() override {
        return std::distance(_it, _rows.end()) > 1;
    }

    void next() override {
        _it++;
    }

    void close() override {
        _rows = std::list<std::shared_ptr<ResultRow>>();
        _it = _rows.end();
    }

    std::shared_ptr<api::DatabaseError> getError() override {
        return nullptr;
    }

private:
    sqlite3_stmt* _stmt;

    std::list<std::shared_ptr<ResultRow>> _rows;
    std::list<std::shared_ptr<ResultRow>>::iterator _it;
    const int _changes;
};

class Statement : public api::DatabaseStatement {
public:
    Statement(sqlite3* db, const std::string& query) : _db(db) {
        SP_PRINT("Create statement with " << query);
        if (sqlite3_prepare_v2(db, query.c_str(), query.size(), &_stmt, NULL) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(_db));
        }
    }

    void bindShort(int32_t pos, int16_t value) override {
        sqlite3_bind_int(_stmt, pos, (int)value);
    }

    void bindInt(int32_t pos, int32_t value) override {
        sqlite3_bind_int(_stmt, pos, (int)value);
    }

    void bindLong(int32_t pos, int64_t value) override {
        sqlite3_bind_int64(_stmt, pos, (sqlite3_int64)value);
    }

    void bindFloat(int32_t pos, float value) override {
        sqlite3_bind_double(_stmt, pos, (double)value);
    }

    void bindDouble(int32_t pos, double value) override {
        sqlite3_bind_double(_stmt, pos, (double)value);
    }

    void bindString(int32_t pos, const std::string &value) override {
        sqlite3_bind_text(_stmt, pos, value.c_str(), value.size(), NULL);
    }

    void bindBlob(int32_t pos, const std::shared_ptr<api::DatabaseBlob> &value) override {
        sqlite3_bind_blob(
                _stmt,
                pos,
                std::dynamic_pointer_cast<Blob>(value)->getBuffer().data(),
                std::dynamic_pointer_cast<Blob>(value)->getBuffer().size(),
                NULL
        );
    }

    void bindNull(int32_t pos) override {
        sqlite3_bind_null(_stmt, pos);
    }

    void reset() override {
        sqlite3_reset(_stmt);
    }

    std::shared_ptr<api::DatabaseColumn> describeColumn(int32_t colNum) override {
        // Find type using decltype
        std::string type {sqlite3_column_decltype(_stmt, colNum - 1)};

        for (auto& match : SQLITE3_TYPES) {
            if (boost::iequals(type, std::get<0>(match))) {
                return std::make_shared<Column>(std::get<1>(match),   std::string(sqlite3_column_name(_stmt, colNum - 1)));
            }
        }

        // Find by hack

        throw std::runtime_error("Unknow type");
    }

    int32_t getColumnCount() override {
        return sqlite3_column_count(_stmt);
    }

    std::shared_ptr<api::DatabaseResultSet> execute() override {
        return std::make_shared<Results>(_db, _stmt);
    }

    void close() override {
        sqlite3_finalize(_stmt);
    }

private:
    sqlite3* _db;
    sqlite3_stmt* _stmt;
};

class Connection : public api::DatabaseConnection {
public:
    Connection(sqlite3* db, const std::string &dbName) : _db(db), _dbName(dbName) {};

    std::shared_ptr<api::DatabaseStatement> prepareStatement(const std::string &query, bool repeatable) override {
        return std::make_shared<Statement>(_db, query);
    };

    void begin() override {
        sqlite3_exec(_db, "BEGIN", 0, 0, 0);
    };

    void rollback() override {
        sqlite3_exec(_db, "ROLLBACK", 0, 0, 0);
    };

    void commit() override {
        sqlite3_exec(_db, "COMMIT", 0, 0, 0);
    };

    void close() override {
        sqlite3_close(_db);
    };

    std::shared_ptr<api::DatabaseBlob> newBlob() override {
        return std::make_shared<Blob>();
    }

    void check_sqlite_err(sqlite3 *db, int result, const std::string &errorMessage) {
        if (SQLITE_OK != result)
        {
            const char *errMsg = sqlite3_errmsg(db);
            std::ostringstream ss;
            ss << errorMessage << errMsg;
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, ss.str());
        }
    };

    void changePassword(const std::string & oldPassword, const std::string & newPassword) {
        setPassword(oldPassword);
        if (!newPassword.empty()) {
            auto res = sqlite3_rekey_v2(_db, _dbName.c_str(), newPassword.c_str(), strlen(newPassword.c_str()));
            check_sqlite_err(_db, res, "Failed to change database's password. ");
        }
    };

    void setPassword(const std::string &password) {
        if (!password.empty()) {
            auto res = sqlite3_key_v2(_db, _dbName.c_str(), password.c_str(), strlen(password.c_str()));
            check_sqlite_err(_db, res, "Failed to encrypt database. ");
        }
    };

    bool isAlive() override {
        return true;
    }

private:
    sqlite3* _db;
    std::string _dbName;
};

// Parse parameters got from opening a session
// Same as in soci_sqlite
static std::tuple<std::string, std::string, std::string> parseParameters(const std::string &parameters) {
    std::string passKey, newPassKey;
    std::string dbname(parameters);
    std::stringstream ssconn(parameters);
    while (!ssconn.eof() && ssconn.str().find('=') != std::string::npos) {
        std::string key, val;
        std::getline(ssconn, key, '=');
        std::getline(ssconn, val, ' ');

        if (val.size()>0 && val[0]=='\"') {
            std::string quotedVal = val.erase(0, 1);

            if (quotedVal[quotedVal.size()-1] ==  '\"') {
                quotedVal.erase(val.size()-1);
            }
            else {// space inside value string
                std::getline(ssconn, val, '\"');
                quotedVal = quotedVal + " " + val;
                std::string keepspace;
                std::getline(ssconn, keepspace, ' ');
            }

            val = quotedVal;
        }

        if ("dbname" == key || "db" == key) {
            dbname = val;
        }
        else if ("key" == key) {
            passKey = val;
        }
        else if ("new_key" == key) {
            newPassKey = val;
        }
    }

    return std::make_tuple(dbname, passKey, newPassKey);
};

class ConnectionPool : public api::DatabaseConnectionPool {
public:
    ConnectionPool(const std::string &connectUrl) {
        sqlite3* db;
        auto params = parseParameters(connectUrl);
        int connection_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        if (sqlite3_open_v2(std::get<0>(params).c_str(), &db, connection_flags, NULL) != SQLITE_OK) {
            throw std::runtime_error("Unable to open database");
        }
        _conn = std::make_shared<Connection>(db, std::get<0>(params));
        if (!std::get<1>(params).empty()) {
            _conn->setPassword(std::get<1>(params));
            if (!std::get<2>(params).empty()) {
                _conn->changePassword(std::get<1>(params), std::get<2>(params));
            }
        }
    }
    std::shared_ptr<api::DatabaseConnection> getConnection() override { return _conn; }

private:
    std::shared_ptr<Connection> _conn;
};

std::shared_ptr<ledger::core::api::DatabaseConnectionPool> MemoryDatabaseProxy::connect(const std::string &connectUrl) {
    return _pool = std::make_shared<ConnectionPool>(connectUrl);
}

int32_t MemoryDatabaseProxy::getPoolSize() {
    return 1;
}
