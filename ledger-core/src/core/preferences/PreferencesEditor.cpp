/*
 *
 * PreferencesEditor
 * ledger-core
 *
 * Created by Pierre Pollastri on 11/01/2017.
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

#include <core/bytes/BytesWriter.h>
#include <core/preferences/PreferencesEditor.hpp>

namespace ledger {
    namespace core {
        std::shared_ptr<api::PreferencesEditor>
        PreferencesEditor::putString(const std::string &key, const std::string &value) {
            PreferencesChange change;
            change.type = PreferencesChangeType::PUT_TYPE;
            change.key = _preferences.wrapKey(key);
            BytesWriter writer;
            writer.writeString(value);
            change.value = writer.toByteArray();
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor> PreferencesEditor::putInt(const std::string &key, int32_t value) {
            PreferencesChange change;
            change.type = PreferencesChangeType::PUT_TYPE;
            change.key = _preferences.wrapKey(key);
            BytesWriter writer;
            writer.writeLeValue<uint32_t>((uint32_t)value);
            change.value = writer.toByteArray();
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor> PreferencesEditor::putLong(const std::string &key, int64_t value) {
            PreferencesChange change;
            change.type = PreferencesChangeType::PUT_TYPE;
            change.key = _preferences.wrapKey(key);
            BytesWriter writer;
            writer.writeLeValue<uint64_t>((uint64_t)value);
            change.value = writer.toByteArray();
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor> PreferencesEditor::putBoolean(const std::string &key, bool value) {
            PreferencesChange change;
            change.type = PreferencesChangeType::PUT_TYPE;
            change.key = _preferences.wrapKey(key);
            BytesWriter writer;
            writer.writeByte(value ? (uint8_t)0x01 : (uint8_t)0x00);
            change.value = writer.toByteArray();
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor>
        PreferencesEditor::putStringArray(const std::string &key, const std::vector<std::string> &value) {
            PreferencesChange change;
            change.type = PreferencesChangeType::PUT_TYPE;
            change.key = _preferences.wrapKey(key);
            BytesWriter writer;
            for (auto& item : value) {
                writer.writeVarString(item);
            }
            change.value = writer.toByteArray();
            _changes.push_back(change);
            return shared_from_this();
        }

        std::shared_ptr<api::PreferencesEditor> PreferencesEditor::remove(const std::string &key) {
            PreferencesChange change;
            change.type = PreferencesChangeType::DELETE_TYPE;
            change.key = _preferences.wrapKey(key);
            _changes.push_back(change);
            return shared_from_this();
        }

        void PreferencesEditor::commit() {
            _preferences._backend.commit(_changes);
            _changes.clear();
        }

        void PreferencesEditor::clear() {
            _preferences._backend.clear();
            _changes.clear();
        }

        PreferencesEditor::PreferencesEditor(Preferences &preferences) : _preferences(preferences) {

        }

        std::shared_ptr<api::PreferencesEditor>
        PreferencesEditor::putData(const std::string &key, const std::vector<uint8_t> &value) {
            PreferencesChange change;
            change.type = PreferencesChangeType::PUT_TYPE;
            change.key = _preferences.wrapKey(key);
            BytesWriter writer;
            writer.writeByteArray(value);
            change.value = writer.toByteArray();
            _changes.push_back(change);
            return shared_from_this();
        }
    }
}
