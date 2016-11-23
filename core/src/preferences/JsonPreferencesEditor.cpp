/*
 *
 * JsonPreferencesEditor
 * ledger-core
 *
 * Created by Pierre Pollastri on 23/11/2016.
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
#include "JsonPreferencesEditor.hpp"

namespace ledger {
    namespace core {

        JsonPreferencesEditor::JsonPreferencesEditor(std::weak_ptr<IPreferencesBackend> backend, std::string name,
                                                     rapidjson::MemoryPoolAllocator<> &allocator) : _allocator(allocator) {
            _backend = backend;
            _name = name;
        }

        std::shared_ptr<api::PreferencesEditor>
        JsonPreferencesEditor::putString(const std::string &key, const std::string &value) {
            rapidjson::Value v;
            v.SetString(value.c_str(), value.size(), _allocator);
            auto change = new PreferencesChanges();
            change->name = key;
            change->value = v;
            change->type = PreferencesChangeType::SET;
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor> JsonPreferencesEditor::putInt(const std::string &key, int32_t value) {
            rapidjson::Value k(rapidjson::kStringType);
            k.SetString(key.c_str(), key.size(), _allocator);
            rapidjson::Value v;
            v.SetInt(value);
            auto change = new PreferencesChanges();
            change->name = key;
            change->value = v;
            change->type = PreferencesChangeType::SET;
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor> JsonPreferencesEditor::putLong(const std::string &key, int64_t value) {
            rapidjson::Value k(rapidjson::kStringType);
            k.SetString(key.c_str(), key.size(), _allocator);
            rapidjson::Value v;
            v.SetInt64(value);
            auto change = new PreferencesChanges();
            change->name = key;
            change->value = v;
            change->type = PreferencesChangeType::SET;
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor> JsonPreferencesEditor::putBoolean(const std::string &key, bool value) {
            rapidjson::Value k(rapidjson::kStringType);
            k.SetString(key.c_str(), key.size(), _allocator);
            rapidjson::Value v;
            v.SetBool(value);
            auto change = new PreferencesChanges();
            change->name = key;
            change->value = v;
            change->type = PreferencesChangeType::SET;
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor>
        JsonPreferencesEditor::putStringArray(const std::string &key, const std::vector<std::string> &value) {
            rapidjson::Value k(rapidjson::kStringType);
            k.SetString(key.c_str(), key.size(), _allocator);
            rapidjson::Value v(rapidjson::kArrayType);
            auto array = v.GetArray();
            for (auto& item : value) {
                rapidjson::Value jsonValue(rapidjson::kStringType);
                jsonValue.SetString(item.c_str(), item.size(), _allocator);
                array.PushBack(jsonValue, _allocator);
            }
            auto change = new PreferencesChanges();
            change->name = key;
            change->value = v;
            change->type = PreferencesChangeType::SET;
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor> JsonPreferencesEditor::remove(const std::string &key) {
            auto change = new PreferencesChanges();
            change->name = key;
            change->type = PreferencesChangeType::REMOVE;
            _changes.push_back(change);
            return shared_from_this();
        }

        void JsonPreferencesEditor::commit() {
            auto backend = _backend.lock();
            backend->save(_name, _changes);
            _changes = std::vector<PreferencesChanges *>();
        }

        JsonPreferencesEditor::~JsonPreferencesEditor() {
            for (auto change : _changes) {
                delete change;
            }
        }
    }
}