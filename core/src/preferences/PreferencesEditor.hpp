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
#ifndef LEDGER_CORE_PREFERENCESEDITOR_HPP
#define LEDGER_CORE_PREFERENCESEDITOR_HPP

#include "../api/PreferencesEditor.hpp"
#include <memory>
#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>

namespace ledger {
    namespace core {
        struct PreferencesChange;
        class Preferences;

        class PreferencesEditor : public api::PreferencesEditor, public std::enable_shared_from_this<PreferencesEditor> {
        public:
            PreferencesEditor(Preferences& preferences);

            std::shared_ptr<api::PreferencesEditor>
            putString(const std::string &key, const std::string &value) override;

            std::shared_ptr<api::PreferencesEditor> putInt(const std::string &key, int32_t value) override;

            std::shared_ptr<api::PreferencesEditor> putLong(const std::string &key, int64_t value) override;

            std::shared_ptr<api::PreferencesEditor> putBoolean(const std::string &key, bool value) override;

            std::shared_ptr<api::PreferencesEditor>
            putStringArray(const std::string &key, const std::vector<std::string> &value) override;

            std::shared_ptr<api::PreferencesEditor> remove(const std::string &key) override;

            template <typename T>
            std::shared_ptr<PreferencesEditor> putObject(const std::string& key, T& object) {
                std::stringstream is;
                ::cereal::PortableBinaryOutputArchive archive(is);
                archive(object);
                auto savedState = is.str();
                putData(key, std::vector<uint8_t>((const uint8_t *)savedState.data(),(const uint8_t *)savedState.data() + savedState.length()));
                return shared_from_this();
            };

            std::shared_ptr<api::PreferencesEditor>
            putData(const std::string &key, const std::vector<uint8_t> &value) override;

            void commit() override;

            /// Clear all preferences.
            void clear() override;

        private:
            std::vector<PreferencesChange> _changes;
            Preferences& _preferences;
        };
    }
}


#endif //LEDGER_CORE_PREFERENCESEDITOR_HPP
