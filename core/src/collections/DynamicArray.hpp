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
#ifndef LEDGER_CORE_DYNAMICARRAY_HPP
#define LEDGER_CORE_DYNAMICARRAY_HPP

#include "../api/DynamicArray.hpp"
#include "../api/DynamicObject.hpp"
#include "../api/DynamicType.hpp"
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include "../collections/collections.hpp"
#include "DynamicValue.hpp"
#include <sstream>

namespace ledger {
    namespace core {

        class DynamicArray : public api::DynamicArray, public std::enable_shared_from_this<DynamicArray> {
        public:
            DynamicArray() {};
            int64_t size() override;
            optional<std::string> getString(int64_t index) override;
            optional<int32_t> getInt(int64_t index) override;
            optional<int64_t> getLong(int64_t index) override;
            optional<double> getDouble(int64_t index) override;
            optional<std::vector<uint8_t>> getData(int64_t index) override;
            optional<bool> getBoolean(int64_t index) override;
            std::shared_ptr<api::DynamicArray> pushInt(int32_t value) override;
            std::shared_ptr<api::DynamicObject> getObject(int64_t index) override;
            std::shared_ptr<api::DynamicArray> pushLong(int64_t value) override;
            std::shared_ptr<api::DynamicArray> pushString(const std::string &value) override;
            std::shared_ptr<api::DynamicArray> pushDouble(double value) override;
            std::shared_ptr<api::DynamicArray> pushData(const std::vector<uint8_t> &value) override;
            std::shared_ptr<api::DynamicArray> pushBoolean(bool value) override;
            std::shared_ptr<api::DynamicArray> pushObject(const std::shared_ptr<api::DynamicObject> &value) override;
            std::shared_ptr<api::DynamicArray> pushArray(const std::shared_ptr<api::DynamicArray> &value) override;
            std::shared_ptr<api::DynamicArray> concat(const std::shared_ptr<api::DynamicArray> &array) override;

            std::shared_ptr <api::DynamicArray> getArray(int64_t index) override;
            optional<api::DynamicType> getType(int64_t index) override;
            bool remove(int64_t index) override;
            std::string dump() override;
            std::vector<uint8_t> serialize() override;

            std::ostream& dump(std::ostream& ss, int depth) const;

            template <class Archive>
            void serialize(Archive& ar) {
                ar(_values.getContainer());
            }

        private:
            Array<DynamicValue> _values;
        };
    }
}


#endif //LEDGER_CORE_DYNAMICARRAY_HPP
