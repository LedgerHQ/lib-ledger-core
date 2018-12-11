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
#include "../crypto/AESCipher.hpp"
#include <iterator>

namespace ledger {
    namespace core {

        // number of iteration to perform for PBKDF2
        const auto PBKDF2_ITERS = 10000; // see https://pages.nist.gov/800-63-3/sp800-63b.html#sec5

        std::unordered_map<std::string, std::weak_ptr<leveldb::DB>> PreferencesBackend::LEVELDB_INSTANCE_POOL;
        std::mutex PreferencesBackend::LEVELDB_INSTANCE_POOL_MUTEX;

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

        PreferencesBackend::PreferencesBackend(const std::string &path,
                                               const std::shared_ptr<api::ExecutionContext> &writingContext,
                                               const std::shared_ptr<api::PathResolver> &resolver,
                                               std::shared_ptr<api::RandomNumberGenerator> rng,
                                               optional<const std::string&> password) {
            _context = writingContext;
            _db = obtainInstance(resolver->resolvePreferencesPath(path));
            _rng = rng;

            if (_password) {
                _password = optional<std::string>(*password);
            } else {
                _password = optional<std::string>();
            }
        }

        void PreferencesBackend::commit(const std::vector<PreferencesChange> &changes) {
            auto db = _db;
            leveldb::WriteBatch batch;
            leveldb::WriteOptions options;
            options.sync = true;
            for (auto& item : changes) {
                leveldb::Slice k((const char *)item.key.data(), item.key.size());
                if (item.type == PreferencesChangeType::PUT_TYPE) {
                    if (_password) {
                        auto encrypted = encrypt_preferences_change(*_password, item);
                        leveldb::Slice v((const char *)encrypted.data(), encrypted.size());
                        batch.Put(k, v);
                    } else {
                        leveldb::Slice v((const char *)item.value.data(), item.value.size());
                        batch.Put(k, v);
                    }
                } else {
                    batch.Delete(k);
                }
            }
            db->Write(options, &batch);
        }

        optional<std::string> PreferencesBackend::get(const std::vector<uint8_t>& key) const {
            leveldb::Slice k((const char *)key.data(), key.size());
            std::string value;

            auto status = _db->Get(leveldb::ReadOptions(), k, &value);
            if (status.ok()) {
                if (_password) {
                    auto ciphertext = std::vector<uint8_t>(std::begin(value), std::end(value));
                    auto plaindata = decrypt_preferences_change(*_password, key, ciphertext);
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
                // TODO: decrypt here
                if (it->key().compare(limit) > 0 || !f(it->key(), it->value())) {
                    break;
                }

                if (it->key().compare(limit) > 0) {
                    break;
                }

                if (_password) {
                } else {
                }
            }
        }

        std::shared_ptr<Preferences> PreferencesBackend::getPreferences(const std::string &name) {
            return std::make_shared<Preferences>(*this, std::vector<uint8_t>(name.data(), name.data() + name.size()));
        }

        // Encrypt a preference change.
        std::vector<uint8_t> PreferencesBackend::encrypt_preferences_change(
            const std::string& password,
            const PreferencesChange& change
        ) const {
          // Create an AES256 cipher
          const auto salt = std::string(std::begin(change.key), std::end(change.key));
          auto cipher = AESCipher(_rng, password, salt, PBKDF2_ITERS);

          // encrypt
          auto input = BytesReader(change.value);
          auto output = BytesWriter();
          cipher.encrypt(input, output);

          return output.toByteArray();
        }

        // Decrypt a preference change value.
        std::vector<uint8_t> PreferencesBackend::decrypt_preferences_change(
            const std::string& password,
            const std::vector<uint8_t>& key,
            const std::vector<uint8_t>& data
        ) const {
          // Create an AES256 cipher
          const auto salt = std::string(std::begin(key), std::end(key));
          auto cipher = AESCipher(_rng, password, salt, PBKDF2_ITERS);

          // decrypt
          auto input = BytesReader(data);
          auto output = BytesWriter();
          cipher.decrypt(input, output);

          return output.toByteArray();
        }
    }
}
