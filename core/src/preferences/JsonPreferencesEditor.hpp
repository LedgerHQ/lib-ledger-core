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
#ifndef LEDGER_CORE_JSONPREFERENCESEDITOR_HPP
#define LEDGER_CORE_JSONPREFERENCESEDITOR_HPP

#include "IPreferencesBackend.hpp"
#include "../api/PreferencesEditor.hpp"
#include <memory>
#include <vector>

namespace ledger {
    namespace core {
        class JsonPreferencesEditor : public api::PreferencesEditor, public std::enable_shared_from_this<JsonPreferencesEditor> {
        public:
            JsonPreferencesEditor(std::weak_ptr<IPreferencesBackend> backend,
                                  std::string name,
                                  rapidjson::MemoryPoolAllocator<>& allocator);
            virtual std::shared_ptr<api::PreferencesEditor>
            putString(const std::string &key, const std::string &value) override;

            virtual std::shared_ptr<api::PreferencesEditor> putInt(const std::string &key, int32_t value) override;

            virtual std::shared_ptr<api::PreferencesEditor> putLong(const std::string &key, int64_t value) override;

            virtual std::shared_ptr<api::PreferencesEditor> putBoolean(const std::string &key, bool value) override;

            virtual std::shared_ptr<api::PreferencesEditor>
            putStringArray(const std::string &key, const std::vector<std::string> &value) override;

            virtual std::shared_ptr<api::PreferencesEditor> remove(const std::string &key) override;

            virtual void commit() override;
            ~JsonPreferencesEditor();
        private:
            std::vector<PreferencesChanges *> _changes;
            std::weak_ptr<IPreferencesBackend> _backend;
            std::string _name;
            rapidjson::MemoryPoolAllocator<>& _allocator;
        };
    }
}

#endif //LEDGER_CORE_JSONPREFERENCESEDITOR_HPP
