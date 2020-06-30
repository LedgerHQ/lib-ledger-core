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
#include "Preferences.hpp"
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
            const uint32_t PBKDF2_ITERS = 10000; // see https://pages.nist.gov/800-63-3/sp800-63b.html#sec5

            // key at which the encryption salt is found
            const std::string ENCRYPTION_SALT_KEY = "preferences.backend.salt";
        }

        std::unordered_map<std::string, std::shared_ptr<leveldb::DB>> PreferencesBackend::LEVELDB_INSTANCE_POOL;
        std::mutex PreferencesBackend::LEVELDB_INSTANCE_POOL_MUTEX;

        PreferencesBackend::PreferencesBackend(const std::string &path,
                                               const std::shared_ptr<api::ExecutionContext>& writingContext,
                                               const std::shared_ptr<api::PathResolver> &resolver)
            : api::PreferencesBackend() {
            _context = writingContext;
            _dbName = resolver->resolvePreferencesPath(path);
            _db = obtainInstance(_dbName);
        }

        std::weak_ptr<leveldb::DB> PreferencesBackend::obtainInstance(const std::string &path) {
            std::lock_guard<std::mutex> lock(LEVELDB_INSTANCE_POOL_MUTEX);
            auto it = LEVELDB_INSTANCE_POOL.find(path);
            if (it != LEVELDB_INSTANCE_POOL.end()) {
                auto db = it->second;
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

            LEVELDB_INSTANCE_POOL[path] = instance;

            return weakInstance;
        }

        bool PreferencesBackend::commit(const std::vector<api::PreferencesChange> &changes) {
            auto db = _db.lock();

            if (db == nullptr) {
              return false;
            }

            leveldb::WriteBatch batch;
            leveldb::WriteOptions options;
            options.sync = true;

            for (auto& item : changes) {
                putPreferencesChange(batch, _cipher, item);
            }

            db->Write(options, &batch);

            return true;
        }

        // Put a single PreferencesChange.
        void PreferencesBackend::putPreferencesChange(
            leveldb::WriteBatch& batch,
            Option<AESCipher>& cipher,
            const api::PreferencesChange& change
        ) {
            leveldb::Slice k((const char *)change.key.data(), change.key.size());

            if (change.type == api::PreferencesChangeType::PUT) {
                if (cipher.hasValue()) {
                    auto encrypted = encrypt_preferences_change(change, *cipher);

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

        optional<std::string> PreferencesBackend::get(const std::vector<uint8_t>& key) const {
            auto value = getRaw(key);

            if (value) {
                if (_cipher.hasValue()) {
                    auto ciphertext = std::vector<uint8_t>(value->cbegin(), value->cend());
                    auto plaindata = decrypt_preferences_change(ciphertext, *_cipher);
                    auto plaintext = std::string(plaindata.cbegin(), plaindata.cend());

                    return optional<std::string>(plaintext);
                } else {
                    return optional<std::string>(value);
                }
            } else {
                return optional<std::string>();
            }
        }

        optional<std::string> PreferencesBackend::getRaw(const std::vector<uint8_t>& key) const {
            auto db = _db.lock();

            if (db == nullptr) {
              return optional<std::string>();
            }

            leveldb::Slice k((const char *)key.data(), key.size());
            std::string value;

            auto status = db->Get(leveldb::ReadOptions(), k, &value);
            if (status.ok()) {
                return optional<std::string>(value);
            } else {
                return optional<std::string>();
            }
        }

        std::shared_ptr<Preferences> PreferencesBackend::getPreferences(const std::string &name) {
            return std::make_shared<Preferences>(*this, std::vector<uint8_t>(name.data(), name.data() + name.size()));
        }

        std::string PreferencesBackend::createNewSalt(const std::shared_ptr<api::RandomNumberGenerator>& rng) {
            auto bytes = rng->getRandomBytes(128);
            return std::string(bytes.begin(), bytes.end());
        }

        void PreferencesBackend::setEncryption(
            const std::shared_ptr<api::RandomNumberGenerator>& rng,
            const std::string& password
        ) {
            // setting encryption is akin to resetting with an old password that is empty
            resetEncryption(rng, "", password);
        }

        void PreferencesBackend::unsetEncryption() {
            _cipher = Option<AESCipher>::NONE;
        }

        bool PreferencesBackend::resetEncryption(
            const std::shared_ptr<api::RandomNumberGenerator>& rng,
            const std::string& oldPassword,
            const std::string& newPassword
        ) {
            auto db = _db.lock();

            if (db == nullptr) {
              return false;
            }

            Option<AESCipher> noCipher;
            auto newCipher = noCipher;
            auto salt = getEncryptionSalt();

            if (oldPassword.empty()) {
                // password empty means we either want to encrypt a plaintext DB (if there’s no
                // salt already) or that we want to set encryption on (if a salt is persisted)
                if (!newPassword.empty()) {
                    if (salt.empty()) {
                        // no salt, then we want to encrypt a plaintext DB: we only need to set the
                        // new cipher and leave the decrypting cipher disabled; we’ll also not leave
                        // this function right away as we need to encrypt the plaintext values
                        salt = createNewSalt(rng);
                        newCipher = Option<AESCipher>(AESCipher(rng, newPassword, salt, PBKDF2_ITERS));
                        _cipher = noCipher;
                    } else {
                        // we want to enable encryption with a given password (new) without
                        // decrypting the data already present; we don’t need anything besides
                        // setting the cipher and returning from the function
                        _cipher = Option<AESCipher>(AESCipher(rng, newPassword, salt, PBKDF2_ITERS));
                        return true;
                    }
                } else {
                    // no old password and no new password; do nothing (this is not considered an
                    // error)
                    return true;
                }
            } else {
                // the old password is present; it means that we always want to decrypt data and
                // change encryption; either we want to got to full plaintext again (empty new
                // password), or just change encryption); in both cases, we need a salt and to go
                // on through this function
                if (salt.empty()) {
                    // trying to decrypt with an old password but without a persisted salt: error
                    return false;
                }

                // we’ll read with this cipher
                _cipher = Option<AESCipher>(AESCipher(rng, oldPassword, salt, PBKDF2_ITERS));

                if (!newPassword.empty()) {
                    // encrypt with the new password if present
                    salt = createNewSalt(rng);
                    newCipher = Option<AESCipher>(AESCipher(rng, newPassword, salt, PBKDF2_ITERS));
                }
            }

            // function to get a value from the DB out of a string; this function is an optimization
            // so that the branching on _cipher is done once before the loop instead of doing it at
            // every iteration
            std::function<std::vector<uint8_t> (std::vector<uint8_t> const&)> readValue;

            if (_cipher.hasValue()) {
                readValue = [&](std::vector<uint8_t> const& value) {
                    auto ciphertext = std::vector<uint8_t>(value.cbegin(), value.cend());
                    return decrypt_preferences_change(ciphertext, *_cipher);
                };
            } else {
                readValue = [&](std::vector<uint8_t> const& value) {
                    return value;
                };
            }

            // now we can iterate over all data, decrypt with the “old” cipher, encrypt with the
            // “new” cipher and generate two changes per entry: one that deletes it (to prevent to
            // duplicating it) and one that puts the new encoded one; once the iteration is done,
            // we submit the batch to leveldb and it applies it atomically
            leveldb::WriteBatch batch;
            auto it = std::unique_ptr<leveldb::Iterator>(db->NewIterator(leveldb::ReadOptions()));

            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                // decrypt with the old cipher, if any
                auto value = it->value().ToString();
                auto plaindata = std::vector<uint8_t>(value.cbegin(), value.cend());
                plaindata = readValue(plaindata);

                // the key is never encrypted
                auto keyStr = it->key().ToString();
                auto key = std::vector<uint8_t>(keyStr.cbegin(), keyStr.cend());

                // remove the key and its associated value to prevent duplication
                putPreferencesChange(batch, _cipher, api::PreferencesChange(api::PreferencesChangeType::DELETE, key, {}));

                // encrypt with the new cipher (if any); in order to do that, we need a PreferencesChange
                // to add with the new cipher
                auto change = api::PreferencesChange(api::PreferencesChangeType::PUT, key, plaindata);
                putPreferencesChange(batch, newCipher, change);
            }

            // we also need to update the salt if we are encrypting
            auto saltKey = std::vector<uint8_t>(ENCRYPTION_SALT_KEY.cbegin(), ENCRYPTION_SALT_KEY.cend());
            putPreferencesChange(batch, noCipher, api::PreferencesChange(api::PreferencesChangeType::DELETE, saltKey, {}));

            if (newCipher.hasValue()) {
                // we put a new salt only if we are encrypting
                auto newSaltBytes = std::vector<uint8_t>(salt.cbegin(), salt.cend());

                // remove the previous salt
                // add the new salt
                putPreferencesChange(batch, noCipher, api::PreferencesChange(api::PreferencesChangeType::PUT, saltKey, newSaltBytes));
            }

            // atomic update
            leveldb::WriteOptions writeOpts;
            writeOpts.sync = true;
            db->Write(writeOpts, &batch);

            // update the cipher to use with the new one
            _cipher = newCipher;

            return true;
        }

        void PreferencesBackend::clear() {
            // unset encryption because it’s disabled by default
            unsetEncryption();

            // drop and recreate the DB; we need to scope that because the lock must be released
            // in order for obtainInstance to work correctly
            {
                std::lock_guard<std::mutex> lock(LEVELDB_INSTANCE_POOL_MUTEX);
                auto it = LEVELDB_INSTANCE_POOL.find(_dbName);
                if (it != LEVELDB_INSTANCE_POOL.end()) {
                    // drop the cached DB first and closes the database
                    LEVELDB_INSTANCE_POOL.erase(it);
                }

                _db.reset();

                leveldb::Options options;
                leveldb::DestroyDB(_dbName, options);
            }

            _db = obtainInstance(_dbName);
        }

        std::string PreferencesBackend::getEncryptionSalt() const {
            auto saltKey = std::vector<uint8_t>(ENCRYPTION_SALT_KEY.cbegin(), ENCRYPTION_SALT_KEY.cend());
            return getRaw(saltKey).value_or("");
        }

        std::vector<uint8_t> PreferencesBackend::encrypt_preferences_change(
            const api::PreferencesChange& change,
            AESCipher& cipher
        ) {
          auto input = BytesReader(change.value);
          auto output = BytesWriter();
          cipher.encrypt(input, output);

          return output.toByteArray();
        }

        std::vector<uint8_t> PreferencesBackend::decrypt_preferences_change(
            const std::vector<uint8_t>& data,
            const AESCipher& cipher
        ) const {
          auto input = BytesReader(data);
          auto output = BytesWriter();
          cipher.decrypt(input, output);

          return output.toByteArray();
        }
    }
}
