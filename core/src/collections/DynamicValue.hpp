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
#include <cereal/cereal.hpp>
#include <vector>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary.hpp>

namespace ledger {
    namespace core {
        class DynamicArray;
        class DynamicObject;

        struct DynamicValue {
            std::string string;
            std::vector<uint8_t> bytes;
            bool boolean;
            int32_t int32;
            int64_t int64;
            double doubleFloat;
            std::shared_ptr<DynamicArray> array;
            std::shared_ptr<DynamicObject> object;

            api::DynamicType type;

            template <class Archive>
            void serialize(Archive& ar) {
            }

            std::string dump() const;
            std::ostream& dump(std::ostream& ss, int depth) const;
        };

        template<> void DynamicValue::serialize<cereal::PortableBinaryOutputArchive>(cereal::PortableBinaryOutputArchive & );
        template<> void DynamicValue::serialize<cereal::PortableBinaryInputArchive>(cereal::PortableBinaryInputArchive & );
        template<> void DynamicValue::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive & );
        template<> void DynamicValue::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive & );
    }
}

#endif //LEDGER_CORE_DYNAMICVALUE_HPP
