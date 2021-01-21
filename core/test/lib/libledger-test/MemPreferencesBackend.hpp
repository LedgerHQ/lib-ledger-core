#pragma once

#include <memory>
#include <map>
#include <api/PreferencesBackend.hpp>
#include <api/PreferencesChange.hpp>

namespace ledger {
    namespace core {
        namespace test {
            class MemPreferencesBackend : public api::PreferencesBackend {
            public:
                virtual ~MemPreferencesBackend();

                virtual std::experimental::optional<std::vector<uint8_t>> get(const std::vector<uint8_t> & key) const override;

                virtual bool commit(const std::vector<api::PreferencesChange> & changes) override;

                virtual void setEncryption(const std::shared_ptr<api::RandomNumberGenerator> & rng, const std::string & password) override;

                virtual void unsetEncryption() override;

                virtual bool resetEncryption(const std::shared_ptr<api::RandomNumberGenerator> & rng, const std::string & oldPassword, const std::string & newPassword) override;

                virtual std::string getEncryptionSalt() const override;

                virtual void clear() override;

            private:
                std::map<std::vector<uint8_t>, std::vector<uint8_t>> _data;
            };
        }
    }
}

