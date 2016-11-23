/*
 *
 * JsonPreferences
 * ledger-core
 *
 * Created by Pierre Pollastri on 17/11/2016.
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
#include "JsonPreferences.hpp"
#include "IPreferencesBackend.hpp"
#include "JsonPreferencesEditor.hpp"

std::string ledger::core::JsonPreferences::getString(const std::string &key, const std::string &fallbackValue) {
    auto backend = _backend.lock();
    auto object = backend->getObject(_name);
    auto itr = object->FindMember(key.c_str());
    if (itr == object->MemberEnd() || !itr->value.IsString())
        return fallbackValue;
    return itr->value.GetString();
}

int32_t ledger::core::JsonPreferences::getInt(const std::string &key, int32_t fallbackValue) {
    auto backend = _backend.lock();
    auto object = backend->getObject(_name);
    auto itr = object->FindMember(key.c_str());
    if (itr == object->MemberEnd() || !itr->value.IsInt())
        return fallbackValue;
    return itr->value.GetInt();
}

int64_t ledger::core::JsonPreferences::getLong(const std::string &key, int64_t fallbackValue) {
    auto backend = _backend.lock();
    auto object = backend->getObject(_name);
    auto itr = object->FindMember(key.c_str());
    if (itr == object->MemberEnd() || !itr->value.IsInt64())
        return fallbackValue;
    return itr->value.GetInt64();
}

bool ledger::core::JsonPreferences::getBoolean(const std::string &key, bool fallbackValue) {
    auto backend = _backend.lock();
    auto object = backend->getObject(_name);
    auto itr = object->FindMember(key.c_str());
    if (itr == object->MemberEnd() || !itr->value.IsBool())
        return fallbackValue;
    return itr->value.GetBool();
}

std::vector<std::string>
ledger::core::JsonPreferences::getStringArray(const std::string &key, const std::vector<std::string> &fallbackValue) {
    auto backend = _backend.lock();
    auto object = backend->getObject(_name);
    auto itr = object->FindMember(key.c_str());
    if (itr == object->MemberEnd() || !itr->value.IsArray())
        return fallbackValue;
    auto array = itr->value.GetArray();
    std::vector<std::string> result;
    for (auto& v : array) {
        if (v.IsString()) {
            result.push_back(v.GetString());
        }
    }
    return result;
}

bool ledger::core::JsonPreferences::contains(const std::string &key) {
    auto backend = _backend.lock();
    auto object = backend->getObject(_name);
    auto itr = object->FindMember(key.c_str());
    return itr != object->MemberEnd();
}

std::shared_ptr<ledger::core::api::PreferencesEditor> ledger::core::JsonPreferences::edit() {
    return std::make_shared<JsonPreferencesEditor>(_backend, _name, _allocator);
}

ledger::core::JsonPreferences::JsonPreferences(std::weak_ptr<ledger::core::IPreferencesBackend> backend,
                                               std::string name, rapidjson::MemoryPoolAllocator<> &allocator) : _allocator(allocator) {
    _name = name;
    _backend = backend;
}
