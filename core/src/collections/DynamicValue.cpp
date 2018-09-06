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

        template<> void DynamicValue::serialize<cereal::PortableBinaryOutputArchive>( cereal::PortableBinaryOutputArchive & ar) {
            ar(type);
            switch (type) {
                case api::DynamicType::ARRAY:
                    ar(array);
                    break;
                case api::DynamicType::OBJECT:
                    ar(object);
                    break;
                case api::DynamicType::BOOLEAN:
                    ar(boolean);
                    break;
                case api::DynamicType::DATA:
                    ar(bytes);
                    break;
                case api::DynamicType::DOUBLE:
                    ar(doubleFloat);
                    break;
                case api::DynamicType::INT32:
                    ar(int32);
                    break;
                case api::DynamicType::INT64:
                    ar(int64);
                    break;
                case api::DynamicType::STRING:
                    ar(string);
                    break;
                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        template<> void DynamicValue::serialize<cereal::PortableBinaryInputArchive>( cereal::PortableBinaryInputArchive & ar) {
            ar(type);
            switch (type) {
                case api::DynamicType::ARRAY:
                    ar(array);
                    break;
                case api::DynamicType::OBJECT:
                    ar(object);
                    break;
                case api::DynamicType::BOOLEAN:
                    ar(boolean);
                    break;
                case api::DynamicType::DATA:
                    ar(bytes);
                    break;
                case api::DynamicType::DOUBLE:
                    ar(doubleFloat);
                    break;
                case api::DynamicType::INT32:
                    ar(int32);
                    break;
                case api::DynamicType::INT64:
                    ar(int64);
                    break;
                case api::DynamicType::STRING:
                    ar(string);
                    break;
                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        template<> void DynamicValue::serialize<cereal::BinaryOutputArchive>( cereal::BinaryOutputArchive & ar) {
            ar(type);
            switch (type) {
                case api::DynamicType::ARRAY:
                    ar(array);
                    break;
                case api::DynamicType::OBJECT:
                    ar(object);
                    break;
                case api::DynamicType::BOOLEAN:
                    ar(boolean);
                    break;
                case api::DynamicType::DATA:
                    ar(bytes);
                    break;
                case api::DynamicType::DOUBLE:
                    ar(doubleFloat);
                    break;
                case api::DynamicType::INT32:
                    ar(int32);
                    break;
                case api::DynamicType::INT64:
                    ar(int64);
                    break;
                case api::DynamicType::STRING:
                    ar(string);
                    break;
                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        template<> void DynamicValue::serialize<cereal::BinaryInputArchive>( cereal::BinaryInputArchive &ar) {
            ar(type);
            switch (type) {
                case api::DynamicType::ARRAY:
                    ar(array);
                    break;
                case api::DynamicType::OBJECT:
                    ar(object);
                    break;
                case api::DynamicType::BOOLEAN:
                    ar(boolean);
                    break;
                case api::DynamicType::DATA:
                    ar(bytes);
                    break;
                case api::DynamicType::DOUBLE:
                    ar(doubleFloat);
                    break;
                case api::DynamicType::INT32:
                    ar(int32);
                    break;
                case api::DynamicType::INT64:
                    ar(int64);
                    break;
                case api::DynamicType::STRING:
                    ar(string);
                    break;
                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        std::string DynamicValue::dump() const {
            switch (type) {
                case api::DynamicType::ARRAY:
                    return fmt::format("({})\n", api::to_string(type), array->dump());
                case api::DynamicType::OBJECT:
                    return fmt::format("({})\n", api::to_string(type), object->dump());
                case api::DynamicType::BOOLEAN:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), boolean);
                case api::DynamicType::DATA:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), hex::toString(bytes));
                case api::DynamicType::DOUBLE:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), doubleFloat);
                case api::DynamicType::INT32:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), int32);
                case api::DynamicType::INT64:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), int64);
                case api::DynamicType::STRING:
                    return fmt::format("<[{}] {}>\n", api::to_string(type), string);
                case api::DynamicType::UNDEFINED:
                    return fmt::format("<[{}]>\n", api::to_string(type));
            }
        }

        std::ostream &DynamicValue::dump(std::ostream &ss, int depth) const {
            switch (type) {
                case api::DynamicType::ARRAY:
                    ss << fmt::format("[{}] (\n", api::to_string(type));
                    array->dump(ss, depth + 1);
                    ss << (" "_S * depth).str() << ")";
                    return ss;
                case api::DynamicType::OBJECT:
                    ss << fmt::format("[{}] (", api::to_string(type)) << std::endl;
                    object->dump(ss, depth + 1);
                    ss << (" "_S * depth).str() << ")";
                    return ss;
                case api::DynamicType::BOOLEAN:
                    return ss << fmt::format("[{}] {}", api::to_string(type), boolean);
                case api::DynamicType::DATA:
                    return ss << fmt::format("[{}] {}", api::to_string(type), hex::toString(bytes));
                case api::DynamicType::DOUBLE:
                    return ss << fmt::format("[{}] {}", api::to_string(type), doubleFloat);
                case api::DynamicType::INT32:
                    return ss << fmt::format("[{}] {}", api::to_string(type), int32);
                case api::DynamicType::INT64:
                    return ss << fmt::format("[{}] {}", api::to_string(type), int64);
                case api::DynamicType::STRING:
                    return ss << fmt::format("[{}] {}", api::to_string(type), string);
                case api::DynamicType::UNDEFINED:
                    return ss << fmt::format("[{}]", api::to_string(type));
            }
        }
    }
}