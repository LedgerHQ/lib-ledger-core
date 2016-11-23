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
#ifndef LEDGER_CORE_JSONPREFERENCES_HPP
#define LEDGER_CORE_JSONPREFERENCES_HPP

#include "../api/Preferences.hpp"
#include "../api/PathResolver.hpp"
#include "../api/ExecutionContext.hpp"
#include "../api/Lock.hpp"
#include <rapidjson/document.h>
#include <rapidjson/allocators.h>
#include <functional>
#include "IPreferencesBackend.hpp"

namespace ledger {
    namespace core {
        class JsonPreferences : public api::Preferences {
        public:
            JsonPreferences(
                    std::weak_ptr<IPreferencesBackend> backend,
                    std::string name,
                    rapidjson::MemoryPoolAllocator<>& allocator
            );
        private:
        public:
            virtual std::string getString(const std::string &key, const std::string &fallbackValue) override;

            virtual int32_t getInt(const std::string &key, int32_t fallbackValue) override;

            virtual int64_t getLong(const std::string &key, int64_t fallbackValue) override;

            virtual bool getBoolean(const std::string &key, bool fallbackValue) override;

            virtual std::vector<std::string>
            getStringArray(const std::string &key, const std::vector<std::string> &fallbackValue) override;

            virtual bool contains(const std::string &key) override;

            virtual std::shared_ptr<api::PreferencesEditor> edit() override;

            friend class IPreferencesBackend;
        private:
            std::weak_ptr<IPreferencesBackend> _backend;
            rapidjson::MemoryPoolAllocator<>& _allocator;
            std::string _name;
        };
    }
}

#endif //LEDGER_CORE_JSONPREFERENCES_HPP
