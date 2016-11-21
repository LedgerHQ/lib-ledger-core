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

ledger::core::AtomicPreferencesBackend::AtomicPreferencesBackend(const std::string &absolutePath,
                                                                 const std::shared_ptr<ledger::core::api::ExecutionContext> &ownerContext,
                                                                 const std::shared_ptr<ledger::core::api::PathResolver> &pathResolver,
                                                                 const std::shared_ptr<ledger::core::api::Lock> &lock)
        : IPreferencesBackend(absolutePath, ownerContext, pathResolver, lock) {

}

void ledger::core::AtomicPreferencesBackend::load(std::function<void()> callback) {

}

std::shared_ptr<ledger::core::api::Preferences>
ledger::core::AtomicPreferencesBackend::getPreferences(const std::string &name) {
    return nullptr;
}

void ledger::core::AtomicPreferencesBackend::save(std::vector<ledger::core::PreferencesChanges> changes) {

}

ledger::core::LockedResource<rapidjson::Value::Object>
ledger::core::AtomicPreferencesBackend::getObject(const std::string &name) {
    _lock->lock();
    auto object = _dom.GetObject().FindMember(name.c_str());
    if (object->value.IsNull()) {
        object->value.SetObject();
    }
    _lock->unlock();
    return LockedResource<rapidjson::Value::Object>(_lock, object->value.GetObject());
}

