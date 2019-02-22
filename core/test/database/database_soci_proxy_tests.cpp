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

#include <database/ProxyBackend.hpp>
#include <gtest/gtest.h>
#include <soci.h>
#include "MemoryDatabaseProxy.h"
#include <fmt/format.h>
#include <list>
#include <algorithm>
#include <cmath>
#include <array>
#include <utils/DateUtils.hpp>
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
    long long age;
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

static const std::string DB_KEY = "test_key";
static const std::string DB_NEW_KEY = "test_new_key";
class SociProxyBaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        Test::SetUp();
        auto engine = std::make_shared<MemoryDatabaseProxy>();
        _backend = std::make_shared<ProxyBackend>(engine);
        _backend->enableQueryLogging(true);
        auto date = DateUtils::toJSON(std::chrono::system_clock::now());
        dbName = "test_db_" + date;
        _backend->init(nullptr, dbName, DB_KEY, sql);
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
        if (!people.picture.empty()) {
            soci::blob blob(sql);
            blob.write(0, (const char *)people.picture.data(), people.picture.size());
            sql << "INSERT INTO people(id, name, age, grade, picture) VALUES(:id, :name, :age, :grade, :picture)",
                    soci::use(people.id), soci::use(people.name), soci::use(people.age), soci::use(people.grade),
                    soci::use(blob);
        } else {
            sql << "INSERT INTO people(id, name, age, grade) VALUES(:id, :name, :age, :grade)",
                    soci::use(people.id), soci::use(people.name), soci::use(people.age), soci::use(people.grade);
        }
        for (const auto& i : people.items) {
            insertItem(sql, i);
        }
    }

    void insertItem(soci::session& sql, const Item& item) {
        sql << "INSERT INTO item(name, quantity, owner_id) VALUES(:name, :quantity, :owner_id)", soci::use(item.name),
            soci::use(item.quantity), soci::use(item.owner);
    }

    std::vector<uint8_t> blob_to_vector(soci::blob& blob) {
        std::vector<uint8_t> out;
        std::array<uint8_t, 2> buffer;
        std::size_t offset = 0;
        begin:
        auto r = blob.read(offset, (char *)buffer.data(), buffer.size());
        if (r > 0) {
            out.insert(out.end(), buffer.begin(), buffer.begin() + r);
            offset += r;
            goto begin;
        }
        return out;
    }

    std::vector<People> generateData(int count, bool withPicture = false) {
        int64_t Long = 0xDEADBEEFC0FFEL; // Random sentence
        int Int = 1337; // Because 1337 is always random
        uint8_t Byte = 0x0F;
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
            // Generate the picture
            if (withPicture) {
                auto pictureLen = 10 + RAND(Int) % 200;
                while (pictureLen > 0) {
                    p.picture.push_back(RAND(Byte));
                    pictureLen -= 1;
                }
            }
            people.emplace_back(p);
            count = count - 1;
        }
        return people;
    }

    void changePassword(const std::string &oldPassword, const std::string &newPassword) {
        _backend->changePassword(oldPassword, newPassword, sql);
    }

public:
    soci::session sql;
    std::string dbName;
protected:
    std::shared_ptr<ProxyBackend> _backend;
};

class SociProxyTest : public SociProxyBaseTest {
protected:
    void SetUp() override {
        Test::SetUp();
        auto engine = std::make_shared<MemoryDatabaseProxy>();
        _backend = std::make_shared<ProxyBackend>(engine);
        _backend->enableQueryLogging(true);
        dbName = ":memory:";
        _backend->init(nullptr, dbName, DB_KEY, sql);
    }
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
        retrieved.age = row.get<long long>(2);
        retrieved.grade = row.get<double>(3);
        EXPECT_EQ(expected.name, retrieved.name);
        EXPECT_EQ(expected.age, retrieved.age);
        EXPECT_EQ(expected.grade, retrieved.grade);
        from_db.emplace_back(retrieved);
        it++;
    }
    EXPECT_EQ(people.size(), from_db.size());
}

TEST_F(SociProxyTest, SelectNullField) {
    createTables(sql);
    auto people = generateData(1);
    insertPeople(sql, people);

    soci::indicator ind_name, ind_picture;
    std::string name;
    soci::blob picture(sql);

    sql << "SELECT name, picture FROM people", soci::into(name, ind_name), soci::into(picture, ind_picture);
    EXPECT_EQ(ind_picture, soci::i_null);
    EXPECT_EQ(ind_name, soci::i_ok);
    EXPECT_EQ(name, people.front().name);
}

TEST_F(SociProxyTest, InsertAndGetBlobs) {
    createTables(sql);
    auto people = generateData(1, true);
    people[0].items.clear();
    insertPeople(sql, people[0]);

    std::string name;
    soci::blob picture(sql);

    sql << "SELECT name, picture FROM people", soci::into(name), soci::into(picture);
    EXPECT_EQ(name, people.front().name);
    EXPECT_EQ(blob_to_vector(picture), people.front().picture);
}

TEST_F(SociProxyTest, SelectAllFields) {
    createTables(sql);
    auto people = generateData(1, true);
    insertPeople(sql, people);
    auto& witness = people[0];
    witness.items.clear();

    soci::rowset<soci::row> rows = (sql.prepare << "SELECT * FROM people WHERE age = :age AND name = :name AND grade = :grade AND id = :id",
            soci::use(witness.age), soci::use(witness.name), soci::use(witness.grade), soci::use(witness.id));
    auto& row = *rows.begin();
    People retrieved;
    bool hasPicture = false;
    for (size_t i = 0; i < row.size(); i++) {
        auto prop_name = row.get_properties(i).get_name();
        if (prop_name == "name")
            retrieved.name = row.get<std::string>(i);
        else if (prop_name == "age")
            retrieved.age = row.get<long long>(i);
        else if (prop_name == "grade")
            retrieved.grade = row.get<double>(i);
        else if (prop_name == "id")
            retrieved.id = row.get<int>(i);
        else if (prop_name == "picture") {
            // Weird behaviour of SOCI regarding BLOB in dynamic queries. Backends reference BLOB type to STRING type.
            // Furthermore there
            hasPicture = true;
            witness.picture.clear();
        }
    }
    EXPECT_EQ(witness.name, retrieved.name);
    EXPECT_EQ(witness.age, retrieved.age);
    EXPECT_EQ(witness.grade, retrieved.grade);
    EXPECT_EQ(witness.id, retrieved.id);
    EXPECT_TRUE(hasPicture);
}

TEST_F(SociProxyBaseTest, ChangePassword) {
    createTables(sql);
    auto people = generateData(1, true);
    people[0].items.clear();
    insertPeople(sql, people[0]);

    std::string name;
    soci::blob picture(sql);

    sql << "SELECT name, picture FROM people", soci::into(name), soci::into(picture);
    EXPECT_EQ(name, people.front().name);
    EXPECT_EQ(blob_to_vector(picture), people.front().picture);

    changePassword(DB_KEY, DB_NEW_KEY);

    sql << "SELECT name, picture FROM people", soci::into(name), soci::into(picture);
    EXPECT_EQ(name, people.front().name);
    EXPECT_EQ(blob_to_vector(picture), people.front().picture);
}

TEST_F(SociProxyTest, SelectCountWhenEmpty) {
    createTables(sql);
    int count;
    sql << "SELECT COUNT(*) FROM people WHERE name = 'paul'", soci::into(count);
    EXPECT_EQ(count, 0);
}

TEST_F(SociProxyTest, SelectCount) {
    createTables(sql);
    auto people = generateData(10, false);
    insertPeople(sql, people);
    int count;
    sql << "SELECT COUNT(*) FROM people", soci::into(count);
    EXPECT_EQ(count, people.size());
}

TEST_F(SociProxyTest, SelectRowsWhenEmpty) {
    createTables(sql);
    soci::rowset<soci::row> rows (sql.prepare << "SELECT * FROM PEOPLE");
    int count = 0;
    for (auto& row : rows) {
        count += 1;
    }
    EXPECT_EQ(count, 0);
}