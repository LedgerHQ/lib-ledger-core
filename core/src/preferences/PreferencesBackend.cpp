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

#include "PreferencesBackend.hpp"
#include "../utils/Exception.hpp"
#include "../utils/LambdaRunnable.hpp"
#include <leveldb/write_batch.h>
#include <cstring>
#include <leveldb/env.h>
#include <iterator>

namespace ledger {
    namespace core {
        namespace {
            // number of iteration to perform for PBKDF2
            const auto PBKDF2_ITERS = 10000; // see https://pages.nist.gov/800-63-3/sp800-63b.html#sec5
        }

        PreferencesChange::PreferencesChange(PreferencesChangeType t, std::vector<uint8_t> k, std::vector<uint8_t> v)
            : type(t), key(k), value(v) {
        }

        std::unordered_map<std::string, std::weak_ptr<leveldb::DB>> PreferencesBackend::LEVELDB_INSTANCE_POOL;
        std::mutex PreferencesBackend::LEVELDB_INSTANCE_POOL_MUTEX;

        PreferencesBackend::PreferencesBackend(const std::string &path,
                                               const std::shared_ptr<api::ExecutionContext>& writingContext,
                                               const std::shared_ptr<api::PathResolver> &resolver) {
            _context = writingContext;
            _dbName = resolver->resolvePreferencesPath(path);
            _db = obtainInstance(_dbName);
        }

        std::shared_ptr<leveldb::DB> PreferencesBackend::obtainInstance(const std::string &path) {
            std::lock_guard<std::mutex> lock(LEVELDB_INSTANCE_POOL_MUTEX);
            auto it = LEVELDB_INSTANCE_POOL.find(path);
            if (it != LEVELDB_INSTANCE_POOL.end()) {
                auto db = it->second.lock();
                if (db != nullptr)
                    return db;
            }

            leveldb::DB *db;
            leveldb::Options options;
            options.create_if_missing = true;

            auto status = leveldb::DB::Open(options, path, &db);
            if (!status.ok()) {
                throw Exception(api::ErrorCode::UNABLE_TO_OPEN_LEVELDB, status.ToString());
            }

            auto instance = std::shared_ptr<leveldb::DB>(db);
            std::weak_ptr<leveldb::DB> weakInstance = instance;

            LEVELDB_INSTANCE_POOL[path] = weakInstance;

            return instance;
        }

        void PreferencesBackend::dropInstance(const std::string &path) {
            std::lock_guard<std::mutex> lock(LEVELDB_INSTANCE_POOL_MUTEX);

            auto it = LEVELDB_INSTANCE_POOL.find(path);
            if (it != LEVELDB_INSTANCE_POOL.end()) {
                // drop the cached DB first
                LEVELDB_INSTANCE_POOL.erase(it);
            }

            _db.reset(); // this should completely drop

            leveldb::Options options;
            leveldb::DestroyDB(path, options);
        }

        void PreferencesBackend::commit(const std::vector<PreferencesChange> &changes) {
            auto db = _db;
            leveldb::WriteBatch batch;
            leveldb::WriteOptions options;
            options.sync = true;

            for (auto& item : changes) {
                putPreferencesChange(batch, _cipher, item);
            }

            db->Write(options, &batch);
        }

        // Put a single PreferencesChange.
        void PreferencesBackend::putPreferencesChange(
            leveldb::WriteBatch& batch,
            optional<AESCipher>& cipher,
            const PreferencesChange& change
        ) {
            leveldb::Slice k((const char *)change.key.data(), change.key.size());

            if (change.type == PreferencesChangeType::PUT_TYPE) {
                if (cipher.hasValue()) {
                    auto encrypted = encrypt_preferences_change(change, cipher);

                    leveldb::Slice v((const char *)encrypted.data(), encrypted.size());
                    batch.Put(k, v);
                } else {
                    leveldb::Slice v((const char *)change.value.data(), change.value.size());
                    batch.Put(k, v);
                }
            } else {
                batch.Delete(k);
            }
        }

        optional<std::string> PreferencesBackend::get(const std::vector<uint8_t>& key) {
            leveldb::Slice k((const char *)key.data(), key.size());
            std::string value;

            auto status = _db->Get(leveldb::ReadOptions(), k, &value);
            if (status.ok()) {
                if (_cipher.hasValue()) {
                    auto ciphertext = std::vector<uint8_t>(std::begin(value), std::end(value));
                    auto plaindata = decrypt_preferences_change(ciphertext, *_cipher);
                    auto plaintext = std::string(std::begin(plaindata), std::end(plaindata));

                    return optional<std::string>(plaintext);
                } else {
                    return optional<std::string>(value);
                }
            } else {
                return optional<std::string>();
            }
        }

        void PreferencesBackend::iterate(const std::vector<uint8_t> &keyPrefix,
                                         std::function<bool (leveldb::Slice &&, leveldb::Slice &&)> f) {
            std::unique_ptr<leveldb::Iterator> it(_db->NewIterator(leveldb::ReadOptions()));
            leveldb::Slice start((const char *) keyPrefix.data(), keyPrefix.size());
            std::vector<uint8_t> limitRaw(keyPrefix.begin(), keyPrefix.end());

            limitRaw[limitRaw.size() - 1] += 1;
            leveldb::Slice limit((const char *) limitRaw.data(), limitRaw.size());

            for (it->Seek(start); it->Valid(); it->Next()) {
                if (it->key().compare(limit) > 0) {
                    break;
                }

                if (_cipher.hasValue()) {
                    // decrypt the value on the fly
                    auto value = it->value().ToString();
                    auto ciphertext = std::vector<uint8_t>(std::begin(value), std::end(value));
                    auto plaindata = decrypt_preferences_change(ciphertext, *_cipher);
                    auto plaintext = std::string(std::begin(plaindata), std::end(plaindata));
                    leveldb::Slice slice(plaintext);

                    if (!f(it->key(), std::move(slice))) {
                        break;
                    }
                } else {
                    if (!f(it->key(), it->value())) {
                        break;
                    }
                }
            }
        }

        std::shared_ptr<Preferences> PreferencesBackend::getPreferences(const std::string &name) {
            return std::make_shared<Preferences>(*this, std::vector<uint8_t>(name.data(), name.data() + name.size()));
        }

        std::string PreferencesBackend::createNewSalt(const std::shared_&tr<api::RandomNumberGenerator>& rng) {
            auto bytes = rng->getRandomBytes(128);
            return std::string(std::begin(bytes), std::end(bytes));
        }

        void PreferencesBackend::setEncryption(
            const std::shared_ptr<api::RandomNumberGenerator>& rng,
            const std::string& password
        ) {
            // disable encryption to check whether we have a salt already persisted
            unsetEncryption();

            auto emptySalt = std::string("");
            auto pref = getPreferences("__core");
            auto salt = pref->getString("preferences.backend.salt", emptySalt);

            if (salt == emptySalt) {
                // we don’t have a proper salt; create one and persist it for future use
<<<<<<< HEAD
                auto bytes = rng->getRandomBytes(128);

                salt = std::string(std::begin(bytes), std::end(bytes));
=======
                salt = createNewSalt(rng);
>>>>>>> First version of the re-encryption algorithm for preferences.
                pref->editor()->putString("preferences.backend.salt", salt)->commit();
            }

            // create the AES cipher
            _cipher = AESCipher(rng, password, salt, PBKDF2_ITERS);
        }

        void PreferencesBackend::unsetEncryption() {
            _cipher = Option<AESCipher>::NONE;
        }

<<<<<<< HEAD
        void PreferencesBackend::clear() {
            // unset encryption_tests because it’s disabled by default
=======
        // first one
        bool PreferencesBackend::resetEncryption(
            const std::shared_ptr<api::RandomNumberGenerator>& rng,
            const std::string& newPassword,
            const std::string& oldPassword
        ) {
            // turn on encryption with the old password first
            setEncryption(rng, oldPassword);

            // from now on, reading data will use the old password, but we want to persist with a
            // brand new cipher; create it here
            auto newSalt = createNewSalt(rng);
            auto newCipher = AESCipher(rng, newPassword, newSalt, PBKDF2_ITERS);

            // we will also need a brand new leveldb to implement a somewhat atomic database swap;
            // the idea is to read from the old leveldb and write to this new one; when we’re done,
            // we just “swap” the database and remove the old one; this swap is needed so that we
            // don’t corrupt the old database if we crash or get interrupted while in the middle of
            // the process of resetting
            auto tempDBName = _dbName + "__temp_copy__";
            auto tempDB = obtainInstance(tempDBName);

            // now we can iterate over all data, decrypt with the “old” cipher, encrypt with the
            // “new” cipher and persist to the temporary leveldb
            {
                auto it = std::unique_ptr(_db->NewIterator(leveldb::ReadOptions()));
                leveldb::WriteBatch batch;
                leveldb::WriteOptions writeOpts;
                writeOpts.sync = true;

                // persist the salt so that we can repair later if needed
                {
                    std::string keyStr = "preferences.backend.salt";
                    auto key = std::vector<uint8_t>(keyStr.cbegin(), keyStr.cend());
                    auto value = std::vector<uint8_t>(newSalt.cbegin(), newSalt.cend());
                    auto saltChange = PreferencesChange(PreferencesChangeType::PUT_TYPE, key, value);
                    putPreferencesChange(batch, optional<AESCipher>(newCipher), saltChange);
                    tempDB->Write(writeOpts, &batch);
                }

                for (it->SeekToFirst(); it->Valid(); it->Next()) {
                    // decrypt with the old cipher
                    auto value = it->value().ToString();
                    auto ciphertext = std::vector<uint8_t>(std::begin(value), std::end(value));
                    auto plaindata = decrypt_preferences_change(ciphertext, *_cipher);

                    // the key is not encrypted
                    auto keyStr = it->key().ToString();
                    auto key = std::vector<uint8_t>(keyStr.data(), keyStr.size());

                    // encrypt with the new cipher; in order to do that, we need a PreferencesChange to
                    // add with the new cipher and then put to the new database
                    auto change = PreferencesChange(PreferencesChangeType::PUT_TYPE, key, plaindata);
                    putPreferencesChange(batch, optional<AESCipher>(newCipher), change);
                    tempDB->Write(writeOpts, &batch);
                }
            }

            // we re-encrypted the whole database by copy; now we need to perform the atomic swap;
            // first, we delete the old database; then we recreate it (so that it’s empty); finally,
            // we perform a raw copy from the temporary to the fresh and we remove the temporary
            // (that was such a trip, yep)
            dropInstance(_dbName);
            _db = obtainInstance(_dbName);

            // DANGER ZONE here: if we crash, we will have to repair because we’ve dropped the
            // database; see the constructor to see how repairing occurs

            // raw copy
            {
                auto it = std::unique_ptr(tempDB->NewIterator(leveldb::ReadOptions()));
                leveldb::WriteBatch batch;
                leveldb::WriteOptions writeOpts;
                writeOpts.sync = true;

                for (it->SeekToFirst(); it->Valid(); it->Next()) {
                    auto key_ = it->key();
                    auto key = std::vector<uint8_t>(key_.data(), key_.size());
                    auto value_ = it->value();
                    auto value = std::vector<uint8_t>(key_.data(), key_.size());
                    auto change = PreferencesChange(PreferencesChangeType::PUT_TYPE, key, value);
                    _db->Write(writeOpts, &batch);
                }
            }

            tempDB.reset(); // we don’t need that anymore
            dropInstance(tempDBName);
        }

        void PreferencesBackend::clear() {
            // unset encryption because it’s disabled by default
>>>>>>> First version of the re-encryption algorithm for preferences.
            unsetEncryption();

            // drop and recreate the DB; we need to scope that because the lock must be released
            // in order for obtainInstance to work correctly
            {
                std::lock_guard<std::mutex> lock(LEVELDB_INSTANCE_POOL_MUTEX);
                auto it = LEVELDB_INSTANCE_POOL.find(_dbName);
                if (it != LEVELDB_INSTANCE_POOL.end()) {
                    // drop the cached DB first
                    LEVELDB_INSTANCE_POOL.erase(it);
                }

                _db.reset(); // this should completely drop

                leveldb::Options options;
                leveldb::DestroyDB(_dbName, options);
<<<<<<< HEAD

=======
>>>>>>> First version of the re-encryption algorithm for preferences.
            }

            _db = obtainInstance(_dbName);
        }

<<<<<<< HEAD
        std::vector<uint8_t> PreferencesBackend::encrypt_preferences_change(const PreferencesChange& change) {
=======
        std::vector<uint8_t> PreferencesBackend::encrypt_preferences_change(
            const PreferencesChange& change,
            AESCipher& cipher
        ) {
>>>>>>> First version of the re-encryption algorithm for preferences.
          auto input = BytesReader(change.value);
          auto output = BytesWriter();
          cipher->encrypt(input, output);

          return output.toByteArray();
        }

        std::vector<uint8_t> PreferencesBackend::decrypt_preferences_change(
            const std::vector<uint8_t>& data,
            AESCipher& cipher
        ) {
          auto input = BytesReader(data);
          auto output = BytesWriter();
          cipher->decrypt(input, output);

          return output.toByteArray();
        }
    }
}
