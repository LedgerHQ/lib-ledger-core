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
#include <fmt/format.h>
#include <list>
#include <algorithm>
#include <cmath>

using namespace ledger::core;

struct Item {
    std::string name;
    int64_t quantity;
    int owner;

    Item() {};
    Item(const std::string& n, int64_t q, int o) : name(n), quantity(q), owner(o) {};
};

struct People {
    int id;
    std::string name;
    int64_t age;
    double grade;
    std::vector<uint8_t> picture;
    std::list<Item> items;

    People() {};
    People(int i, const std::string& n, int64_t a, double g, const std::vector<uint8_t>& p) :
        id(i), name(n), age(a), grade(g), picture(p)
    {};
};

template <class T>
static T random(T seed) {
    return seed * 31415821 + 1; // Well that's random
}
#define RAND(var_name) (var_name = random(var_name))

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

    void insertPeople(soci::session& sql, const std::vector<People>& people) {
        for (const auto& p : people) {
            insertPeople(sql, p);
        }
    }

    void insertPeople(soci::session& sql, const People& people) {
        //soci::blob blob(sql);
        //blob.write(0, (const char *)picture.data(), picture.size());

        sql << "INSERT INTO people(id, name, age, grade) VALUES(:id, :name, :age, :grade)",
            soci::use(people.id), soci::use(people.name), soci::use(people.age), soci::use(people.grade);
        for (const auto& i : people.items) {
            insertItem(sql, i);
        }
    }

    void insertItem(soci::session& sql, const Item& item) {
        sql << "INSERT INTO item(name, quantity, owner_id) VALUES(:name, :quantity, :owner_id)", soci::use(item.name),
            soci::use(item.quantity), soci::use(item.owner);
    }

    std::vector<People> generateData(int count) {
        int64_t Long = 0xDEADBEEFC0FFEL; // Random sentence
        int Int = 1337; // Because 1337 is always random
        int id = 1;
        double Double = 55.67891;
        std::vector<People> people;
        while (count > 0) {
            People p {id++, fmt::format("{}", RAND(Int)), RAND(Long), ::ceil((int)RAND(Double) % 3000000) / 100.00, {}};
            auto itemsCount = ((unsigned int) RAND(Int)) % 21;
            for (auto i = 0; i < itemsCount; i++) {
                Item item {fmt::format("{}", RAND(Double)), RAND(Long), p.id};
                p.items.emplace_back(item);
            }
            people.emplace_back(p);
            count = count - 1;
        }
        return people;
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

    insertPeople(sql, {expected_id, expected_name, expected_age, expected_grade, expected_picture});

    std::string name;
    int64_t age;
    double grade;

    sql << "SELECT name, age, grade FROM people WHERE id = :id", soci::use(expected_id), soci::into(name),
                                                                 soci::into(age), soci::into(grade);
    EXPECT_EQ(age, expected_age);
    EXPECT_EQ(name, expected_name);
    EXPECT_EQ(grade, expected_grade);
}

TEST_F(SociProxyTest, SelectWithJoin) {
    createTables(sql);
    auto people = generateData(1)[0];
    // Just ensure we only 1 item
    people.items.erase(++people.items.begin(), people.items.end());
    const auto& item = people.items.front();

    People p;
    Item i;

    insertPeople(sql, people);

    sql << "SELECT p.name, p.age, p.grade, i.name, i.quantity "
           "FROM people AS p JOIN item AS i ON i.owner_id = p.id "
           "WHERE id = :id", soci::use(people.id), soci::into(p.name), soci::into(p.age), soci::into(p.grade),
           soci::into(i.name), soci::into(i.quantity);

    EXPECT_EQ(people.name, p.name);
    EXPECT_EQ(people.grade, p.grade);
    EXPECT_EQ(people.age, p.age);
    EXPECT_EQ(item.name, i.name);
    EXPECT_EQ(item.quantity, i.quantity);
}

TEST_F(SociProxyTest, SelectWithRows) {
    createTables(sql);
    auto people = generateData(100);
    std::vector<People> from_db;
    std::sort(people.begin(), people.end(), [] (const People& a, const People& b) -> bool {
        return a.id < b.id;
    });
    insertPeople(sql, people);
    soci::rowset<soci::row> rows = (sql.prepare << "SELECT id, name, age, grade FROM people ORDER by id");
    auto it = people.begin();
    for (const auto& row : rows) {
        const auto& expected = *it;
        People retrieved;
        retrieved.id = row.get<int>(0);
        retrieved.name = row.get<std::string>(1);
        retrieved.age = row.get<int64_t>(2);
        retrieved.grade = row.get<double>(3);
        EXPECT_EQ(expected.name, retrieved.name);
        EXPECT_EQ(expected.age, retrieved.age);
        EXPECT_EQ(expected.grade, retrieved.grade);
        from_db.emplace_back(retrieved);
        it++;
    }
    EXPECT_EQ(people.size(), from_db.size());
}

TEST_F(SociProxyTest, GetEntryRowId) {

}

TEST_F(SociProxyTest, InsertAndGetBlobs) {

}