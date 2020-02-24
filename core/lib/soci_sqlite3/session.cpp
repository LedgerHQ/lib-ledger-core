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

    void post_connection_helper(sqlite_api::sqlite3* conn, const std::string &dbname, int connection_flags, int timeout, const std::string &synchronous)
    {
        int res = sqlite3_open_v2(dbname.c_str(), &conn, connection_flags, NULL);
        check_sqlite_err(conn, res, "Cannot establish connection to the database. ");

        if (!synchronous.empty())
        {
            std::string const query("pragma synchronous=" + synchronous);
            std::string const errMsg("Query failed: " + query);
            execute_hardcoded(conn, query.c_str(), errMsg.c_str());
        }

        res = sqlite3_busy_timeout(conn, timeout * 1000);
        check_sqlite_err(conn, res, "Failed to set busy timeout for connection. ");
    }

    void sqlcipher_export(sqlite_api::sqlite3* conn,
                          int flags,
                          int timeout,
                          const std::string &synchronous,
                          const std::string &exportQuery,
                          const std::string &dbName,
                          const std::string &newDbName,
                          const std::string &msgError,
                          const std::string &newPassKey)
    {
        execute_hardcoded(conn, exportQuery.c_str(), msgError.c_str());
        auto r = sqlite3_close_v2(conn);

        if (r == SQLITE_OK) {
            // correctly closed, rename the file
            if (std::remove(dbName.c_str()) == 0) {
                if (std::rename(newDbName.c_str(), dbName.c_str()) == 0) {
                    r = sqlite3_open_v2(dbName.c_str(), &conn, flags, NULL);
                    check_sqlite_err(conn, r, "Cannot establish connection to the database. ");
                    post_connection_helper(conn, dbName, flags, timeout, synchronous);

                    if (!newPassKey.empty()) {
                        r = sqlite3_key_v2(conn, dbName.c_str(), newPassKey.c_str(), strlen(newPassKey.c_str()));
                        check_sqlite_err(conn, r, "Cannot establish connection to the database with newly set password. ");
                    }
                } else {
                    // old DB removed but cannot rename the new one: bad
                    throw sqlite3_soci_error("Cannot rename new SQLCipher database", r);
                }
            } else {
                // cannot remove the old database
                throw sqlite3_soci_error("Cannot remove old SQLCipher database", r);
            }
        } else {
            // likely a still busy database
            throw sqlite3_soci_error("Cannot close SQLCipher database", r);
        }
    }

} // namespace anonymous


sqlite3_session_backend::sqlite3_session_backend(
    connection_parameters const & parameters)
{
    int timeout = 0;
    int connection_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    std::string synchronous, passKey, newPassKey;
    bool disableEncryption = false;
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
        else if ("disable_encryption" == key && val == "true")
        {
            disableEncryption = true;
        }
    }

    int res = sqlite3_open_v2(dbname.c_str(), &conn_, connection_flags, NULL);
    check_sqlite_err(conn_, res, "Cannot establish connection to the database. ");
    post_connection(dbname, connection_flags, timeout, synchronous);

    //Set password
    if (!passKey.empty() || !newPassKey.empty())
    {
        if (!passKey.empty()){
            res = sqlite3_key_v2(conn_, dbname.c_str(), passKey.c_str(), strlen(passKey.c_str()));
            check_sqlite_err(conn_, res, "Failed to encrypt database. ");
        }

        if (!passKey.empty() && !newPassKey.empty()) {
            //Set new password
            res = sqlite3_rekey_v2(conn_, dbname.c_str(), newPassKey.c_str(), strlen(newPassKey.c_str()));
            check_sqlite_err(conn_, res, "Failed to change database's password. ");
        }
        else if (!newPassKey.empty()) {
            std::string const dbNameEncrypted = dbname + ".encrypt";
            std::string const query(
                            "ATTACH DATABASE '" + dbNameEncrypted + "' AS encrypted KEY '" + newPassKey +"';"
                            "SELECT sqlcipher_export('encrypted');"
                            "DETACH DATABASE encrypted;"
            );
            std::string const errMsg("Unable to enable encryption");
            sqlcipher_export(conn_, connection_flags, timeout, synchronous, query, dbname,  dbNameEncrypted, errMsg, newPassKey);
        }
        else if (disableEncryption)
        {
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
            sqlcipher_export(conn_, connection_flags, timeout, synchronous, query, dbname, dbnamePlain, errMsg, newPassKey);
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
    post_connection_helper(conn_, dbname, connection_flags, timeout, synchronous);
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

bool sqlite3_session_backend::isAlive() const {
    return true;
}
