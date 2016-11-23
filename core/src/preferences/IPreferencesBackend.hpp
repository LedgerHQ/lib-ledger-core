/*
 *
 * IPreferencesBackend
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
#ifndef LEDGER_CORE_IPREFERENCESBACKEND_HPP
#define LEDGER_CORE_IPREFERENCESBACKEND_HPP

#include "../api/ExecutionContext.hpp"
#include "../api/Preferences.hpp"
#include "../api/PathResolver.hpp"
#include "../api/Lock.hpp"
#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include "../utils/optional.hpp"
#include "../async/LockedResource.hpp"

namespace ledger {
    namespace core {
        enum PreferencesChangeType {
            SET,
            REMOVE
        };

        struct PreferencesChanges {
            PreferencesChangeType type;
            std::string name;
            rapidjson::Value value;
        };

        class IPreferencesBackend {
        public:
            IPreferencesBackend(
                    const std::string& absolutePath,
                    const std::shared_ptr<api::ExecutionContext>& ownerContext,
                    const std::shared_ptr<api::PathResolver>& pathResolver,
                    const std::shared_ptr<api::Lock>& lock
            ) {
                _path = absolutePath;
                _context = ownerContext;
                _pathResolver = pathResolver;
                _lock = lock;
            };
            virtual void load(std::function<void()> callback) = 0;
            virtual std::shared_ptr<api::Preferences> getPreferences(const std::string& name) = 0;
            virtual void save(const std::string& name, std::vector<PreferencesChanges *> changes) = 0;
            virtual LockedResource<rapidjson::Value::Object> getObject(const std::string& name) = 0;
        protected:
            std::string _path;
            std::shared_ptr<api::ExecutionContext> _context;
            std::shared_ptr<api::PathResolver> _pathResolver;
            std::shared_ptr<api::Lock> _lock;
            rapidjson::Document _dom;
        };
    };
};

#endif //LEDGER_CORE_IPREFERENCESBACKEND_HPP
