/*
 *
 * DynamicArray
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
#include "DynamicArray.hpp"
#include <iterator>
#include "DynamicObject.hpp"
#include "DynamicValue.hpp"
#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/memory.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

namespace ledger {
    namespace core {

        int64_t DynamicArray::size() {
            return (int64_t)_values.size();
        }

        optional<std::string> DynamicArray::getString(int64_t index) {
            if (index >= size())
                return optional<std::string>();
            auto value = _values[index];
            if (value.type != api::DynamicType::STRING)
                return optional<std::string>();
            return optional<std::string>(value.string);
        }

        optional<int32_t> DynamicArray::getInt(int64_t index) {
            if (index >= size())
                return optional<int32_t>();
            auto value = _values[index];
            if (value.type != api::DynamicType::INT32)
                return optional<int32_t>();
            return optional<int32_t>(value.int32);
        }

        optional<int64_t> DynamicArray::getLong(int64_t index) {
            if (index >= size())
                return optional<int64_t>();
            auto value = _values[index];
            if (value.type != api::DynamicType::INT64)
                return optional<int64_t>();
            return optional<int64_t>(value.int64);
        }

        optional<double> DynamicArray::getDouble(int64_t index) {
            if (index >= size())
                return optional<double>();
            auto value = _values[index];
            if (value.type != api::DynamicType::DOUBLE)
                return optional<double>();
            return optional<double>(value.doubleFloat);
        }

        optional<std::vector<uint8_t>> DynamicArray::getData(int64_t index) {
            if (index >= size())
                return optional<std::vector<uint8_t>>();
            auto value = _values[index];
            if (value.type != api::DynamicType::DATA)
                return optional<std::vector<uint8_t>>();
            return optional<std::vector<uint8_t>>(value.bytes);
        }

        optional<bool> DynamicArray::getBoolean(int64_t index) {
            if (index >= size())
                return optional<bool>();
            auto value = _values[index];
            if (value.type != api::DynamicType::BOOLEAN)
                return optional<bool>();
            return optional<bool>(value.boolean);
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::pushInt(int32_t value) {
            DynamicValue v;
            v.type = api::DynamicType::INT32;
            v.int32 = value;
            _values += v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicObject> DynamicArray::getObject(int64_t index) {
            if (index >= size())
                return nullptr;
            auto value = _values[index];
            if (value.type != api::DynamicType::OBJECT)
                return nullptr;
            return value.object;
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::getArray(int64_t index) {
            if (index >= size())
                return nullptr;
            auto value = _values[index];
            if (value.type != api::DynamicType::ARRAY)
                return nullptr;
            return value.array;
        }

        optional<api::DynamicType> DynamicArray::getType(int64_t index) {
            if (index >= size())
                return optional<api::DynamicType>();
            return _values[index].type;
        }

        bool DynamicArray::remove(int64_t index) {
            if (index > size())
                return false;
            _values.remove(index);
            return true;
        }

        std::string DynamicArray::dump() {
            std::stringstream ss;
            dump(ss, 0);
            return ss.str();
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::pushLong(int64_t value) {
            DynamicValue v;
            v.type = api::DynamicType::INT64;
            v.int64 = value;
            _values += v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::pushString(const std::string &value) {
            DynamicValue v;
            v.type = api::DynamicType::STRING;
            v.string = value;
            _values += v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::pushDouble(double value) {
            DynamicValue v;
            v.type = api::DynamicType::DOUBLE;
            v.doubleFloat = value;
            _values += v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::pushData(const std::vector<uint8_t> &value) {
            DynamicValue v;
            v.type = api::DynamicType::DATA;
            v.bytes = value;
            _values += v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::pushBoolean(bool value) {
            DynamicValue v;
            v.type = api::DynamicType::BOOLEAN;
            v.boolean = value;
            _values += v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::pushObject(const std::shared_ptr<api::DynamicObject> &value) {
            DynamicValue v;
            v.type = api::DynamicType::OBJECT;
            v.object = std::static_pointer_cast<DynamicObject>(value);
            _values += v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::pushArray(const std::shared_ptr<api::DynamicArray> &value) {
            DynamicValue v;
            v.type = api::DynamicType::ARRAY;
            v.array = std::static_pointer_cast<DynamicArray>(value);
            _values += v;
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicArray> DynamicArray::concat(const std::shared_ptr<api::DynamicArray> &array) {
            auto a = std::static_pointer_cast<DynamicArray>(array);
            for (auto& v : a->_values.getContainer()) {
                _values += v;
            }
            return shared_from_this();
        }

        std::shared_ptr<api::DynamicArray> api::DynamicArray::newInstance() {
            return std::make_shared<ledger::core::DynamicArray>();
        }

        std::vector<uint8_t> DynamicArray::serialize() {
            std::stringstream is;
            ::cereal::PortableBinaryOutputArchive archive(is);
            archive(shared_from_this());
            auto savedState = is.str();
            return std::vector<uint8_t>((const uint8_t *)savedState.data(),(const uint8_t *)savedState.data() + savedState.length());
        }

        std::ostream &DynamicArray::dump(std::ostream &ss, int depth) const {
            auto index = 0;
            for (auto& item : _values.getContainer()) {
                ss << (" "_S * depth).str() << index << " -> ";
                item.dump(ss, depth);
                ss << std::endl;
                index += 1;
            }
            return ss;
        }

        std::shared_ptr<api::DynamicArray> api::DynamicArray::load(const std::vector<uint8_t> &serialized) {
            std::shared_ptr<ledger::core::DynamicArray> array;
            boost::iostreams::array_source my_vec_source(reinterpret_cast<const char*>(&serialized[0]), serialized.size());
            boost::iostreams::stream<boost::iostreams::array_source> is(my_vec_source);
            ::cereal::PortableBinaryInputArchive archive(is);
            archive(array);
            return array;
        }

    }
}