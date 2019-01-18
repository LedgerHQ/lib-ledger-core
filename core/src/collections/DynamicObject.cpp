/*
 *
 * DynamicObject
 * ledger-core
 *
 * Created by Pierre Pollastri on 08/03/2017.
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

#include "DynamicObject.hpp"
#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/memory.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

namespace ledger {
    namespace core {

        std::shared_ptr<api::DynamicObject> DynamicObject::putString(const std::string &key, const std::string &value) {
            if (!_readOnly) {
                _values[key] = DynamicValue(value);
            }

            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::putInt(const std::string &key, int32_t value) {
            if (!_readOnly) {
                _values[key] = DynamicValue(value);
            }

            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::putLong(const std::string &key, int64_t value) {
            if (!_readOnly) {
                _values[key] = DynamicValue(value);
            }

            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::putDouble(const std::string &key, double value) {
            if (!_readOnly) {
                _values[key] = DynamicValue(value);
            }

            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject>
        DynamicObject::putData(const std::string &key, const std::vector<uint8_t> &value) {
            if (!_readOnly) {
                _values[key] = DynamicValue(value);
            }

            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::putBoolean(const std::string &key, bool value) {
            if (!_readOnly) {
                _values[key] = DynamicValue(value);
            }

            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject>
        DynamicObject::putObject(const std::string &key, const std::shared_ptr<api::DynamicObject> &value) {
            if (!_readOnly) {
                _values[key] = DynamicValue(std::static_pointer_cast<DynamicObject>(value));
            }

            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject>
        DynamicObject::putArray(const std::string &key, const std::shared_ptr<api::DynamicArray> &value) {
            if (!_readOnly) {
                _values[key] = DynamicValue(std::static_pointer_cast<DynamicArray>(value));
            }

            return shared_from_this();
        }

        optional<std::string> DynamicObject::getString(const std::string &key) {
            const auto v = optional<DynamicValue>(_values.lift(key));

            if (v) {
                return *v->asStr();
            } else {
                return optional<std::string>();
            }
        }

        optional<int32_t> DynamicObject::getInt(const std::string &key) {
            const auto v = optional<DynamicValue>(_values.lift(key));

            if (v) {
                return v->asInt32();
            } else {
                return optional<int32_t>();
            }
        }

        optional<int64_t> DynamicObject::getLong(const std::string &key) {
            const auto v = optional<DynamicValue>(_values.lift(key));

            if (v) {
                return v->asInt64();
            } else {
                return optional<int64_t>();
            }
        }

        optional<double> DynamicObject::getDouble(const std::string &key) {
            const auto v = optional<DynamicValue>(_values.lift(key));

            if (v) {
                return v->asDouble();
            } else {
                return optional<double>();
            }
        }

        optional<bool> DynamicObject::getBoolean(const std::string &key) {
            const auto v = optional<DynamicValue>(_values.lift(key));

            if (v) {
                return v->asBool();
            } else {
                return optional<bool>();
            }
        }

        optional<std::vector<uint8_t>> DynamicObject::getData(const std::string &key) {
            const auto v = optional<DynamicValue>(_values.lift(key));

            if (v) {
                return *v->asData();
            } else {
                return optional<std::vector<uint8_t>>();
            }
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::getObject(const std::string &key) {
            const auto v = optional<DynamicValue>(_values.lift(key));

            if (v) {
                return *v->asObject();
            } else {
                return nullptr;
            }
        }

        std::shared_ptr<api::DynamicArray> DynamicObject::getArray(const std::string &key) {
            const auto v = optional<DynamicValue>(_values.lift(key));

            if (v) {
                return *v->asArray();
            } else {
                return nullptr;
            }
        }

        bool DynamicObject::contains(const std::string &key) {
            return _values.contains(key);
        }

        bool DynamicObject::remove(const std::string &key) {
            return _values.remove(key);
        }

        std::vector<std::string> DynamicObject::getKeys() {
            return _values.getKeys().getContainer();
        }

        optional<api::DynamicType> DynamicObject::getType(const std::string &key) {
            auto v = _values.lift(key);
            if (_values.empty() || !v.hasValue())
                return optional<api::DynamicType>();
            return optional<api::DynamicType>(v.getValue().getType());
        }

        std::string DynamicObject::dump() {
            std::stringstream ss;
            dump(ss, 0);
            return ss.str();
        }

        std::vector<uint8_t> DynamicObject::serialize() {
            std::stringstream is;
            ::cereal::PortableBinaryOutputArchive archive(is);
            archive(shared_from_this());
            auto savedState = is.str();
            return std::vector<uint8_t>((const uint8_t *)savedState.data(),(const uint8_t *)savedState.data() + savedState.length());
        }

        std::shared_ptr<api::DynamicObject> api::DynamicObject::load(const std::vector<uint8_t> &serialized) {
            std::shared_ptr<ledger::core::DynamicObject> object;
            boost::iostreams::array_source my_vec_source(reinterpret_cast<const char*>(&serialized[0]), serialized.size());
            boost::iostreams::stream<boost::iostreams::array_source> is(my_vec_source);
            ::cereal::PortableBinaryInputArchive archive(is);
            archive(object);
            return object;
        }

        std::shared_ptr<api::DynamicObject> api::DynamicObject::newInstance() {
            return std::make_shared<ledger::core::DynamicObject>();
        }

        std::ostream &DynamicObject::dump(std::ostream &ss, int depth) const {
            for (auto& item : _values.getContainer()) {
                ss << (" "_S * depth).str() << '"' << item.first << "\" -> ";
                item.second.dump(ss, depth);
                ss << std::endl;
            }
            return ss;
        }

        int64_t DynamicObject::size() {
            return _values.getContainer().size();
        }

        bool DynamicObject::isReadOnly() {
            return _readOnly;
        }

        void DynamicObject::setReadOnly(bool enable) {
            _readOnly = enable;

            for (auto& v : _values.getContainer()) {
                // try to set the read-only attribute on the contained value as an array, and if it
                // fails, try to do the same as if it were an object
                auto array = v.second.asArray();
                if (array) {
                    (*array)->setReadOnly(enable);
                } else {
                    auto object = v.second.asObject();
                    if (object) {
                        (*object)->setReadOnly(enable);
                    }
                }
            }
        }
    }
}
