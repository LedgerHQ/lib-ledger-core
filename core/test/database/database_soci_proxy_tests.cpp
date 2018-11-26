/*
 *
 * database_soci_proxy_tests.cpp
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

#include <database/ProxyBackend.h>
#include <gtest/gtest.h>
#include <soci.h>
#include "MemoryDatabaseProxy.h"

using namespace ledger::core;

class SociProxyTest : public ::testing::Test {
protected:
    void SetUp() override {
        Test::SetUp();
        auto engine = std::make_shared<MemoryDatabaseProxy>();
        _backend = std::make_shared<ProxyBackend>(engine);
        _backend->enableQueryLogging(true);
        _backend->init(nullptr, "test_db", sql);
    }

    void TearDown() override {
        Test::TearDown();
    }

    void createTables(soci::session& sql) {
        sql << "CREATE TABLE item ("
               "    name TEXT,"
               "    quantity BIGINT,"
               "    owner_id INT"
               ")";

        sql << "CREATE TABLE people ("
               "    id INT PRIMARY KEY NOT NULL,"
               "    name TEXT NOT NULL,"
               "    age BIGINT,"
               "    grade DOUBLE,"
               "    picture BLOB"
               ")";
    }

    void insertPeople(soci::session& sql, int id, const std::string& name, int64_t age, double grade, const std::vector<uint8_t>& picture) {
        //soci::blob blob(sql);
        //blob.write(0, (const char *)picture.data(), picture.size());

        sql << "INSERT INTO people(id, name, age, grade) VALUES(:id, :name, :age, :grade)",
            soci::use(id), soci::use(name), soci::use(age), soci::use(grade);
    }

public:
    soci::session sql;
private:
    std::shared_ptr<ProxyBackend> _backend;
};

TEST_F(SociProxyTest, ShouldDisplayProxy) {
    EXPECT_EQ(sql.get_backend_name(), "proxy");
}

TEST_F(SociProxyTest, ShouldCreateTable) {
    createTables(sql); // Just expect not to throw anything
}

TEST_F(SociProxyTest, ProcedureShouldCallableMultipleTime) {
    createTables(sql);
    int i;
    std::string str;
    soci::statement st = (sql.prepare <<
                                "insert into item(name, quantity) values(:val, :val2)",
            soci::use(i), soci::use(str));
    for (i = 0; i != 3; ++i)
    {
        st.execute(true);
    }
}

TEST_F(SociProxyTest, InsertAndSelectSimpleType) {
    createTables(sql);

    const int expected_id {1};
    const std::string expected_name {"John Doe"};
    const int64_t expected_age {42};
    const double expected_grade {0.5782910293847};
    const std::vector<uint8_t> expected_picture {};

    insertPeople(sql, expected_id, expected_name, expected_age, expected_grade, expected_picture);

    std::string name;
    int64_t age;
    double grade;

    sql << "SELECT name, age, grade FROM people WHERE id = :id", soci::use(expected_id), soci::into(name),
                                                                 soci::into(age), soci::into(grade);
    EXPECT_EQ(age, expected_age);
    EXPECT_EQ(name, expected_name);
    EXPECT_EQ(grade, expected_grade);
}