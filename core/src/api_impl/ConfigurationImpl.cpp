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
#include "ConfigurationImpl.hpp"

namespace ledger {

    namespace core {

        std::shared_ptr<api::Configuration>
        ConfigurationImpl::putString(const std::string &key, const std::string &value) {
            ConfigurationValue v;
            v.type = ConfigurationValueType::STRING;
            v.variant.string = value;
            _values[key] = v;
            return shared_from_this();
        }

        int32_t ConfigurationImpl::getInt(const std::string &key, int32_t fallback) {
            auto it = _values.find(key);
            return (it == _values.end() || it->second.type != ConfigurationValueType::INTEGER) ? fallback
                                                                                               : it->second.variant.integer;
        }

        std::shared_ptr<api::Configuration>
        ConfigurationImpl::putInt(const std::string &key, int32_t value) {
            ConfigurationValue v;
            v.type = ConfigurationValueType::INTEGER;
            v.variant.integer = value;
            _values[key] = v;
            return shared_from_this();
        }

        bool ConfigurationImpl::getBoolean(const std::string &key, bool fallback) {
            auto it = _values.find(key);
            return (it == _values.end() || it->second.type != ConfigurationValueType::BOOLEAN) ? fallback
                                                                                               : it->second.variant.boolean;
        }

        std::shared_ptr<api::Configuration>
        ConfigurationImpl::putBoolean(const std::string &key, bool value) {
            ConfigurationValue v;
            v.type = ConfigurationValueType::BOOLEAN;
            v.variant.boolean = value;
            _values[key] = v;
            return shared_from_this();
        }

        std::shared_ptr<api::Configuration>
        ConfigurationImpl::putData(const std::string &key, const std::vector<uint8_t> &data) {
            ConfigurationValue v;
            v.type = ConfigurationValueType::BYTES;
            v.variant.bytes = data;
            _values[key] = v;
            return shared_from_this();
        }

        std::string ConfigurationImpl::getString(const std::string &key, const std::string &fallback) {
            auto it = _values.find(key);
            return (it == _values.end() || it->second.type != ConfigurationValueType::STRING) ? fallback
                                                                                              : it->second.variant.string;
        }

        std::vector<uint8_t> ConfigurationImpl::getData(const std::string &key, const std::vector<uint8_t> &fallback) {
            auto it = _values.find(key);
            return (it == _values.end() || it->second.type != ConfigurationValueType::BYTES) ? fallback
                                                                                             : it->second.variant.bytes;
        }

        ConfigurationImpl::ConfigurationImpl() {

        }

        ConfigurationImpl::ConfigurationImpl(const ConfigurationImpl &cpy) {
            _values = cpy._values;
        }

        ConfigurationImpl &ConfigurationImpl::operator=(std::shared_ptr<api::Configuration> configuration) {
            auto conf = std::static_pointer_cast<ConfigurationImpl>(configuration);
            _values = conf->_values;
            return *this;
        }

        std::shared_ptr<api::Configuration> api::Configuration::newInstance() {
            return std::make_shared<ledger::core::ConfigurationImpl>();
        }
    }
}