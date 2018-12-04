#pragma once

#include <map>

namespace ledger {
    namespace core {
        class KeysDB {
        public:
            virtual ~KeysDB() {};
            virtual void addKey(const DeterministicPublicKey& address, uint32_t index) = 0;
            virtual std::vector<DeterministicPublicKey> getAllKeys() = 0;
            virtual void setMaxUsedIndex(uint32_t index) = 0;
            virtual uint32_t getMaxUsedIndex() = 0;
        };
    }
}