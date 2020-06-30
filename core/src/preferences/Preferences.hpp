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
#ifndef LEDGER_CORE_PREFERENCES_HPP
#define LEDGER_CORE_PREFERENCES_HPP

#include "../api/Preferences.hpp"
#include "../api/PreferencesBackend.hpp"
#include "../api/PreferencesEditor.hpp"
#include "PreferencesBackend.hpp"
#include "PreferencesEditor.hpp"
#include "../utils/Option.hpp"
#include <leveldb/db.h>
#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/set.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

namespace ledger {
    namespace core {
        class PreferencesBackend;
        class PreferencesEditor;

        class Preferences : public api::Preferences {
        public:
            Preferences(api::PreferencesBackend &backend, const std::vector<uint8_t> &keyPrefix);

            Preferences(api::PreferencesBackend &backend, const std::string &keyPrefix);

            std::string getString(const std::string &key, const std::string &fallbackValue) const override;

            int32_t getInt(const std::string &key, int32_t fallbackValue) const override;

            int64_t getLong(const std::string &key, int64_t fallbackValue) const override;

            bool getBoolean(const std::string &key, bool fallbackValue) const override;

            std::vector<std::string>
            getStringArray(const std::string &key, const std::vector<std::string> &fallbackValue) const override;

            bool contains(const std::string &key) const override;

            std::shared_ptr<api::PreferencesEditor> edit() override;
            std::shared_ptr<PreferencesEditor> editor();

            std::vector<uint8_t> getData(const std::string &key, const std::vector<uint8_t> &fallbackValue) const override;

            template <typename T>
            Option<T> getObject(const std::string& key) const {
                auto data = getData(key, {});
                if (data.size() == 0) {
                    return Option<T>();
                }
                T object;
                boost::iostreams::array_source my_vec_source(reinterpret_cast<char*>(&data[0]), data.size());
                boost::iostreams::stream<boost::iostreams::array_source> is(my_vec_source);
                ::cereal::PortableBinaryInputArchive archive(is);
                archive(object);
                return Option<T>(object);
            };

            std::shared_ptr<Preferences> getSubPreferences(std::string prefix) const;

        protected:
            std::vector<uint8_t> wrapKey(const std::string& key) const;

        private:
            api::PreferencesBackend& _backend;
            std::vector<uint8_t> _keyPrefix;
            friend class ledger::core::PreferencesEditor;
        };
    }
}

#endif //LEDGER_CORE_PREFERENCES_HPP
