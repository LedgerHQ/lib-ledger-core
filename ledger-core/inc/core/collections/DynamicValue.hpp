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

#pragma once

#include <vector>
#include <boost/variant.hpp>

#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary.hpp>
#include <core/api/DynamicType.hpp>
#include <core/utils/Optional.hpp>

namespace ledger {
    namespace core {
        class DynamicArray;
        class DynamicObject;

        struct DynamicValue {
            // Mandatory default constructor for it to be default-constructible in (e.g.) maps.
            explicit DynamicValue() = default;
            explicit DynamicValue(const DynamicValue& rhs);
            explicit DynamicValue(const char* x);
            explicit DynamicValue(const std::string& x);
            explicit DynamicValue(const std::vector<uint8_t>& x);
            explicit DynamicValue(bool x);
            explicit DynamicValue(int32_t x);
            explicit DynamicValue(int64_t x);
            explicit DynamicValue(double x);
            explicit DynamicValue(const std::shared_ptr<DynamicArray>& x);
            explicit DynamicValue(const std::shared_ptr<DynamicObject>& x);

            template <class Archive>
            void serialize(Archive& ar) {
            }

            std::string dump() const;
            std::ostream& dump(std::ostream& ss, int depth) const;

            /// Get the underlying type.
            api::DynamicType getType() const;

            /// Change the value to a std::string by copying.
            DynamicValue& operator=(const std::string& rhs);
            /// Change the value to a std::string by moving in.
            DynamicValue& operator=(std::string&& rhs);

            /// Change the value to a std::vector<uint8_t> by copying.
            DynamicValue& operator=(const std::vector<uint8_t>& rhs);
            /// Change the value to a std::vector<uint8_t> by moving in.
            DynamicValue& operator=(std::vector<uint8_t>&& rhs);

            /// Change the value to a bool by copying.
            DynamicValue& operator=(bool rhs);

            /// Change the value to a int32_t by copying.
            DynamicValue& operator=(int32_t rhs);

            /// Change the value to a int64_t by copying.
            DynamicValue& operator=(int64_t rhs);

            /// Change the value to a double by copying.
            DynamicValue& operator=(double rhs);

            /// Change the value to a DynamicArray by copying.
            DynamicValue& operator=(const std::shared_ptr<DynamicArray>& rhs);

            /// Change the value to a DynamicObject by copying.
            DynamicValue& operator=(const std::shared_ptr<DynamicObject>& rhs);

            /// Try to get the value as if it were of a given type.
            template <typename T>
            optional<T> get() const {
                static const OptionalVisitor<T> visitor;
                return boost::apply_visitor(visitor, data);
            }

        private:
            // Data. We cannot use a union because of C++ restrictions on non-trivial ctors and
            // dtors. SFOL
            boost::variant<int32_t, int64_t, bool, double, std::string, std::vector<uint8_t>,
                           std::shared_ptr<DynamicArray>, std::shared_ptr<DynamicObject>> data;

            // A visitor used to cast from DynamicValue to typed optional values.
            template <typename T>
            struct OptionalVisitor: boost::static_visitor<optional<T>> {
                optional<T> operator()(T x) const {
                    return x;
                }

                template <typename Q>
                optional<T> operator()(const Q&) const {
                    return optional<T>();
                }
            };

            // A helper function to serialize.
            template <typename Archive>
            void out_serialize(Archive& ar) {
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

            // A helper function to deserialize.
            template <typename Archive>
            void in_serialize(Archive& ar) {
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
        };

        template<> void DynamicValue::serialize<cereal::PortableBinaryOutputArchive>(cereal::PortableBinaryOutputArchive&);
        template<> void DynamicValue::serialize<cereal::PortableBinaryInputArchive>(cereal::PortableBinaryInputArchive&);
        template<> void DynamicValue::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&);
        template<> void DynamicValue::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);
    }
}