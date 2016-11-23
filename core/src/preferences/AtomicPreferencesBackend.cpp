/*
 *
 * AtomicPreferencesBackend
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/11/2016.
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
#include <bigd.h>
#include <functional>
#include "AtomicPreferencesBackend.hpp"
#include "../utils/LambdaRunnable.hpp"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include "JsonPreferences.hpp"
#include <fstream>
#include <rapidjson/writer.h>
#include <iostream>

ledger::core::AtomicPreferencesBackend::AtomicPreferencesBackend(const std::string &absolutePath,
                                                                 const std::shared_ptr<ledger::core::api::ExecutionContext> &ownerContext,
                                                                 const std::shared_ptr<ledger::core::api::PathResolver> &pathResolver,
                                                                 const std::shared_ptr<ledger::core::api::Lock> &lock)
        : IPreferencesBackend(absolutePath, ownerContext, pathResolver, lock) {

}

void ledger::core::AtomicPreferencesBackend::load(std::function<void()> callback) {
    auto self = shared_from_this();
    _context->execute(make_runnable([self, callback] () {
        auto path = self->_pathResolver->resolvePreferencesPath(self->_path);
        std::ifstream ifs(path);
        if (ifs.is_open()) {
            rapidjson::IStreamWrapper is(ifs);
            self->_dom.ParseStream(is);
        } else {
            self->_dom.SetObject();
        }
        callback();
    }));
}

std::shared_ptr<ledger::core::api::Preferences>
ledger::core::AtomicPreferencesBackend::getPreferences(const std::string &name) {
    return std::make_shared<JsonPreferences>(shared_from_this(), name, _dom.GetAllocator());
}

ledger::core::LockedResource<rapidjson::Value::Object>
ledger::core::AtomicPreferencesBackend::getObject(const std::string &name) {
    _lock->lock();
    if (_dom.GetObject().FindMember(name.c_str()) == _dom.MemberEnd()) {
        rapidjson::Value v(rapidjson::kObjectType);
        _dom.AddMember(rapidjson::StringRef(name.c_str()), v, _dom.GetAllocator());
    }
    auto object = _dom.GetObject().FindMember(name.c_str())->value.GetObject();
    _lock->unlock();
    return LockedResource<rapidjson::Value::Object>(_lock, object);
}

void ledger::core::AtomicPreferencesBackend::save(const std::string &name,
                                                  std::vector<ledger::core::PreferencesChanges *> changes) {
    auto self = shared_from_this();
    _context->execute(make_runnable([self, name, changes] () {
        self->merge(name, changes);
        auto finalPath = self->_pathResolver->resolvePreferencesPath(self->_path);
        auto path = self->_pathResolver->resolvePreferencesPath(self->_path + ".tmp");
        std::ofstream ofs(path);
        if (ofs.is_open()) {
            rapidjson::OStreamWrapper  os(ofs);
            rapidjson::Writer<rapidjson::OStreamWrapper> writer(os);
            self->_dom.Accept(writer);
            std::rename(path.c_str(), finalPath.c_str());
            ofs.close();
        } else {
            // TODO LOG ERROR
        }
    }));
}

void ledger::core::AtomicPreferencesBackend::merge(const std::string &name,
                                                   std::vector<ledger::core::PreferencesChanges *> changes) {
    _lock->lock();
    if (_dom.GetObject().FindMember(name.c_str()) == _dom.MemberEnd()) {
        rapidjson::Value v(rapidjson::kObjectType);
        _dom.AddMember(rapidjson::StringRef(name.c_str()), v, _dom.GetAllocator());
    }
    auto object = _dom.GetObject().FindMember(name.c_str())->value.GetObject();
    for (auto change : changes) {
        if (change->type == ledger::core::PreferencesChangeType::SET) {
            rapidjson::Value key(rapidjson::kStringType);
            key.SetString(rapidjson::StringRef(change->name.c_str()), change->name.size(), _dom.GetAllocator());
            auto itr = object.FindMember(key);
            if (itr == object.MemberEnd()) {
                object.AddMember(key, change->value, _dom.GetAllocator());
            } else {
                itr->value.Swap(change->value);
            }
        } else {
            object.RemoveMember(change->name.c_str());
        }
        delete change;
    }
    _lock->unlock();
}

