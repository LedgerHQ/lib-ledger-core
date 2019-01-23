/*
 *
 * dynamics_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 09/03/2017.
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
#include <ledger/core/api/DynamicArray.hpp>
#include <ledger/core/api/DynamicObject.hpp>
#include <ledger/core/collections/DynamicValue.hpp>
#include <ledger/core/utils/optional.hpp>

using namespace ledger::core::api;

TEST(Dynamics, Value) {
    int64_t i64 = 9364936249LL;

    EXPECT_EQ(*ledger::core::DynamicValue(std::string("foo")).asStr(), "foo");
    EXPECT_EQ(*ledger::core::DynamicValue(3946).asInt32(), 3946);
    EXPECT_EQ(*ledger::core::DynamicValue(i64).asInt64(), i64);
    EXPECT_EQ(*ledger::core::DynamicValue(3.141592F).asDouble(), 3.141592F);
    EXPECT_EQ(*ledger::core::DynamicValue(true).asBool(), true);
}

TEST(Dynamics, ValueSerialization) {
    // serialize
    std::stringstream is;
    ::cereal::PortableBinaryOutputArchive oarchive(is);
    oarchive(ledger::core::DynamicValue(std::string("foobarzoo")));

    // deserialize
    ledger::core::DynamicValue v;
    ::cereal::PortableBinaryInputArchive iarchive(is);
    iarchive(v);

    EXPECT_EQ(*v.asStr(), "foobarzoo");
}

TEST(Dynamics, Array) {
    auto array = DynamicArray::newInstance();
    array
    ->pushBoolean(true)
    ->pushDouble(12.6)
    ->pushString("Hello World")
    ->pushInt(12)
    ->pushLong(16)
    ->pushData({0x09, 0x03});

    EXPECT_EQ(array->size(), 6);
    EXPECT_EQ(array->getBoolean(0).value(), true);
    EXPECT_EQ(array->getDouble(1).value(), 12.6);
    EXPECT_EQ(array->getString(2).value(), "Hello World");
    EXPECT_EQ(array->getInt(3).value(), 12);
    EXPECT_EQ(array->getLong(4).value(), 16);
    EXPECT_EQ(array->getData(5).value(), std::vector<uint8_t>({0x09, 0x03}));

    array->remove(2);
    EXPECT_EQ(array->getBoolean(0).value(), true);
    EXPECT_EQ(array->getDouble(1).value(), 12.6);
    EXPECT_EQ(array->getInt(2).value(), 12);
    EXPECT_EQ(array->getLong(3).value(), 16);
    EXPECT_EQ(array->getData(4).value(), std::vector<uint8_t>({0x09, 0x03}));
}

TEST(Dynamics, ArraySerialization) {
    std::vector<uint8_t> serialized;
    {
        auto array = DynamicArray::newInstance();
        array
        ->pushBoolean(true)
        ->pushDouble(12.6)
        ->pushString("Hello World")
        ->pushInt(12)
        ->pushLong(16)
        ->pushData({0x09, 0x03});
        serialized = array->serialize();
    }
    {
        auto array = DynamicArray::load(serialized);
        EXPECT_EQ(array->size(), 6);
        EXPECT_EQ(array->getBoolean(0).value(), true);
        EXPECT_EQ(array->getDouble(1).value(), 12.6);
        EXPECT_EQ(array->getString(2).value(), "Hello World");
        EXPECT_EQ(array->getInt(3).value(), 12);
        EXPECT_EQ(array->getLong(4).value(), 16);
        EXPECT_EQ(array->getData(5).value(), std::vector<uint8_t>({0x09, 0x03}));

        std::cout << array->dump() << std::endl;
    }
}

TEST(Dynamics, Object) {
    auto object = DynamicObject::newInstance();
    object
        ->putBoolean("boolean", true)
        ->putDouble("double", 12.6)
        ->putString("string", "Hello World")
        ->putInt("int", 12)
        ->putLong("long", 16)
        ->putData("data", {0x09, 0x03});
    EXPECT_EQ(object->size(), 6);
    EXPECT_EQ(object->getBoolean("boolean").value(), true);
    EXPECT_EQ(object->getDouble("double").value(), 12.6);
    EXPECT_EQ(object->getString("string").value(), "Hello World");
    EXPECT_EQ(object->getInt("int").value(), 12);
    EXPECT_EQ(object->getLong("long").value(), 16);
    EXPECT_EQ(object->getData("data").value(), std::vector<uint8_t>({0x09, 0x03}));
    std::cout << object->dump() << std::endl;
}

TEST(Dynamics, ObjectSerialization) {
    std::vector<uint8_t> serialized;
    {
        auto object = DynamicObject::newInstance();
        object
        ->putBoolean("boolean", true)
        ->putDouble("double", 12.6)
        ->putString("string", "Hello World")
        ->putInt("int", 12)
        ->putLong("long", 16)
        ->putData("data", {0x09, 0x03});
       serialized = object->serialize();
    }
    {
        auto object = DynamicObject::load(serialized);
        object
        ->putBoolean("boolean", true)
        ->putDouble("double", 12.6)
        ->putString("string", "Hello World")
        ->putInt("int", 12)
        ->putLong("long", 16)
        ->putData("data", {0x09, 0x03});
        EXPECT_EQ(object->size(), 6);
        EXPECT_EQ(object->getBoolean("boolean").value(), true);
        EXPECT_EQ(object->getDouble("double").value(), 12.6);
        EXPECT_EQ(object->getString("string").value(), "Hello World");
        EXPECT_EQ(object->getInt("int").value(), 12);
        EXPECT_EQ(object->getLong("long").value(), 16);
        EXPECT_EQ(object->getData("data").value(), std::vector<uint8_t>({0x09, 0x03}));
        std::cout << object->dump() << std::endl;
    }
}

TEST(Dynamics, ArrayWithObjects) {
    std::vector<uint8_t> serialized;
    {
        auto array = DynamicArray::newInstance();
        auto object = DynamicObject::newInstance();
        object
        ->putBoolean("boolean", true)
        ->putDouble("double", 12.6)
        ->putString("string", "Hello World")
        ->putInt("int", 12)
        ->putLong("long", 16)
        ->putData("data", {0x09, 0x03});
        array->pushBoolean(false);
        array->pushObject(object);
        serialized = array->serialize();
    }
    {
        auto array = DynamicArray::load(serialized);
        auto object = array->getObject(1);
        EXPECT_EQ(object->size(), 6);
        EXPECT_EQ(object->getBoolean("boolean").value(), true);
        EXPECT_EQ(object->getDouble("double").value(), 12.6);
        EXPECT_EQ(object->getString("string").value(), "Hello World");
        EXPECT_EQ(object->getInt("int").value(), 12);
        EXPECT_EQ(object->getLong("long").value(), 16);
        EXPECT_EQ(object->getData("data").value(), std::vector<uint8_t>({0x09, 0x03}));
        std::cout << array->dump() << std::endl;
    }
}

TEST(Dynamics, ObjectWithArray) {
    std::vector<uint8_t> serialized;
    {
        auto object = DynamicObject::newInstance();
        auto array = DynamicArray::newInstance();
        array
        ->pushBoolean(true)
        ->pushDouble(12.6)
        ->pushString("Hello World")
        ->pushInt(12)
        ->pushLong(16)
        ->pushData({0x09, 0x03});
        object->putArray("array", array);
        serialized = object->serialize();
    }
    {
        auto object = DynamicObject::load(serialized);
        auto array = object->getArray("array");
        EXPECT_EQ(array->size(), 6);
        EXPECT_EQ(array->getBoolean(0).value(), true);
        EXPECT_EQ(array->getDouble(1).value(), 12.6);
        EXPECT_EQ(array->getString(2).value(), "Hello World");
        EXPECT_EQ(array->getInt(3).value(), 12);
        EXPECT_EQ(array->getLong(4).value(), 16);
        EXPECT_EQ(array->getData(5).value(), std::vector<uint8_t>({0x09, 0x03}));
        std::cout << object->dump() << std::endl;
    }
}
