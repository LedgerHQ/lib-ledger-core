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
            if (_readOnly) return shared_from_this();
            DynamicValue v;
            v.type = api::DynamicType::STRING;
            v.string = value;
            _values[key] = v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::putInt(const std::string &key, int32_t value) {
            if (_readOnly) return shared_from_this();
            DynamicValue v;
            v.type = api::DynamicType::INT32;
            v.int32 = value;
            _values[key] = v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::putLong(const std::string &key, int64_t value) {
            if (_readOnly) return shared_from_this();
            DynamicValue v;
            v.type = api::DynamicType::INT64;
            v.int64 = value;
            _values[key] = v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::putDouble(const std::string &key, double value) {
            if (_readOnly) return shared_from_this();
            DynamicValue v;
            v.type = api::DynamicType::DOUBLE;
            v.doubleFloat = value;
            _values[key] = v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject>
        DynamicObject::putData(const std::string &key, const std::vector<uint8_t> &value) {
            if (_readOnly) return shared_from_this();
            DynamicValue v;
            v.type = api::DynamicType::DATA;
            v.bytes = value;
            _values[key] = v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::putBoolean(const std::string &key, bool value) {
            if (_readOnly) return shared_from_this();
            DynamicValue v;
            v.type = api::DynamicType::BOOLEAN;
            v.boolean = value;
            _values[key] = v;
            return shared_from_this();
        }

        optional<std::string> DynamicObject::getString(const std::string &key) {
            auto v = _values.lift(key);
            if (_values.empty() || v.getValue().type != api::DynamicType::STRING)
                return optional<std::string>();
            return optional<std::string>(v.getValue().string);
        }

        optional<int32_t> DynamicObject::getInt(const std::string &key) {
            return getNumber<int32_t>(key);
        }

        optional<int64_t> DynamicObject::getLong(const std::string &key) {
            return getNumber<int64_t>(key);
        }

        optional<double> DynamicObject::getDouble(const std::string &key) {
            return getNumber<double>(key);
        }

        optional<std::vector<uint8_t>> DynamicObject::getData(const std::string &key) {
            auto v = _values.lift(key);
            if (_values.empty() || v.getValue().type != api::DynamicType::DATA)
                return optional<std::vector<uint8_t >>();
            return optional<std::vector<uint8_t>>(v.getValue().bytes);
        }

        optional<bool> DynamicObject::getBoolean(const std::string &key) {
            return getNumber<bool>(key);
        }

        std::shared_ptr<api::DynamicObject>
        DynamicObject::putObject(const std::string &key, const std::shared_ptr<api::DynamicObject> &value) {
            if (_readOnly) return shared_from_this();
            DynamicValue v;
            v.type = api::DynamicType::OBJECT;
            v.object = std::static_pointer_cast<DynamicObject>(value);
            _values[key] = v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject>
        DynamicObject::putArray(const std::string &key, const std::shared_ptr<api::DynamicArray> &value) {
            if (_readOnly) return shared_from_this();
            DynamicValue v;
            v.type = api::DynamicType::ARRAY;
            v.array = std::static_pointer_cast<DynamicArray>(value);
            _values[key] = v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicObject::getObject(const std::string &key) {
            auto v = _values.lift(key);
            if (_values.empty() || v.getValue().type != api::DynamicType::OBJECT)
                return nullptr;
            return v.getValue().object;
        }

        std::shared_ptr<api::DynamicArray> DynamicObject::getArray(const std::string &key) {
            auto v = _values.lift(key);
            if (_values.empty() || v.getValue().type != api::DynamicType::ARRAY)
                return nullptr;
            return v.getValue().array;
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
            if (_values.empty())
                return optional<api::DynamicType>();
            return optional<api::DynamicType>(v.getValue().type);
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
                if (v.second.type == api::DynamicType::ARRAY) {
                    v.second.array->setReadOnly(enable);
                } else if (v.second.type == api::DynamicType::OBJECT ) {
                    v.second.object->setReadOnly(enable);
                }
            }
        }
    }
}