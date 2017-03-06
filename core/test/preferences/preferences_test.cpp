/*
 *
 * json_preferences_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/11/2016.
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

#include <gtest/gtest.h>
#include <EventLooper.hpp>
#include <EventThread.hpp>
#include <NativeThreadDispatcher.hpp>
#include <ledger/core/preferences/PreferencesBackend.hpp>
#include <NativePathResolver.hpp>
#include <fstream>

TEST(Preferences, StoreAndGetWithPreferencesAPI) {
    auto resolver = std::make_shared<NativePathResolver>();
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto preferencesLock = dispatcher->newLock();
    auto backend = std::make_shared<ledger::core::PreferencesBackend>(
            "/preferences/tests.db",
            dispatcher->getSerialExecutionContext("worker"),
            resolver
    );
    auto preferences = backend->getPreferences("my_test_preferences");
    dispatcher->getSerialExecutionContext("not_my_worker")->execute(make_runnable([=] () {
        preferences->edit()
                ->putString("my_string", "Hello World!")
                ->putBoolean("my_bool", true)
                ->putInt("my_int", 42)
                ->putLong("my_long", 42LL << 42LL)
                ->putStringArray("my_string_array", std::vector<std::string>({"Hello", "world", "!"}))
                ->commit();
    }));
    dispatcher->getSerialExecutionContext("worker")->execute(make_runnable([=] () {
        preferences
                ->edit()
                ->putString("my_worker_string", "Hey World!")
                ->putString("my_string_to_remove", "Remove this please!")
                ->commit();
    }));
    // Assume that 100ms should be enough to persist data
    dispatcher->getMainExecutionContext()->delay(make_runnable([=] () {
        EXPECT_EQ(preferences->getString("my_string", ""), "Hello World!");
        EXPECT_EQ(preferences->getString("my_worker_string", ""), "Hey World!");
        EXPECT_EQ(preferences->getString("my_fake_string", "Not My String"), "Not My String");
        EXPECT_EQ(preferences->getString("my_string_to_remove", ""), "Remove this please!");
        EXPECT_EQ(preferences->getInt("my_int", -1), 42);
        EXPECT_EQ(preferences->getLong("my_long", -1),  42LL << 42LL);
        EXPECT_EQ(preferences->getBoolean("my_bool", false), true);
        EXPECT_EQ(preferences->getStringArray("my_string_array", {}), std::vector<std::string>({"Hello", "world", "!"}));
        preferences->edit()->remove("my_string_to_remove")->commit();
        dispatcher->getSerialExecutionContext("worker")->delay(make_runnable([=] () {
            EXPECT_EQ(preferences->getString("my_string_to_remove", "Removed"), "Removed");
            dispatcher->stop();
        }), 50);
    }), 100);
    dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([dispatcher]() {
        dispatcher->stop();
        FAIL() << "Timeout";
    }), 5000);

    dispatcher->waitUntilStopped();
    resolver->clean();
}

TEST(Preferences, IterateThroughMembers) {
    auto resolver = std::make_shared<NativePathResolver>();
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto preferencesLock = dispatcher->newLock();
    auto backend = std::make_shared<ledger::core::PreferencesBackend>(
            "/preferences/tests.db",
            dispatcher->getSerialExecutionContext("worker"),
            resolver
    );
    auto preferences = backend->getPreferences("my_test_preferences");
    auto otherPreferences = backend->getPreferences("my_other_test_preferences");
    dispatcher->getSerialExecutionContext("not_my_worker")->execute(make_runnable([=] () {
        preferences->edit()
                ->putString("address:0", "Hello World!")
                ->putString("address:1", "Hello World!")
                ->putString("address:2", "Hello World!")
                ->putString("address:3", "Hello World!")
                ->putString("address:4", "Hello World!")
                ->putString("address:5", "Hello World!")
                ->putString("address:6", "Hello World!")
                ->putString("address:7", "Hello World!")
                ->putString("address:8", "Hello World!")
                ->putString("address:9", "Hello World!")
                ->putString("address:10", "Hello World!")
                ->putString("address:16", "Hello World!")
                ->putString("notaddress:0", "Hello World!")
                ->putString("notaddress:1", "Hello World!")
                ->commit();
        otherPreferences->edit()
                ->putString("address:0", "Hello World!")
                ->putString("address:1", "Hello World!")
                ->commit();
    }));
    dispatcher->getSerialExecutionContext("worker")->execute(make_runnable([=] () {
        preferences
                ->edit()
                ->putString("address:11", "Hello World!")
                ->putString("address:12", "Hello World!")
                ->putString("address:13", "Hello World!")
                ->putString("address:14", "Hello World!")
                ->putString("address:15", "Hello World!")
                ->commit();
    }));
    // Assume that 100ms should be enough to persist data
    dispatcher->getMainExecutionContext()->delay(make_runnable([=] () {
        std::set<std::string> addresses({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                                         "10", "11", "12", "13", "14", "15", "16"});
        preferences->iterate([addresses] (leveldb::Slice&& key, leveldb::Slice&& value) {
            EXPECT_NE(addresses.find(key.ToString()), addresses.end());
            return true;
        }, ledger::core::Option<std::string>("address:"));
        dispatcher->stop();
    }), 100);
    dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([dispatcher]() {
        dispatcher->stop();
        FAIL() << "Timeout";
    }), 5000);

    dispatcher->waitUntilStopped();
    resolver->clean();
}

struct MyClass
{
    int x, y, z;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive( x, y, z );
    }
};
TEST(Preferences, IterateThroughObjectMembers) {
    auto resolver = std::make_shared<NativePathResolver>();
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto preferencesLock = dispatcher->newLock();
    auto backend = std::make_shared<ledger::core::PreferencesBackend>(
    "/preferences/tests.db",
    dispatcher->getSerialExecutionContext("worker"),
    resolver
    );
    auto preferences = backend->getPreferences("my_test_preferences");
    auto otherPreferences = backend->getPreferences("my_other_test_preferences");

    MyClass obj1;
    obj1.x = 06;
    obj1.y = 03;
    obj1.z = 2017;

    MyClass obj2;
    obj2.x = 07;
    obj2.y = 03;
    obj2.z = 2017;

    dispatcher->getSerialExecutionContext("worker")->execute(make_runnable([&] () {
        preferences
        ->editor()
        ->putObject<MyClass>("0", obj1)
        ->putObject<MyClass>("1", obj2)
        ->commit();
    }));
    // Assume that 100ms should be enough to persist data
    dispatcher->getMainExecutionContext()->delay(make_runnable([&] () {
        preferences->iterate<MyClass>([=] (leveldb::Slice&& key, const MyClass& value) {
            if (key.ToString() == "0") {
                EXPECT_EQ(obj1.x, value.x);
                EXPECT_EQ(obj1.y, value.y);
                EXPECT_EQ(obj1.z, value.z);
            } else if (key.ToString() == "1") {
                EXPECT_EQ(obj2.x, value.x);
                EXPECT_EQ(obj2.y, value.y);
                EXPECT_EQ(obj2.z, value.z);
            } else {
               EXPECT_TRUE(false);
            }
            return true;
        });
        dispatcher->stop();
    }), 100);
    dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([dispatcher]() {
        dispatcher->stop();
        FAIL() << "Timeout";
    }), 5000);

    dispatcher->waitUntilStopped();
    resolver->clean();
}
