
#include <MemPreferencesBackend.hpp>
#include "utils/optional.hpp"

namespace ledger {
    namespace core {
        namespace test {
            MemPreferencesBackend::~MemPreferencesBackend() {}

            std::experimental::optional<std::vector<uint8_t>> MemPreferencesBackend::get(const std::vector<uint8_t> & key)
            {
                std::lock_guard<std::mutex> lock(_mtx);
                if (_data.find(key) != _data.end())
                    return _data.at(key);
                return optional<std::vector<uint8_t>>();
            }

            bool MemPreferencesBackend::commit(const std::vector<api::PreferencesChange> & changes)
            {
                for(const auto& change: changes)
                {
                    std::lock_guard<std::mutex> lock(_mtx);
                    switch (change.type) {
                    case api::PreferencesChangeType::PUT_TYPE:
                        _data[change.key] = change.value;
                        break;
                    case api::PreferencesChangeType::DELETE_TYPE:
                        if (_data.erase(change.key) != 1)
                            throw std::runtime_error("Trying to remove element that doesn't exists");
                        break;
                    default:
                        throw std::runtime_error("Unknown change type");
                        break;
                    }
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

            std::string MemPreferencesBackend::getEncryptionSalt()
            {
                return "";
            }

            void MemPreferencesBackend::clear()
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _data.clear();
            }

        }
    }
}


