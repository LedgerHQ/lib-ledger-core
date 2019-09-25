/*
 *
 * ConfigurationMatchable
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/05/2017.
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

#include <core/api/DynamicType.hpp>
#include <core/utils/ConfigurationMatchable.hpp>

ledger::core::ConfigurationMatchable::ConfigurationMatchable(const std::vector<std::string> &matchableKeys)
        : _matchableKeys(matchableKeys) {
}

bool
ledger::core::ConfigurationMatchable::match(const std::shared_ptr<ledger::core::api::DynamicObject> &configuration) {
    for (const auto& key : _matchableKeys) {
        auto valueType = configuration->getType(key).value_or(api::DynamicType::UNDEFINED);
        if (valueType == _configuration->getType(key).value_or(api::DynamicType::UNDEFINED)) {
            switch (valueType) {
                case api::DynamicType::OBJECT:
                    //TODO Make the matcher works with object
                    return false;
                    break;
                case api::DynamicType::INT32:
                    if (configuration->getInt(key) != _configuration->getInt(key)) {
                        return false;
                    }
                    break;
                case api::DynamicType::INT64:
                    if (configuration->getLong(key) != _configuration->getLong(key)) {
                        return false;
                    }
                    break;
                case api::DynamicType::DOUBLE:
                    if (configuration->getDouble(key) != _configuration->getDouble(key)) {
                        return false;
                    }
                    break;
                case api::DynamicType::BOOLEAN:
                    if (configuration->getBoolean(key) != _configuration->getBoolean(key)) {
                        return false;
                    }
                    break;
                case api::DynamicType::DATA:
                    if (configuration->getData(key) != _configuration->getData(key)) {
                        return false;
                    }
                    break;
                case api::DynamicType::ARRAY:
                    //TODO Make the matcher works with array
                    return false;
                    break;
                case api::DynamicType::STRING:
                    if (configuration->getString(key) != _configuration->getString(key)) {
                        return false;
                    }
                    break;
                case api::DynamicType::UNDEFINED:break;
            }
        }
    }
    return true;
}

void ledger::core::ConfigurationMatchable::setConfiguration(
        const std::shared_ptr<ledger::core::api::DynamicObject> &configuration) {
    _configuration = configuration;
}
