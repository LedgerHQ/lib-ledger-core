
#include <MemPreferencesBackend.hpp>
#include "utils/optional.hpp"

namespace ledger {
    namespace core {
        namespace test {
            MemPreferencesBackend::~MemPreferencesBackend() {}

            std::experimental::optional<std::vector<uint8_t>> MemPreferencesBackend::get(const std::vector<uint8_t> & key) const
            {
               if (_data.find(key) != _data.end()) 
                    return _data.at(key);
                return optional<std::vector<uint8_t>>();
            }

            bool MemPreferencesBackend::commit(const std::vector<api::PreferencesChange> & changes)
            {
                for(const auto& change: changes)
                {
                    _data[change.key] = change.value;
                }
                return true;
            }

            void MemPreferencesBackend::setEncryption(const std::shared_ptr<api::RandomNumberGenerator> & rng, const std::string & password)
            {
            }

            void MemPreferencesBackend::unsetEncryption()
            {
            }

            bool MemPreferencesBackend::resetEncryption(const std::shared_ptr<api::RandomNumberGenerator> & rng, const std::string & oldPassword, const std::string & newPassword)
            {
                return true;
            }

            std::string MemPreferencesBackend::getEncryptionSalt() const
            {
                return "";
            }

            void MemPreferencesBackend::clear()
            {
                _data.clear();
            }
            
        }
    }
}


