/*
 *
 * ConfigurationImpl
 * ledger-core
 *
 * Created by Pierre Pollastri on 17/01/2017.
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
#ifndef LEDGER_CORE_CONFIGURATIONIMPL_HPP
#define LEDGER_CORE_CONFIGURATIONIMPL_HPP

#include "../api/Configuration.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cereal/types/unordered_map.hpp>

namespace ledger {
    namespace core {

        struct ConfigurationVariant {
            std::string string;
            std::vector<uint8_t> bytes;
            bool boolean;
            int32_t integer;
        };

        enum ConfigurationValueType {
            STRING, BYTES, BOOLEAN, INTEGER
        };

        struct ConfigurationValue {
            ConfigurationVariant variant;
            ConfigurationValueType type;

            template <class Archive>
            void serialize(Archive& ar) {
                ar(type);
                switch (type) {
                    case ConfigurationValueType::STRING:
                        ar(variant.string);
                        break;
                    case ConfigurationValueType::BYTES:
                        ar(variant.bytes);
                        break;
                    case ConfigurationValueType::BOOLEAN:
                        ar(variant.boolean);
                        break;
                    case ConfigurationValueType::INTEGER:
                        ar(variant.integer);
                        break;
                }
            };
        };

        class ConfigurationImpl : public api::Configuration, public std::enable_shared_from_this<ConfigurationImpl> {
        public:
            ConfigurationImpl();
            ConfigurationImpl(const ConfigurationImpl& cpy);

            ConfigurationImpl&operator=(std::shared_ptr<api::Configuration> configuration);

            std::shared_ptr<api::Configuration> putString(const std::string &key, const std::string &value) override;
            int32_t getInt(const std::string &key, int32_t fallback) override;
            std::shared_ptr<api::Configuration> putInt(const std::string &key, int32_t value) override;
            bool getBoolean(const std::string &key, bool fallback) override;
            std::shared_ptr<api::Configuration> putBoolean(const std::string &key, bool value) override;
            std::shared_ptr<api::Configuration>
            putData(const std::string &key, const std::vector<uint8_t> &data) override;
            std::string getString(const std::string &key, const std::string &fallback) override;
            std::vector<uint8_t> getData(const std::string &key, const std::vector<uint8_t> &fallback) override;

            template <class Archive>
            void serialize(Archive& ar) {
                ar(_values);
            }
        private:
            std::unordered_map<std::string, ConfigurationValue> _values;
        };
    }
}

#endif //LEDGER_CORE_CONFIGURATIONIMPL_HPP
