#pragma once

#include <crypto/DeterministicPublicKey.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <wallet/Keychain.hpp>
#include <wallet/KeysDB.hpp>


namespace ledger {
    namespace core {
        namespace bitcoin {
            class ChangeKeychain : public core::Keychain {
            public:
                typedef std::function<std::string(const DeterministicPublicKey& key)> AddressGetter;
                ChangeKeychain(
                    const DeterministicPublicKey& key,
                    const AddressGetter& addressGetter,
                    const std::shared_ptr<KeysDB>& keysDB);
                uint32_t getNumberOfUsedAddresses();
                std::vector<std::string> getAddresses(uint32_t startIndex, uint32_t count);
                void markAsUsed(const std::string& address);
            private:
                std::string getAddresse(const DeterministicPublicKey& key);
            private:
                DeterministicPublicKey _parentKey;
                AddressGetter _addressGetter;
                std::shared_ptr<KeysDB> _keysDB;
                uint32_t _maxUsedIndex;
                std::unordered_map<std::string, uint32_t> _indexMap;
                std::mutex _lock;
                std::vector<std::string> _addresses;
            };
} } }