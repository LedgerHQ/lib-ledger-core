/*
 *
 * Preferences
 * ledger-core
 *
 * Created by Pierre Pollastri on 11/01/2017.
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
#include "Preferences.hpp"
#include "PreferencesBackend.hpp" // TODO remove when iterate fixed
#include "../bytes/BytesReader.h"

namespace ledger {
    namespace core {

        Preferences::Preferences(api::PreferencesBackend &backend, const std::vector<uint8_t> &keyPrefix)
            : _backend(backend)
            , _keyPrefix(keyPrefix)
        {}

        Preferences::Preferences(api::PreferencesBackend &backend, const std::string &keyPrefix)
            : Preferences(backend, std::vector<uint8_t>(std::begin(keyPrefix), std::end(keyPrefix)))
        {}

        std::string Preferences::getString(const std::string &key, const std::string &fallbackValue) {
            auto value = _backend.get(wrapKey(key));
            if (!value)
                return fallbackValue;
            return *value;
        }

        int32_t Preferences::getInt(const std::string &key, int32_t fallbackValue) {
            auto value = _backend.get(wrapKey(key));
            if (!value)
                return fallbackValue;
            BytesReader reader(std::vector<uint8_t>(value->data(), value->data() + value->size()));
            return (int32_t)reader.readNextLeUint();
        }

        int64_t Preferences::getLong(const std::string &key, int64_t fallbackValue) {
            auto value = _backend.get(wrapKey(key));
            if (!value)
                return fallbackValue;
            BytesReader reader(std::vector<uint8_t>(value->data(), value->data() + value->size()));
            return (int64_t)reader.readNextLeUlong();
        }

        bool Preferences::getBoolean(const std::string &key, bool fallbackValue) {
            auto value = _backend.get(wrapKey(key));
            if (!value)
                return fallbackValue;
            BytesReader reader(std::vector<uint8_t>(value->data(), value->data() + value->size()));
            return reader.readNextByte() == 0x01;
        }

        std::vector<std::string>
        Preferences::getStringArray(const std::string &key, const std::vector<std::string> &fallbackValue) {
            auto value = _backend.get(wrapKey(key));

            if (!value)
                return fallbackValue;

            BytesReader reader(std::vector<uint8_t>(value->data(), value->data() + value->size()));
            std::vector<std::string> result;

            while (reader.hasNext()) {
                result.push_back(reader.readNextVarString());
            }

            return result;
        }

        bool Preferences::contains(const std::string &key) {
            return _backend.get(wrapKey(key)) ? true : false;
        }

        std::shared_ptr<api::PreferencesEditor> Preferences::edit() {
            return std::make_shared<PreferencesEditor>(*this);
        }

        std::vector<uint8_t> Preferences::wrapKey(const std::string &key) const {
            std::vector<uint8_t> wrappedKey(_keyPrefix.size() + key.size());
            auto wrappedKeyIndex = 0;
            auto prefixSize = _keyPrefix.size();
            auto keySize = key.size();
            for (auto i = 0; i < prefixSize; i++) {
                wrappedKey[wrappedKeyIndex] = _keyPrefix[i];
                wrappedKeyIndex += 1;
            }
            for (auto i = 0; i < keySize; i++) {
                wrappedKey[wrappedKeyIndex] = (uint8_t)key[i];
                wrappedKeyIndex += 1;
            }
            return wrappedKey;
        }

        std::shared_ptr<Preferences> Preferences::getSubPreferences(std::string prefix) {
            std::vector<uint8_t> p = _keyPrefix;
            auto prefixSize = prefix.size();
            for (auto i = 0; i < prefixSize; i++) {
                p.push_back((uint8_t)prefix[i]);
            }
            return std::make_shared<Preferences>(_backend, p);
        }

        void
        Preferences::iterate(std::function<bool (leveldb::Slice&&, leveldb::Slice &&)> f, Option<std::string> begin) {
            auto start = wrapKey(begin.getValueOr(""));
            auto startSize = start.size();
            // TODO fixme
            dynamic_cast<PreferencesBackend&>(_backend).iterate(start, [&] (leveldb::Slice&& k, leveldb::Slice&& value) {
                if (startSize < k.size()) {
                    return f(std::move(leveldb::Slice(k.data() + start.size(), k.size() - start.size())),
                             std::move(value));
                }
                return true;
            });
        }

        std::vector<uint8_t> Preferences::getData(const std::string &key, const std::vector<uint8_t> &fallbackValue) {
            auto value = _backend.get(wrapKey(key));
            if (!value)
                return fallbackValue;
            BytesReader reader(std::vector<uint8_t>(value->data(), value->data() + value->size()));
            return reader.readUntilEnd();
        }

        std::shared_ptr<PreferencesEditor> Preferences::editor() {
            return std::make_shared<PreferencesEditor>(*this);
        }
    }
}
