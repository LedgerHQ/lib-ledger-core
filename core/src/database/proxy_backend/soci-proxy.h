/*
 *
 * soci-proxy.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/11/2018.
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

#ifndef SOCI_PROXY_H_INCLUDED
#define SOCI_PROXY_H_INCLUDED

// If SOCI_PROXY_DECL isn't defined yet define it now
#ifndef SOCI_PROXY_DECL
# define SOCI_PROXY_DECL
#endif

#include <soci-backend.h>
#include <api/DatabaseEngine.hpp>
#include <api/DatabaseConnectionPool.hpp>
#include <api/DatabaseConnection.hpp>
#include <api/DatabaseStatement.hpp>
#include <api/DatabaseError.hpp>
#include <api/DatabaseBlob.hpp>
#include <api/DatabaseResultSet.hpp>
#include <api/DatabaseValueType.hpp>
#include <api/DatabaseColumn.hpp>
#include <api/DatabaseResultRow.hpp>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utils/Either.hpp>
#include <soci.h>
#include <api/DatabaseRowId.hpp>

#ifndef SOCI_OVERRIDE
#   define SOCI_OVERRIDE override
#endif

#define SOCI_PROXY_DEBUG
#ifdef SOCI_PROXY_DEBUG
#define SP_PRINT(p) std::cout << p << std::endl;
#endif

namespace soci
{

    struct proxy_statement_backend;

    struct SOCI_PROXY_DECL proxy_standard_into_type_backend : details::standard_into_type_backend
    {
        proxy_standard_into_type_backend(proxy_statement_backend &st)
                : _statement(st)
        {}

        void define_by_pos(int& position, void* data, details::exchange_type type) SOCI_OVERRIDE;

        void pre_fetch() SOCI_OVERRIDE;
        void post_fetch(bool gotData, bool calledFromFetch, indicator* ind) SOCI_OVERRIDE;

        void clean_up() SOCI_OVERRIDE;

        proxy_statement_backend& _statement;
        void* _data;
        int _position;
        details::exchange_type  _type;
    };

    struct SOCI_PROXY_DECL proxy_vector_into_type_backend : details::vector_into_type_backend
    {
        proxy_vector_into_type_backend(proxy_statement_backend &st)
                : statement_(st)
        {}

        void define_by_pos(int& position, void* data, details::exchange_type type) SOCI_OVERRIDE;

        void pre_fetch() SOCI_OVERRIDE;
        void post_fetch(bool gotData, indicator* ind) SOCI_OVERRIDE;

        void resize(std::size_t sz) SOCI_OVERRIDE;
        std::size_t size() SOCI_OVERRIDE;

        void clean_up() SOCI_OVERRIDE;

        proxy_statement_backend& statement_;
    };

    struct SOCI_PROXY_DECL proxy_standard_use_type_backend : details::standard_use_type_backend
    {
        proxy_standard_use_type_backend(proxy_statement_backend &st)
                : statement_(st), _position(-1)
        {}

        void bind_by_pos(int& position, void* data, details::exchange_type type, bool readOnly) SOCI_OVERRIDE;
        void bind_by_name(std::string const& name, void* data, details::exchange_type type, bool readOnly) SOCI_OVERRIDE;

        void pre_use(indicator const* ind) SOCI_OVERRIDE;
        void post_use(bool gotData, indicator* ind) SOCI_OVERRIDE;

        void clean_up() SOCI_OVERRIDE;

        proxy_statement_backend& statement_;
        ledger::core::Either<int, std::string> _position;
        void* _data;
        details::exchange_type _type;
    };

    struct SOCI_PROXY_DECL proxy_vector_use_type_backend : details::vector_use_type_backend
    {
        proxy_vector_use_type_backend(proxy_statement_backend &st)
                : statement_(st) {}

        void bind_by_pos(int& position, void* data, details::exchange_type type) SOCI_OVERRIDE;
        void bind_by_name(std::string const& name, void* data, details::exchange_type type) SOCI_OVERRIDE;

        void pre_use(indicator const* ind) SOCI_OVERRIDE;

        std::size_t size() SOCI_OVERRIDE;

        void clean_up() SOCI_OVERRIDE;

        proxy_statement_backend& statement_;
    };

    struct proxy_session_backend;
    struct SOCI_PROXY_DECL proxy_statement_backend : details::statement_backend
    {
        proxy_statement_backend(proxy_session_backend &session) : _session(session) {};

        void alloc() SOCI_OVERRIDE;
        void clean_up() SOCI_OVERRIDE;
        void prepare(std::string const& query, details::statement_type eType) SOCI_OVERRIDE;

        exec_fetch_result execute(int number) SOCI_OVERRIDE;
        exec_fetch_result fetch(int number) SOCI_OVERRIDE;

        long long get_affected_rows() SOCI_OVERRIDE;
        int get_number_of_rows() SOCI_OVERRIDE;
        std::string get_parameter_name(int index) const;

        std::string rewrite_for_procedure_call(std::string const& query) SOCI_OVERRIDE;

        int prepare_for_describe() SOCI_OVERRIDE;
        void describe_column(int colNum, data_type& dtype, std::string& columnName) SOCI_OVERRIDE;

        proxy_standard_into_type_backend* make_into_type_backend() SOCI_OVERRIDE;
        proxy_standard_use_type_backend* make_use_type_backend() SOCI_OVERRIDE;
        proxy_vector_into_type_backend* make_vector_into_type_backend() SOCI_OVERRIDE;
        proxy_vector_use_type_backend* make_vector_use_type_backend() SOCI_OVERRIDE;

        bool reset_if_necessary();

        proxy_session_backend& _session;
        std::shared_ptr<ledger::core::api::DatabaseStatement> _stmt;
        std::shared_ptr<ledger::core::api::DatabaseResultSet> _results;
    };

    struct proxy_rowid_backend : details::rowid_backend
    {
        proxy_rowid_backend(proxy_session_backend &session);

        ~proxy_rowid_backend() SOCI_OVERRIDE;

        std::shared_ptr<ledger::core::api::DatabaseRowId> rowid;
    };

    struct proxy_blob_backend : details::blob_backend
    {
        proxy_blob_backend(proxy_session_backend& session);

        ~proxy_blob_backend() SOCI_OVERRIDE;

        std::size_t get_len() SOCI_OVERRIDE;
        std::size_t read(std::size_t offset, char* buf, std::size_t toRead) SOCI_OVERRIDE;
        std::size_t write(std::size_t offset, char const* buf, std::size_t toWrite) SOCI_OVERRIDE;
        std::size_t append(char const* buf, std::size_t toWrite) SOCI_OVERRIDE;
        void trim(std::size_t newLen) SOCI_OVERRIDE;

        proxy_session_backend& session_;

        std::shared_ptr<ledger::core::api::DatabaseBlob> blob;
    };

    struct proxy_session_backend : details::session_backend
    {
        proxy_session_backend(const std::shared_ptr<ledger::core::api::DatabaseConnection>& pool);

        ~proxy_session_backend() SOCI_OVERRIDE;

        void begin() SOCI_OVERRIDE;
        void commit() SOCI_OVERRIDE;
        void rollback() SOCI_OVERRIDE;

        std::string get_dummy_from_table() const { return std::string(); }

        std::string get_backend_name() const SOCI_OVERRIDE { return "proxy"; }
        std::shared_ptr<ledger::core::api::DatabaseConnection> get_connection() const { return _conn; };
        void clean_up();

        proxy_statement_backend* make_statement_backend() SOCI_OVERRIDE;
        proxy_rowid_backend* make_rowid_backend() SOCI_OVERRIDE;
        proxy_blob_backend* make_blob_backend() SOCI_OVERRIDE;

    private:
        std::shared_ptr<ledger::core::api::DatabaseConnection> _conn;
    };

    struct SOCI_PROXY_DECL proxy_backend_factory : backend_factory
    {
        proxy_backend_factory(const std::shared_ptr<ledger::core::api::DatabaseEngine>& engine) : _engine(engine) {}
        proxy_session_backend* make_session(connection_parameters const& parameters) const SOCI_OVERRIDE;
        const std::shared_ptr<ledger::core::api::DatabaseEngine>& getEngine() const;
        ~proxy_backend_factory();

    private:

        std::shared_ptr<ledger::core::api::DatabaseConnectionPool> get_pool(connection_parameters const& parameters) const; // Workaround to make the god damn adapter match the interface

        std::shared_ptr<ledger::core::api::DatabaseEngine> _engine;
        mutable std::unordered_map<std::string, std::shared_ptr<ledger::core::api::DatabaseConnectionPool>> _pools; // Yep ugly
    };

    extern SOCI_PROXY_DECL proxy_backend_factory const proxy;

    SOCI_PROXY_DECL backend_factory const* factory_proxy(const std::shared_ptr<ledger::core::api::DatabaseEngine>& engine);
    SOCI_PROXY_DECL void register_factory_proxy();

} // namespace soci

#undef SOCI_OVERRIDE
#endif // SOCI_PROXY_H_INCLUDED