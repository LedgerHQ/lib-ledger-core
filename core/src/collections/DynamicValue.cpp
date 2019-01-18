/*
 *
 * DynamicValue
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

#include "DynamicValue.hpp"

#include "DynamicObject.hpp"
#include "DynamicArray.hpp"
#include <cereal/types/memory.hpp>
#include "../utils/hex.h"

namespace ledger {
    namespace core {
        DynamicValue::DynamicValue(): type(api::DynamicType::UNDEFINED) {
        }

        DynamicValue::DynamicValue(const DynamicValue& rhs): type(rhs.type), data(rhs.data) {
        }

        DynamicValue::DynamicValue(const std::string& x): type(api::DynamicType::STRING) {
            data = x;
        }

        DynamicValue::DynamicValue(const std::vector<uint8_t>& x): type(api::DynamicType::DATA) {
            data = x;
        }

        DynamicValue::DynamicValue(bool x): type(api::DynamicType::BOOLEAN) {
            data = x;
        }

        DynamicValue::DynamicValue(int32_t x): type(api::DynamicType::INT32) {
            data = x;
        }

        DynamicValue::DynamicValue(int64_t x): type(api::DynamicType::INT64) {
            data = x;
        }

        DynamicValue::DynamicValue(double x): type(api::DynamicType::DOUBLE) {
            data = x;
        }

        DynamicValue::DynamicValue(const std::shared_ptr<DynamicArray>& x): type(api::DynamicType::ARRAY) {
            data = x;
        }

        DynamicValue::DynamicValue(const std::shared_ptr<DynamicObject>& x): type(api::DynamicType::OBJECT) {
            data = x;
        }

        template<> void DynamicValue::serialize<cereal::PortableBinaryOutputArchive>(cereal::PortableBinaryOutputArchive& ar) {
            ar(type);

            switch (type) {
                case api::DynamicType::STRING:
                    ar(boost::get<std::string>(data));
                    break;

                case api::DynamicType::DATA:
                    ar(boost::get<std::vector<uint8_t>>(data));
                    break;

                case api::DynamicType::BOOLEAN:
                    ar(boost::get<bool>(data));
                    break;

                case api::DynamicType::INT32:
                    ar(boost::get<int32_t>(data));
                    break;

                case api::DynamicType::INT64:
                    ar(boost::get<int64_t>(data));
                    break;

                case api::DynamicType::DOUBLE:
                    ar(boost::get<double>(data));
                    break;

                case api::DynamicType::ARRAY:
                    ar(boost::get<std::shared_ptr<DynamicArray>>(data));
                    break;

                case api::DynamicType::OBJECT:
                    ar(boost::get<std::shared_ptr<DynamicObject>>(data));
                    break;

                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        template<> void DynamicValue::serialize<cereal::PortableBinaryInputArchive>(cereal::PortableBinaryInputArchive& ar) {
            ar(type);

            switch (type) {
                case api::DynamicType::STRING:
                    ar(boost::get<std::string>(data));
                    break;

                case api::DynamicType::DATA:
                    ar(boost::get<std::vector<uint8_t>>(data));
                    break;

                case api::DynamicType::BOOLEAN:
                    ar(boost::get<bool>(data));
                    break;

                case api::DynamicType::INT32:
                    ar(boost::get<int32_t>(data));
                    break;

                case api::DynamicType::INT64:
                    ar(boost::get<int64_t>(data));
                    break;

                case api::DynamicType::DOUBLE:
                    ar(boost::get<double>(data));
                    break;

                case api::DynamicType::ARRAY:
                    ar(boost::get<std::shared_ptr<DynamicArray>>(data));
                    break;

                case api::DynamicType::OBJECT:
                    ar(boost::get<std::shared_ptr<DynamicObject>>(data));
                    break;

                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        template<> void DynamicValue::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& ar) {
            ar(type);

            switch (type) {
                case api::DynamicType::STRING:
                    ar(boost::get<std::string>(data));
                    break;

                case api::DynamicType::DATA:
                    ar(boost::get<std::vector<uint8_t>>(data));
                    break;

                case api::DynamicType::BOOLEAN:
                    ar(boost::get<bool>(data));
                    break;

                case api::DynamicType::INT32:
                    ar(boost::get<int32_t>(data));
                    break;

                case api::DynamicType::INT64:
                    ar(boost::get<int64_t>(data));
                    break;

                case api::DynamicType::DOUBLE:
                    ar(boost::get<double>(data));
                    break;

                case api::DynamicType::ARRAY:
                    ar(boost::get<std::shared_ptr<DynamicArray>>(data));
                    break;

                case api::DynamicType::OBJECT:
                    ar(boost::get<std::shared_ptr<DynamicObject>>(data));
                    break;

                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        template<> void DynamicValue::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& ar) {
            ar(type);

            switch (type) {
                case api::DynamicType::STRING:
                    ar(boost::get<std::string>(data));
                    break;

                case api::DynamicType::DATA:
                    ar(boost::get<std::vector<uint8_t>>(data));
                    break;

                case api::DynamicType::BOOLEAN:
                    ar(boost::get<bool>(data));
                    break;

                case api::DynamicType::INT32:
                    ar(boost::get<int32_t>(data));
                    break;

                case api::DynamicType::INT64:
                    ar(boost::get<int64_t>(data));
                    break;

                case api::DynamicType::DOUBLE:
                    ar(boost::get<double>(data));
                    break;

                case api::DynamicType::ARRAY:
                    ar(boost::get<std::shared_ptr<DynamicArray>>(data));
                    break;

                case api::DynamicType::OBJECT:
                    ar(boost::get<std::shared_ptr<DynamicObject>>(data));
                    break;

                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        std::string DynamicValue::dump() const {
            switch (type) {
                case api::DynamicType::STRING:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), boost::get<std::string>(data));

                case api::DynamicType::DATA:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), hex::toString(boost::get<std::vector<uint8_t>>(data)));

                case api::DynamicType::BOOLEAN:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), boost::get<bool>(data));

                case api::DynamicType::INT32:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), boost::get<int32_t>(data));

                case api::DynamicType::INT64:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), boost::get<int64_t>(data));

                case api::DynamicType::DOUBLE:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), boost::get<double>(data));

                case api::DynamicType::ARRAY:
                    return fmt::format("({})\n", api::to_string(type), boost::get<std::shared_ptr<DynamicArray>>(data)->dump());

                case api::DynamicType::OBJECT:
                    return fmt::format("({})\n", api::to_string(type), boost::get<std::shared_ptr<DynamicObject>>(data)->dump());

                case api::DynamicType::UNDEFINED:
                    return fmt::format("<[{}]>\n", api::to_string(type));
            }
        }

        std::ostream& DynamicValue::dump(std::ostream &ss, int depth) const {
            switch (type) {
                case api::DynamicType::STRING:
                    return ss << fmt::format("[{}] {}", api::to_string(type), boost::get<std::string>(data));

                case api::DynamicType::DATA:
                    return ss << fmt::format("[{}] {}", api::to_string(type), hex::toString(boost::get<std::vector<uint8_t>>(data)));

                case api::DynamicType::BOOLEAN:
                    return ss << fmt::format("[{}] {}", api::to_string(type), boost::get<bool>(data));

                case api::DynamicType::INT32:
                    return ss << fmt::format("[{}] {}", api::to_string(type), boost::get<int32_t>(data));

                case api::DynamicType::INT64:
                    return ss << fmt::format("[{}] {}", api::to_string(type), boost::get<int64_t>(data));

                case api::DynamicType::DOUBLE:
                    return ss << fmt::format("[{}] {}", api::to_string(type), boost::get<double>(data));

                case api::DynamicType::ARRAY:
                    ss << fmt::format("[{}] (\n", api::to_string(type));
                    boost::get<std::shared_ptr<DynamicArray>>(data)->dump(ss, depth + 1);
                    ss << (" "_S * depth).str() << ")";
                    return ss;

                case api::DynamicType::OBJECT:
                    ss << fmt::format("[{}] (", api::to_string(type)) << std::endl;
                    boost::get<std::shared_ptr<DynamicObject>>(data)->dump(ss, depth + 1);
                    ss << (" "_S * depth).str() << ")";
                    return ss;

                case api::DynamicType::UNDEFINED:
                    return ss << fmt::format("[{}]", api::to_string(type));
            }
        }

        api::DynamicType DynamicValue::getType() const {
            return type;
        }

        DynamicValue& DynamicValue::operator=(const std::string& rhs) {
            data = rhs;
            type = api::DynamicType::STRING;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(std::string&& rhs) {
            data = std::move(rhs);
            type = api::DynamicType::STRING;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(const std::vector<uint8_t>& rhs) {
            data = rhs;
            type = api::DynamicType::DATA;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(std::vector<uint8_t>&& rhs) {
            data = std::move(rhs);
            type = api::DynamicType::DATA;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(bool rhs) {
            data = rhs;
            type = api::DynamicType::BOOLEAN;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(int32_t rhs) {
            data = rhs;
            type = api::DynamicType::INT32;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(int64_t rhs) {
            data = rhs;
            type = api::DynamicType::INT64;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(double rhs) {
            data = rhs;
            type = api::DynamicType::DOUBLE;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(const std::shared_ptr<DynamicArray>& rhs) {
            data = rhs;
            type = api::DynamicType::ARRAY;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(const std::shared_ptr<DynamicObject>& rhs) {
            data = rhs;
            type = api::DynamicType::OBJECT;
            return *this;
        }

        optional<std::string&> DynamicValue::asStr() {
            if (type == api::DynamicType::STRING) {
                return boost::get<std::string>(data);
            } else {
                return optional<std::string&>();
            }
        }

        optional<const std::string&> DynamicValue::asStr() const {
            if (type == api::DynamicType::STRING) {
                return boost::get<std::string>(data);
            } else {
                return optional<const std::string&>();
            }
        }

        optional<std::vector<uint8_t>&> DynamicValue::asData() {
            if (type == api::DynamicType::DATA) {
                return boost::get<std::vector<uint8_t>>(data);
            } else {
                return optional<std::vector<uint8_t>&>();
            }
        }

        optional<const std::vector<uint8_t>&> DynamicValue::asData() const {
            if (type == api::DynamicType::DATA) {
                return boost::get<std::vector<uint8_t>>(data);
            } else {
                return optional<const std::vector<uint8_t>&>();
            }
        }

        optional<bool&> DynamicValue::asBool() {
            if (type == api::DynamicType::BOOLEAN) {
                return boost::get<bool>(data);
            } else {
                return optional<bool&>();
            }
        }

        optional<bool> DynamicValue::asBool() const {
            if (type == api::DynamicType::BOOLEAN) {
                return boost::get<bool>(data);
            } else {
                return optional<bool>();
            }
        }

        optional<int32_t&> DynamicValue::asInt32() {
            if (type == api::DynamicType::INT32) {
                return boost::get<int32_t>(data);
            } else {
                return optional<int32_t&>();
            }
        }

        optional<int32_t> DynamicValue::asInt32() const {
            if (type == api::DynamicType::INT32) {
                return boost::get<int32_t>(data);
            } else {
                return optional<int32_t>();
            }
        }

        optional<int64_t&> DynamicValue::asInt64() {
            if (type == api::DynamicType::INT64) {
                return boost::get<int64_t>(data);
            } else {
                return optional<int64_t&>();
            }
        }

        optional<int64_t> DynamicValue::asInt64() const {
            if (type == api::DynamicType::INT64) {
                return boost::get<int64_t>(data);
            } else {
                return optional<int64_t>();
            }
        }

        optional<double&> DynamicValue::asDouble() {
            if (type == api::DynamicType::DOUBLE) {
                return boost::get<double>(data);
            } else {
                return optional<double&>();
            }
        }

        optional<double> DynamicValue::asDouble() const {
            if (type == api::DynamicType::DOUBLE) {
                return boost::get<double>(data);
            } else {
                return optional<double>();
            }
        }

        optional<std::shared_ptr<DynamicArray>&> DynamicValue::asArray() {
            if (type == api::DynamicType::ARRAY) {
                return boost::get<std::shared_ptr<DynamicArray>>(data);
            } else {
                return optional<std::shared_ptr<DynamicArray>&>();
            }
        }

        optional<const std::shared_ptr<DynamicArray>&> DynamicValue::asArray() const {
            if (type == api::DynamicType::ARRAY) {
                return boost::get<std::shared_ptr<DynamicArray>>(data);
            } else {
                return optional<const std::shared_ptr<DynamicArray>&>();
            }
        }

        optional<std::shared_ptr<DynamicObject>&> DynamicValue::asObject() {
            if (type == api::DynamicType::OBJECT) {
                return boost::get<std::shared_ptr<DynamicObject>>(data);
            } else {
                return optional<std::shared_ptr<DynamicObject>&>();
            }
        }

        optional<const std::shared_ptr<DynamicObject>&> DynamicValue::asObject() const {
            if (type == api::DynamicType::OBJECT) {
                return boost::get<std::shared_ptr<DynamicObject>>(data);
            } else {
                return optional<const std::shared_ptr<DynamicObject>&>();
            }
        }
    }
}
