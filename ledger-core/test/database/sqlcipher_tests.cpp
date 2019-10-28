/*
 *
 * sqlcipher_tests
 *
 * Created by El Khalil Bellakrid on 30/01/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include <gtest/gtest.h>
#include "BaseFixture.h"
#include <utils/FilesystemUtils.h>
#include <soci.h>
#include <soci-sqlite3.h>
#include <chrono>
using namespace ledger::core;


class SQLCipherTest : public BaseFixture {
};

TEST(SQLCipherTest, SanityCheck) {
    auto date = DateUtils::toJSON(std::chrono::system_clock::now());
    std::remove(date.begin(), date.end(), ':');
    auto dbName = "test_db_" + date;
    auto password = "test_key";
    auto newPassword = "test_key_new";
    {
        //Initiate the DB and interact with it
        auto parameters = fmt::format("dbname=\"{}\" ", dbName) + fmt::format("key=\"{}\" ", password);
        soci::session session;
        session.open(*soci::factory_sqlite3(), parameters);
        session << "CREATE TABLE test_table ("
                "    id INTEGER,"
                "    name VARCHAR(255)"
                ")";

        int value_0 = 123;
        std::string value_1 = "test_name";
        session << "INSERT INTO test_table(id, name) VALUES(:field_0, :field_1)",
                soci::use(value_0), soci::use(value_1);
        session.close();
    }
    {
        //Read previously created DB and check values
        auto parameters = fmt::format("dbname=\"{}\" ", dbName) + fmt::format("key=\"{}\" ", password);
        soci::session session;
        session.open(*soci::factory_sqlite3(), parameters);
        int value_0;
        std::string value_1;
        session << "SELECT id, name FROM test_table", soci::into(value_0), soci::into(value_1);
        EXPECT_EQ(value_0, 123);
        EXPECT_EQ(value_1, "test_name");
        session.close();
    }

    {
        //Change password
        auto factory = soci::factory_sqlite3();
        auto parameters = fmt::format("dbname=\"{}\" ", dbName) + fmt::format("key=\"{}\" ", password);
        soci::session session;
        session.open(*factory, parameters);
        int value_0;
        std::string value_1;

        EXPECT_NO_THROW((session << "SELECT id, name FROM test_table", soci::into(value_0), soci::into(value_1)));

        session.close();

        auto newParameters = fmt::format("dbname=\"{}\" ", dbName) + fmt::format("key=\"{}\" ", password) + fmt::format("new_key=\"{}\" ", newPassword);
        session.open(*factory, newParameters);
        EXPECT_NO_THROW((session << "SELECT id, name FROM test_table", soci::into(value_0), soci::into(value_1)));

        session.close();
    }
}

// Preventing terminating in environments that don't properly define __cplusplus macros
// TODO: remove this check after we migrate to VS2017 and provide /Zc:__cplusplus option
// https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
#if __cplusplus >= 201103L
TEST(SQLCipherTest, ThrowIfWrongPassword) {
    auto date = DateUtils::toJSON(std::chrono::system_clock::now());
    std::remove(date.begin(), date.end(), ':');
    auto dbName = "test_db2_" + date;
    auto password = "test_key";
    auto newPassword = "test_key_new";
    {
        //Initiate the DB and interact with it
        auto parameters = fmt::format("dbname=\"{}\" ", dbName) + fmt::format("key=\"{}\" ", password);
        soci::session session;
        session.open(*soci::factory_sqlite3(), parameters);
        session << "CREATE TABLE test_table ("
            "    id INTEGER,"
            "    name VARCHAR(255)"
            ")";

        int value_0 = 123;
        std::string value_1 = "test_name";
        session << "INSERT INTO test_table(id, name) VALUES(:field_0, :field_1)",
            soci::use(value_0), soci::use(value_1);
        session.close();
    }
    {
        //Fail gracefully
        int value_0;
        std::string value_1;
        soci::session session;
        session.open(*soci::factory_sqlite3(), dbName);
        EXPECT_THROW((session << "SELECT id, name FROM test_table", soci::into(value_0), soci::into(value_1)), std::runtime_error);
        session.close();
    }
}
#endif

TEST(SQLCipherTest, DisableEncryption) {
    auto date = DateUtils::toJSON(std::chrono::system_clock::now());
    std::remove(date.begin(), date.end(), ':');
    auto dbName = "test_db_" + date;
    auto password = "test_key";
    auto newPassword = "test_key_new";
    {
        //Initiate the DB and interact with it
        auto parameters = fmt::format("dbname=\"{}\" ", dbName) + fmt::format("key=\"{}\" ", password);
        soci::session session;
        session.open(*soci::factory_sqlite3(), parameters);
        session << "CREATE TABLE test_table ("
                "    id INTEGER,"
                "    name VARCHAR(255)"
                ")";

        int value_0 = 123;
        std::string value_1 = "test_name";
        session << "INSERT INTO test_table(id, name) VALUES(:field_0, :field_1)",
                soci::use(value_0), soci::use(value_1);
        session.close();
    }

    {
        //Remove encryption
        auto parameters = fmt::format("dbname=\"{}\" ", dbName) + fmt::format("key=\"{}\" ", password) + "disable_encryption=\"true\" ";
        soci::session session;
        session.open(*soci::factory_sqlite3(), parameters);
        session.close();
    }

    {
        //Read previously created DB and check values (without encryption)
        auto parameters = fmt::format("dbname=\"{}\" ", dbName);
        soci::session session;
        session.open(*soci::factory_sqlite3(), parameters);
        int value_0;
        std::string value_1;
        session << "SELECT id, name FROM test_table", soci::into(value_0), soci::into(value_1);
        EXPECT_EQ(value_0, 123);
        EXPECT_EQ(value_1, "test_name");
        session.close();
    }
}
