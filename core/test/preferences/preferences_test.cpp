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
#include <ledger/core/utils/Option.hpp>
#include <NativePathResolver.hpp>
#include <fstream>
#include <OpenSSLRandomNumberGenerator.hpp>

class PreferencesTest : public ::testing::Test {
protected:
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<NativeThreadDispatcher> dispatcher;
    std::shared_ptr<ledger::core::api::Lock> preferencesLock;
    std::shared_ptr<ledger::core::PreferencesBackend> backend;

    void TearDown() override {
        backend->clear();
    }

public:
    PreferencesTest()
        : resolver(std::make_shared<NativePathResolver>())
        , dispatcher(std::make_shared<NativeThreadDispatcher>())
        , preferencesLock(dispatcher->newLock())
        , backend(std::make_shared<ledger::core::PreferencesBackend>(
                    "/preferences/tests.db",
                    dispatcher->getSerialExecutionContext("worker"),
                    resolver
          )) {
    }
};

TEST_F(PreferencesTest, StoreAndGetWithPreferencesAPI) {
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

    dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([=]() {
        dispatcher->stop();
        FAIL() << "Timeout";
    }), 5000);

    dispatcher->waitUntilStopped();
    resolver->clean();
}

TEST_F(PreferencesTest, IterateThroughMembers) {
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

    dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([=]() {
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

TEST_F(PreferencesTest, IterateThroughObjectMembers) {
    auto preferences = backend->getPreferences("my_test_preferences_array");
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

    dispatcher->getSerialExecutionContext("toto")->delay(make_runnable([=]() {
        dispatcher->stop();
        FAIL() << "Timeout";
    }), 5000);

    dispatcher->waitUntilStopped();
    resolver->clean();
}

TEST_F(PreferencesTest, EncryptDecrypt) {
    auto preferences = backend->getPreferences("encrypt_decrypt");
    auto rng = std::make_shared<OpenSSLRandomNumberGenerator>();
    auto password = std::string("v3ry_secr3t_p4sSw0rD");
    auto string_array = std::vector<std::string>{ "foo", "bar", "zoo" };

    backend->setEncryption(rng, password);

    // persist a few stuff, that should be encrypted
    preferences
        ->editor()
        ->putString("string", "dawg")
        ->putInt("int", 9246)
        ->putLong("long", 89356493564)
        ->putBoolean("bool", true)
        ->putStringArray("string_array", { "foo", "bar", "zoo" })
        ->commit();

    ASSERT_EQ(preferences->getString("string", ""), "dawg");
    ASSERT_EQ(preferences->getInt("int", 0), 9246);
    ASSERT_EQ(preferences->getLong("long", 0), 89356493564);
    ASSERT_EQ(preferences->getStringArray("string_array", std::vector<std::string>()), string_array);

    // two things to test:
    //   1. disabling encryption should give us back unreadable data
    //   2. re-enabling encryption; the data should now be correctly read

    // (1.)
    backend->unsetEncryption();

    // boolean is not tested because its entropy is way too low
    ASSERT_NE(preferences->getString("string", ""), "dawg");
    ASSERT_NE(preferences->getInt("int", 0), 9246);
    ASSERT_NE(preferences->getLong("long", 0), 89356493564);

    // encoding is fucked up, so we need to manually check bytes and that the strings are not there
    auto garbage = preferences->getData("string_array", {});
    for (auto& s : string_array) {
        ASSERT_TRUE(std::search(garbage.cbegin(), garbage.cend(), s.cbegin(), s.cend()) == garbage.cend());
    }

    // (2.)
    backend->setEncryption(rng, password);

    ASSERT_EQ(preferences->getString("string", ""), "dawg");
    ASSERT_EQ(preferences->getInt("int", 0), 9246);
    ASSERT_EQ(preferences->getLong("long", 0), 89356493564);
    ASSERT_EQ(preferences->getStringArray("string_array", std::vector<std::string>()), string_array);

    // finally, we want to test that setting the same key to the same value twice gets two different
    // ciphered values
    preferences->editor()->putString("same", "ledger_and_biscuits")->commit();

    backend->unsetEncryption();
    auto cipherText1 = preferences->getString("same", "");

    backend->setEncryption(rng, password);
    preferences->editor()->putString("same", "ledger_and_biscuits")->commit();

    backend->unsetEncryption();
    auto cipherText2 = preferences->getString("same", "");

    ASSERT_NE(cipherText1, cipherText2);
}

TEST_F(PreferencesTest, ResetEncryption) {
    auto preferences = backend->getPreferences("reset_encryption");
    auto rng = std::make_shared<OpenSSLRandomNumberGenerator>();
    auto password = std::string("v3ry_secr3t_p4sSw0rD");

    backend->setEncryption(rng, password);

    preferences
        ->editor()
        ->putString("string", "dawg")
        ->putInt("int", 9246)
        ->putLong("long", 89356493564)
        ->commit();

    ASSERT_EQ(preferences->getString("string", ""), "dawg");
    ASSERT_EQ(preferences->getInt("int", 0), 9246);
    ASSERT_EQ(preferences->getLong("long", 0), 89356493564);

    auto reset = backend->resetEncryption(rng, password, "new password!");

    ASSERT_TRUE(reset);
    ASSERT_EQ(preferences->getString("string", ""), "dawg");
    ASSERT_EQ(preferences->getInt("int", 0), 9246);
    ASSERT_EQ(preferences->getLong("long", 0), 89356493564);
}

TEST_F(PreferencesTest, Clear) {
    auto preferences = backend->getPreferences("clear");

    preferences->editor()->putString("string", "dawg")->commit();
    EXPECT_EQ(preferences->getString("string", ""), "dawg");

    preferences->editor()->clear();
    EXPECT_EQ(preferences->getString("string", "none"), "none");
}

// This test checks that when setting encryption on, already present values are encrypted as well
// so that we can correctly retrieve them after encryption is set.
TEST_F(PreferencesTest, RecryptClearValues) {
    auto preferences = backend->getPreferences("recrypt_clear_values");
    auto rng = std::make_shared<OpenSSLRandomNumberGenerator>();
    auto password = std::string("v3ry_secr3t_p4sSw0rD");

    // add a clear record and then turn encryption on
    preferences->editor()->putString("string", "dawg")->commit();
    backend->setEncryption(rng, password);

    // now, reading the old value should be okay, too
    EXPECT_EQ(preferences->getString("string", "none"), "dawg");
}
