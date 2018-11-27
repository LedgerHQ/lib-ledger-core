#pragma once

#include <mutex>
#include <vector>

namespace ledger {
    namespace core {
        namespace keychain {
            enum KeyPurpose {
                RECEIVE, CHANGE
            };
        }
        template<typename NetworkType>
        class Keychain {
        public:
            virtual ~Keychain() {};
            virtual uint32_t getNumberOfUsedAddresses() = 0;
            virtual std::vector<std::string> getAddresses(uint32_t startIndex, uint32_t count) = 0;
        };
    }
}