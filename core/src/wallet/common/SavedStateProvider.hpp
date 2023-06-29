#pragma once

#include "../../preferences/Preferences.hpp"
#include "../../utils/Option.hpp"

#include <memory>

namespace ledger {
    namespace core {
        template <typename SavedStateType>
        class SavedStateProvider {
          public:
            SavedStateProvider(std::shared_ptr<Preferences> subPreferences) : _subPreferences{std::move(subPreferences)} {}
            Option<SavedStateType> getSavedState() const {
                return _subPreferences->getObject<SavedStateType>("state");
            }
            void setSavedState(SavedStateType &savedState) {
                _subPreferences->editor()->putObject<SavedStateType>("state", savedState)->commit();
            }

          private:
            std::shared_ptr<Preferences> _subPreferences;
        };
    } // namespace core
} // namespace ledger