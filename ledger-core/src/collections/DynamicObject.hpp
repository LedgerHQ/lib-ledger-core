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
#ifndef LEDGER_CORE_DYNAMICOBJECT_HPP
#define LEDGER_CORE_DYNAMICOBJECT_HPP

#include "../api/DynamicArray.hpp"
#include "../api/DynamicObject.hpp"
#include "../api/DynamicType.hpp"
#include <cereal/cereal.hpp>
#include "../collections/collections.hpp"
#include "DynamicValue.hpp"
#include "DynamicArray.hpp"
#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>

namespace ledger {
    namespace core {
        class DynamicObject : public api::DynamicObject, public std::enable_shared_from_this<DynamicObject> {
        public:
            DynamicObject() : _readOnly(false) {}

            /// A generic indexed and type-safe getter.
            template <typename T>
            optional<T> get(const std::string& key) {
                const auto v = optional<DynamicValue>(_values.lift(key));

                if (v) {
                    return v->get<T>();
                } else {
                    return optional<T>();
                }
            }

            /// A generic and type-safe put.
            template <typename T>
            std::shared_ptr<DynamicObject> put(const std::string& key, T value) {
                if (!_readOnly) {
                    _values[key] = DynamicValue(value);
                }

                return shared_from_this();
            }

            optional<std::string> getString(const std::string &key) override;
            optional<int32_t> getInt(const std::string &key) override;
            optional<int64_t> getLong(const std::string &key) override;
            optional<double> getDouble(const std::string &key) override;
            optional<bool> getBoolean(const std::string &key) override;
            optional<std::vector<uint8_t>> getData(const std::string &key) override;
            std::shared_ptr<api::DynamicObject> getObject(const std::string &key) override;
            std::shared_ptr<api::DynamicArray> getArray(const std::string &key) override;

            std::shared_ptr<api::DynamicObject> putString(const std::string &key, const std::string &value) override;
            std::shared_ptr<api::DynamicObject> putInt(const std::string &key, int32_t value) override;
            std::shared_ptr<api::DynamicObject> putLong(const std::string &key, int64_t value) override;
            std::shared_ptr<api::DynamicObject> putDouble(const std::string &key, double value) override;
            std::shared_ptr<api::DynamicObject> putBoolean(const std::string &key, bool value) override;
            std::shared_ptr<api::DynamicObject> putData(const std::string &key, const std::vector<uint8_t> &value) override;
            std::shared_ptr<api::DynamicObject> putObject(const std::string &key, const std::shared_ptr<api::DynamicObject> &value) override;
            std::shared_ptr<api::DynamicObject> putArray(const std::string &key, const std::shared_ptr<api::DynamicArray> &value) override;

            bool contains(const std::string &key) override;

            bool remove(const std::string &key) override;

            std::vector<std::string> getKeys() override;

            optional<api::DynamicType> getType(const std::string &key) override;

            std::string dump() override;

            std::vector<uint8_t> serialize() override;

            bool isReadOnly() override;
            void setReadOnly(bool enable);

            std::shared_ptr<api::DynamicObject> updateWithConfiguration(const std::shared_ptr<DynamicObject> &configuration);
            int64_t size() override;

            std::ostream& dump(std::ostream& ss, int depth) const;

            template <class Archive>
            void serialize(Archive& ar) {
                ar(_values.getContainer());
            }

        private:
            Map<std::string, DynamicValue> _values;
            bool _readOnly;
        };
    }
}

#endif //LEDGER_CORE_DYNAMICOBJECT_HPP
