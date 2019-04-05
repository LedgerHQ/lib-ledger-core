//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton, David Courtney
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//


#include "soci-sqlite3.h"

#include <connection-parameters.h>

#include <sstream>
#include <string>
#include <cstdio>
#include <cstring>
#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace soci;
using namespace soci::details;
using namespace sqlite_api;

namespace // anonymous
{

// helper function for hardcoded queries
void execute_hardcoded(sqlite_api::sqlite3* conn, char const* const query, char const* const errMsg)
{
    char *zErrMsg = 0;
    int const res = sqlite3_exec(conn, query, 0, 0, &zErrMsg);
    if (res != SQLITE_OK)
    {
        std::ostringstream ss;
        ss << errMsg << " " << zErrMsg;
        sqlite3_free(zErrMsg);
        throw sqlite3_soci_error(ss.str(), res);
    }
}

void check_sqlite_err(sqlite_api::sqlite3* conn, int res, char const* const errMsg)
{
    if (SQLITE_OK != res)
    {
        const char *zErrMsg = sqlite3_errmsg(conn);
        std::ostringstream ss;
        ss << errMsg << zErrMsg;
        throw sqlite3_soci_error(ss.str(), res);
    }
}

} // namespace anonymous


sqlite3_session_backend::sqlite3_session_backend(
    connection_parameters const & parameters)
{
    int timeout = 0;
    int connection_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    std::string synchronous, passKey, newPassKey;
    std::string const & connectString = parameters.get_connect_string();
    std::string dbname(connectString);
    std::stringstream ssconn(connectString);

    while (!ssconn.eof() && ssconn.str().find('=') != std::string::npos)
    {
        std::string key, val;
        std::getline(ssconn, key, '=');
        std::getline(ssconn, val, ' ');

        if (val.size()>0 && val[0]=='\"')
        {
            std::string quotedVal = val.erase(0, 1);

            if (quotedVal[quotedVal.size()-1] ==  '\"')
            {
                quotedVal.erase(val.size()-1);
            }
            else // space inside value string
            {
                std::getline(ssconn, val, '\"');
                quotedVal = quotedVal + " " + val;
                std::string keepspace;
                std::getline(ssconn, keepspace, ' ');
            }

            val = quotedVal;
        }

        if ("dbname" == key || "db" == key)
        {
            dbname = val;
        }
        else if ("timeout" == key)
        {
            std::istringstream converter(val);
            converter >> timeout;
        }
        else if ("synchronous" == key)
        {
            synchronous = val;
        }
        else if ("shared_cache" == key && "true" == val)
        {
            connection_flags |=  SQLITE_OPEN_SHAREDCACHE;
        }
        else if ("key" == key)
        {
            passKey = val;
        }
        else if ("new_key" == key)
        {
            newPassKey = val;
        }
    }

    int res = sqlite3_open_v2(dbname.c_str(), &conn_, connection_flags, NULL);
    check_sqlite_err(conn_, res, "Cannot establish connection to the database. ");
    post_connection(dbname, connection_flags, timeout, synchronous);

    //Set password
    if (!passKey.empty())
    {
        res = sqlite3_key_v2(conn_, dbname.c_str(), passKey.c_str(), strlen(passKey.c_str()));
        check_sqlite_err(conn_, res, "Failed to encrypt database. ");

        if (!newPassKey.empty())
        {
            //Set new password
            res = sqlite3_rekey_v2(conn_, dbname.c_str(), newPassKey.c_str(), strlen(newPassKey.c_str()));
            check_sqlite_err(conn_, res, "Failed to change database's password. ");
        }
        else {
            // FIXME: security concerns: maybe we want to sanitize? :D
            // convert back to plain text; this method copies the current database in plaintext
            // mode; once it’s done, we must close the connection, rename the database and re-open
            // connection
            std::string const dbnamePlain = dbname + ".plain";
            std::string const query(
                "PRAGMA key = '" + passKey + "';"
                "ATTACH DATABASE '" + dbnamePlain + "' AS plaintext KEY '';" // empty key = plaintext
                "SELECT sqlcipher_export('plaintext');"
                "DETACH DATABASE plaintext;"
            );
            std::string const errMsg("Unable to disabling encryption");

            execute_hardcoded(conn_, query.c_str(), errMsg.c_str());
            auto r = sqlite3_close(conn_);

            if (r == SQLITE_OK) {
                // correctly closed, rename the file
                if (std::remove(dbname.c_str()) == 0) {
                    if (std::rename(dbnamePlain.c_str(), dbname.c_str()) == 0) {
                        res = sqlite3_open_v2(dbname.c_str(), &conn_, connection_flags, NULL);
                        check_sqlite_err(conn_, res, "Cannot establish connection to the database. ");
                        post_connection(dbname, connection_flags, timeout, synchronous);

                    } else {
                        // old DB removed but cannot rename the new one: bad
                    }
                } else {
                    // cannot remove the old database
                }
            } else {
                // likely a still busy database
            }
        }
    }
}

sqlite3_session_backend::~sqlite3_session_backend()
{
    clean_up();
}

void sqlite3_session_backend::begin()
{
    execute_hardcoded(conn_, "BEGIN", "Cannot begin transaction.");
}

void sqlite3_session_backend::commit()
{
    execute_hardcoded(conn_, "COMMIT", "Cannot commit transaction.");
}

void sqlite3_session_backend::rollback()
{
    execute_hardcoded(conn_, "ROLLBACK", "Cannot rollback transaction.");
}

void sqlite3_session_backend::post_connection(std::string const& dbname, int connection_flags, int timeout, std::string const& synchronous)
{
    int res = sqlite3_open_v2(dbname.c_str(), &conn_, connection_flags, NULL);
    check_sqlite_err(conn_, res, "Cannot establish connection to the database. ");

    if (!synchronous.empty())
    {
        std::string const query("pragma synchronous=" + synchronous);
        std::string const errMsg("Query failed: " + query);
        execute_hardcoded(conn_, query.c_str(), errMsg.c_str());
    }

    res = sqlite3_busy_timeout(conn_, timeout * 1000);
    check_sqlite_err(conn_, res, "Failed to set busy timeout for connection. ");
}

void sqlite3_session_backend::clean_up()
{
    sqlite3_close(conn_);
}

sqlite3_statement_backend * sqlite3_session_backend::make_statement_backend()
{
    return new sqlite3_statement_backend(*this);
}

sqlite3_rowid_backend * sqlite3_session_backend::make_rowid_backend()
{
    return new sqlite3_rowid_backend(*this);
}

sqlite3_blob_backend * sqlite3_session_backend::make_blob_backend()
{
    return new sqlite3_blob_backend(*this);
}
