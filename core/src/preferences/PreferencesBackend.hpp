/*
 *
 * PreferencesBackend
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/01/2017.
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
#ifndef LEDGER_CORE_PREFERENCESBACKEND_HPP
#define LEDGER_CORE_PREFERENCESBACKEND_HPP

#include "../api/Preferences.hpp"
#include "../api/PreferencesEditor.hpp"
#include <leveldb/db.h>
#include <memory>
#include "../api/ThreadDispatcher.hpp"
#include "../api/ExecutionContext.hpp"
#include "../api/PathResolver.hpp"
#include "../api/Lock.hpp"
#include <string>
#include <functional>
#include "../utils/optional.hpp"
#include "Preferences.hpp"
#include <unordered_map>
#include <mutex>
#include <api/RandomNumberGenerator.hpp>

namespace ledger {
    namespace core {
        class Preferences;
        enum PreferencesChangeType {
            PUT_TYPE, DELETE_TYPE
        };

        struct PreferencesChange {
            PreferencesChangeType type;
            std::vector<uint8_t> key;
            std::vector<uint8_t> value;

            PreferencesChange() = default;

            PreferencesChange(PreferencesChangeType t, std::vector<uint8_t> k, std::vector<uint8_t> v)
                : type(t), key(k), value(v) {
            }
        };

        class PreferencesBackend {
        public:
            PreferencesBackend(
                const std::string& path,
                const std::shared_ptr<api::ExecutionContext>& writingContext,
                const std::shared_ptr<api::PathResolver>& resolver,
                std::shared_ptr<api::RandomNumberGenerator> rng,
                optional<const std::string&> password
            );

            ~PreferencesBackend() = default;

            std::shared_ptr<Preferences> getPreferences(const std::string& name);
            void iterate(const std::vector<uint8_t>& keyPrefix, std::function<bool (leveldb::Slice&&, leveldb::Slice&&)>);
            optional<std::string> get(const std::vector<uint8_t>& key) const;
            void commit(const std::vector<PreferencesChange>& changes);

        private:
            std::shared_ptr<api::ExecutionContext> _context;
            std::shared_ptr<leveldb::DB> _db;
            std::shared_ptr<api::RandomNumberGenerator> _rng;
            optional<std::string> _password; // password used to encrypt the database

            // helper method used to encrypt things we want to put in leveldb
            std::vector<uint8_t> encrypt_preferences_change(
                const std::string& password,
                const PreferencesChange& change
            ) const;

            // helper method used to decrypt things we want to retrieve from leveldb
            std::vector<uint8_t> decrypt_preferences_change(
                const std::string& password,
                const std::vector<uint8_t>& key,
                const std::vector<uint8_t>& data
            ) const;

            static std::unordered_map<std::string, std::weak_ptr<leveldb::DB>> LEVELDB_INSTANCE_POOL;
            static std::mutex LEVELDB_INSTANCE_POOL_MUTEX;

            static std::shared_ptr<leveldb::DB> obtainInstance(const std::string& path);
        };
    }
}


#endif //LEDGER_CORE_PREFERENCESBACKEND_HPP
