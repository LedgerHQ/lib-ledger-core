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
        DynamicValue::DynamicValue(const DynamicValue& rhs): data(rhs.data) {
        }

        DynamicValue::DynamicValue(const char* x) {
            data = std::string(x);
        }

        DynamicValue::DynamicValue(const std::string& x) {
            data = x;
        }

        DynamicValue::DynamicValue(const std::vector<uint8_t>& x) {
            data = x;
        }

        DynamicValue::DynamicValue(bool x) {
            data = x;
        }

        DynamicValue::DynamicValue(int32_t x) {
            data = x;
        }

        DynamicValue::DynamicValue(int64_t x) {
            data = x;
        }

        DynamicValue::DynamicValue(double x) {
            data = x;
        }

        DynamicValue::DynamicValue(const std::shared_ptr<DynamicArray>& x) {
            data = x;
        }

        DynamicValue::DynamicValue(const std::shared_ptr<DynamicObject>& x) {
            data = x;
        }

        template<> void DynamicValue::serialize<cereal::PortableBinaryOutputArchive>(cereal::PortableBinaryOutputArchive& ar) {
            auto type = getType();
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
            auto type = api::DynamicType::UNDEFINED;
            ar(type);

            switch (type) {
                case api::DynamicType::STRING:
                    {
                        std::string x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::DATA:
                    {
                        std::vector<uint8_t> x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::BOOLEAN:
                    {
                        bool x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::INT32:
                    {
                        int32_t x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::INT64:
                    {
                        int64_t x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::DOUBLE:
                    {
                        double x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::ARRAY:
                    {
                        std::shared_ptr<DynamicArray> x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::OBJECT:
                    {
                        std::shared_ptr<DynamicObject> x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        template<> void DynamicValue::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& ar) {
            auto type = getType();
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
            auto type = api::DynamicType::UNDEFINED;
            ar(type);

            switch (type) {
                case api::DynamicType::STRING:
                    {
                        std::string x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::DATA:
                    {
                        std::vector<uint8_t> x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::BOOLEAN:
                    {
                        bool x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::INT32:
                    {
                        int32_t x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::INT64:
                    {
                        int64_t x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::DOUBLE:
                    {
                        double x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::ARRAY:
                    {
                        std::shared_ptr<DynamicArray> x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::OBJECT:
                    {
                        std::shared_ptr<DynamicObject> x;
                        ar(x);
                        data = x;
                    }
                    break;

                case api::DynamicType::UNDEFINED:
                    break;
            }
        }

        std::string DynamicValue::dump() const {
            auto type = getType();

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
            auto type = getType();

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
            struct ReifyType: boost::static_visitor<api::DynamicType> {
                api::DynamicType operator()(int32_t) const {
                    return api::DynamicType::INT32;
                }

                api::DynamicType operator()(int64_t) const {
                    return api::DynamicType::INT64;
                }

                api::DynamicType operator()(double) const {
                    return api::DynamicType::DOUBLE;
                }

                api::DynamicType operator()(bool) const {
                    return api::DynamicType::BOOLEAN;
                }

                api::DynamicType operator()(const std::string&) const {
                    return api::DynamicType::STRING;
                }

                api::DynamicType operator()(const std::vector<uint8_t>&) const {
                    return api::DynamicType::DATA;
                }

                api::DynamicType operator()(const std::shared_ptr<DynamicArray>&) const {
                    return api::DynamicType::ARRAY;
                }

                api::DynamicType operator()(const std::shared_ptr<DynamicObject>&) const {
                    return api::DynamicType::OBJECT;
                }
            };

            static const ReifyType visitor;
            return boost::apply_visitor(visitor, data);
        }

        DynamicValue& DynamicValue::operator=(const std::string& rhs) {
            data = rhs;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(std::string&& rhs) {
            data = std::move(rhs);
            return *this;
        }

        DynamicValue& DynamicValue::operator=(const std::vector<uint8_t>& rhs) {
            data = rhs;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(std::vector<uint8_t>&& rhs) {
            data = std::move(rhs);
            return *this;
        }

        DynamicValue& DynamicValue::operator=(bool rhs) {
            data = rhs;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(int32_t rhs) {
            data = rhs;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(int64_t rhs) {
            data = rhs;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(double rhs) {
            data = rhs;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(const std::shared_ptr<DynamicArray>& rhs) {
            data = rhs;
            return *this;
        }

        DynamicValue& DynamicValue::operator=(const std::shared_ptr<DynamicObject>& rhs) {
            data = rhs;
            return *this;
        }
    }
}
