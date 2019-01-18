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

#ifndef LEDGER_CORE_DYNAMICVALUE_HPP
#define LEDGER_CORE_DYNAMICVALUE_HPP

#include "../api/DynamicType.hpp"
#include <boost/variant.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary.hpp>
#include <vector>
#include <utils/optional.hpp>

namespace ledger {
    namespace core {
        class DynamicArray;
        class DynamicObject;

        struct DynamicValue {
            // Mandatory default constructor for it to be default-constructible in (e.g.) maps.
            DynamicValue();
            DynamicValue(const DynamicValue& rhs);
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

            /// Try to get the value as if it were a std::string.
            optional<std::string&> asStr();
            /// Try to get the value as if it were a std::string.
            optional<const std::string&> asStr() const;

            /// Try to get the value as if it were a std::vector<uint8_t>.
            optional<std::vector<uint8_t>&> asData();
            /// Try to get the value as if it were a std::vector<uint8_t>.
            optional<const std::vector<uint8_t>&> asData() const;

            /// Try to get the value as if it were a bool.
            optional<bool&> asBool();
            /// Try to get the value as if it were a bool.
            optional<bool> asBool() const;

            /// Try to get the value as if it were a int32_t.
            optional<int32_t&> asInt32();
            /// Try to get the value as if it were a int32_t.
            optional<int32_t> asInt32() const;

            /// Try to get the value as if it were a int64_t.
            optional<int64_t&> asInt64();
            /// Try to get the value as if it were a int64_t.
            optional<int64_t> asInt64() const;

            /// Try to get the value as if it were a double.
            optional<double&> asDouble();
            /// Try to get the value as if it were a double.
            optional<double> asDouble() const;

            /// Try to get the value as if it were a DynamicArray.
            optional<std::shared_ptr<DynamicArray>&> asArray();
            /// Try to get the value as if it were a DynamicArray.
            optional<const std::shared_ptr<DynamicArray>&> asArray() const;

            /// Try to get the value as if it were a DynamicObject.
            optional<std::shared_ptr<DynamicObject>&> asObject();
            /// Try to get the value as if it were a DynamicObject.
            optional<const std::shared_ptr<DynamicObject>&> asObject() const;

        private:
            // Tag representing the current in-use variant
            api::DynamicType type;

            // Data. We cannot use a union because of C++ restrictions on non-trivial ctors and
            // dtors. SFOL
            boost::variant<int32_t, int64_t, bool, double, std::string, std::vector<uint8_t>,
                           std::shared_ptr<DynamicArray>, std::shared_ptr<DynamicObject>> data;
        };

        template<> void DynamicValue::serialize<cereal::PortableBinaryOutputArchive>(cereal::PortableBinaryOutputArchive&);
        template<> void DynamicValue::serialize<cereal::PortableBinaryInputArchive>(cereal::PortableBinaryInputArchive&);
        template<> void DynamicValue::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&);
        template<> void DynamicValue::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);
    }
}

#endif //LEDGER_CORE_DYNAMICVALUE_HPP
