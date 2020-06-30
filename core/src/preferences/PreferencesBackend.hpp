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
#include "../api/PreferencesBackend.hpp"
#include "../api/PreferencesChange.hpp"
#include <leveldb/db.h>
#include <memory>
#include "../api/ThreadDispatcher.hpp"
#include "../api/ExecutionContext.hpp"
#include "../api/PathResolver.hpp"
#include "../api/Lock.hpp"
#include <string>
#include <functional>
#include "../utils/optional.hpp"
#include <unordered_map>
#include <mutex>
#include <api/RandomNumberGenerator.hpp>
#include <utils/Option.hpp>
#include <crypto/AESCipher.hpp>

namespace ledger {
    namespace core {
        class Preferences;

        class PreferencesBackend : public api::PreferencesBackend {
        public:
            PreferencesBackend(
                const std::string& path,
                const std::shared_ptr<api::ExecutionContext>& writingContext,
                const std::shared_ptr<api::PathResolver>& resolver
            );

            std::shared_ptr<Preferences> getPreferences(const std::string& name);

            optional<std::string> get(const std::vector<uint8_t>& key) const override;

            bool commit(const std::vector<api::PreferencesChange>& changes) override;

            void setEncryption(
                const std::shared_ptr<api::RandomNumberGenerator>& rng,
                const std::string& password
            ) override;

            void unsetEncryption() override;

            bool resetEncryption(
                const std::shared_ptr<api::RandomNumberGenerator>& rng,
                const std::string& oldPassword,
                const std::string& newPassword
            ) override;

            std::string getEncryptionSalt() const override;

            void clear() override;

        private:
            std::shared_ptr<api::ExecutionContext> _context;
            std::weak_ptr<leveldb::DB> _db;
            std::string _dbName;
            Option<AESCipher> _cipher;

            // Get a raw entry from the key-value store.
            optional<std::string> getRaw(const std::vector<uint8_t>& key) const;

            // Drop a database instance.
            void dropInstance(const std::string &path);

            // Put a single PreferencesChange.
            void putPreferencesChange(
                leveldb::WriteBatch& batch,
                Option<AESCipher>& cipher,
                const api::PreferencesChange& change
            );

            // Create a new salt to use with an AESCipher.
            std::string createNewSalt(const std::shared_ptr<api::RandomNumberGenerator>& rng);

            // helper method used to encrypt things we want to put in leveldb
            std::vector<uint8_t> encrypt_preferences_change(
                const api::PreferencesChange& change,
                AESCipher& cipher
            );

            // helper method used to decrypt things we want to retrieve from leveldb
            std::vector<uint8_t> decrypt_preferences_change(
                const std::vector<uint8_t>& data,
                const AESCipher& cipher
            ) const;

            // an owning table that holds connection opened
            static std::unordered_map<std::string, std::shared_ptr<leveldb::DB>> LEVELDB_INSTANCE_POOL;
            static std::mutex LEVELDB_INSTANCE_POOL_MUTEX;

            static std::weak_ptr<leveldb::DB> obtainInstance(const std::string& path);
        };
    }
}

#endif //LEDGER_CORE_PREFERENCESBACKEND_HPP
