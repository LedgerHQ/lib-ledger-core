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
#include <utils/Option.hpp>
#include <crypto/AESCipher.hpp>

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
            PreferencesChange(PreferencesChangeType t, std::vector<uint8_t> k, std::vector<uint8_t> v);
        };

        class PreferencesBackend {
        public:
            PreferencesBackend(
                const std::string& path,
                const std::shared_ptr<api::ExecutionContext>& writingContext,
                const std::shared_ptr<api::PathResolver>& resolver
            );

            ~PreferencesBackend() = default;

            std::shared_ptr<Preferences> getPreferences(const std::string& name);
            void iterate(const std::vector<uint8_t>& keyPrefix, std::function<bool (leveldb::Slice&&, leveldb::Slice&&)>);
            optional<std::string> get(const std::vector<uint8_t>& key);

            /// Commit a change. Return false if unsuccessful (might happen if the underlying DB
            /// was destroyed).
            bool commit(const std::vector<PreferencesChange>& changes);

            /// Turn encryption on for all future uses.
            ///
            /// This method will set encryption on for all future values that will be persisted.
            /// If this function is called on a plaintext storage (i.e. first encryption for
            /// instance), it will also encrypt all data already present.
            void setEncryption(
                const std::shared_ptr<api::RandomNumberGenerator>& rng,
                const std::string& password
            );

            /// Turn off encryption by disabling the use of the internal cipher. Data is left
            /// untouched.
            ///
            /// This method is suitable when you want to get back raw, encrypted data. If you want
            /// to disable encryption in order to read clear data back without password, consider
            /// the resetEncryption method instead.
            void unsetEncryption();

            /// Reset the encryption with a new password by first decrypting on the
            /// fly with the old password the data present.
            ///
            /// If the new password is an empty string, after this method is called, the database
            /// is completely unciphered and no password is required to read from it.
            ///
            /// Return true if the reset occurred correctly, false otherwise (e.g. trying to change
            /// password with an old password but without a proper salt already persisted).
            bool resetEncryption(
                const std::shared_ptr<api::RandomNumberGenerator>& rng,
                const std::string& oldPassword,
                const std::string& newPassword
            );

            /// Get encryption salt, if any.
            std::string getEncryptionSalt();

            /// Clear all preferences.
            void clear();

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
                const PreferencesChange& change
            );

            // Create a new salt to use with an AESCipher.
            std::string createNewSalt(const std::shared_ptr<api::RandomNumberGenerator>& rng);

            // helper method used to encrypt things we want to put in leveldb
            std::vector<uint8_t> encrypt_preferences_change(
                const PreferencesChange& change,
                AESCipher& cipher
            );

            // helper method used to decrypt things we want to retrieve from leveldb
            std::vector<uint8_t> decrypt_preferences_change(
                const std::vector<uint8_t>& data,
                AESCipher& cipher
            );

            // an owning table that holds connection opened
            static std::unordered_map<std::string, std::shared_ptr<leveldb::DB>> LEVELDB_INSTANCE_POOL;
            static std::mutex LEVELDB_INSTANCE_POOL_MUTEX;

            static std::weak_ptr<leveldb::DB> obtainInstance(const std::string& path);
        };
    }
}

#endif //LEDGER_CORE_PREFERENCESBACKEND_HPP
