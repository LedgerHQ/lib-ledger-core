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

namespace ledger {
    namespace core {

        PreferencesBackend::PreferencesBackend(const std::string &path,
                                               const std::shared_ptr<api::ExecutionContext> &writingContext,
                                               const std::shared_ptr<api::PathResolver> &resolver) {
            _context = writingContext;
            leveldb::DB* db;
            leveldb::Options options;
            options.create_if_missing = true;
            auto status = leveldb::DB::Open(options, resolver->resolvePreferencesPath(path), &db);
            if (!status.ok()) {
                throw Exception(api::ErrorCode::UNABLE_TO_OPEN_LEVELDB, status.ToString());
            }
            _db = std::shared_ptr<leveldb::DB>(db);
        }

        void PreferencesBackend::commit(const std::vector<PreferencesChange> &changes) {
            auto db = _db;
            _context->execute(make_runnable([changes, db] () {
                leveldb::WriteBatch batch;
                leveldb::WriteOptions options;
                options.sync = true;
                for (auto& item : changes) {
                    leveldb::Slice k((const char *)item.key.data(), item.key.size());
                    if (item.type == PreferencesChangeType::PUT) {
                        leveldb::Slice v((const char *)item.value.data(), item.value.size());
                        batch.Put(k, v);
                    } else {
                        batch.Delete(k);
                    }
                }
                db->Write(options, &batch);
            }));
        }

        optional<std::string>  PreferencesBackend::get(const std::vector<uint8_t> &key) const {
            leveldb::Slice k((const char *)key.data(), key.size());
            std::string value;
            auto status = _db->Get(leveldb::ReadOptions(), k, &value);
            if (status.ok())
                return optional<std::string>(value);
            else
                return optional<std::string>();
        }

        void PreferencesBackend::iterate(const std::vector<uint8_t> &keyPrefix,
                                         std::function<bool (leveldb::Slice &&, leveldb::Slice &&)> f) {
            leveldb::Iterator* it = _db->NewIterator(leveldb::ReadOptions());
            leveldb::Slice start((const char *)keyPrefix.data(), keyPrefix.size());
            std::vector<uint8_t> limitRaw(keyPrefix.begin(), keyPrefix.end());
            limitRaw[limitRaw.size() - 1] += 1;
            leveldb::Slice limit((const char *)limitRaw.data(), limitRaw.size());
            for (it->Seek(start); it->Valid(); it->Next()) {
                if (it->key().compare(limit) > 0 || !f(it->key(), it->value())) {
                    break;
                }
            }
            delete it;
        }

        std::shared_ptr<Preferences> PreferencesBackend::getPreferences(const std::string &name) {
            return std::make_shared<Preferences>(*this, std::vector<uint8_t>(name.data(), name.data() + name.size()));
        }
    }
}