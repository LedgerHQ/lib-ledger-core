//
// Created by PIERRE POLLASTRI on 25/08/2017.
//

#ifndef LEDGER_CORE_SECP256K1API_H
#define LEDGER_CORE_SECP256K1API_H

#include <api/Secp256k1.hpp>
#include <include/secp256k1.h>

namespace ledger {
    namespace core {
        class Secp256k1Api : public api::Secp256k1 {
        public:
            Secp256k1Api();
            std::vector<uint8_t> computePubKey(const std::vector<uint8_t> &privKey, bool compress) override;
            std::vector<uint8_t> sign(const std::vector<uint8_t> &privKey, const std::vector<uint8_t> &data) override;
            bool verify(const std::vector<uint8_t> &data, const std::vector<uint8_t>& signature, const std::vector<uint8_t> &pubKey) override;

            ~Secp256k1Api();
        private:
            secp256k1_context* _context;
        };
    }
}


#endif //LEDGER_CORE_SECP256K1API_H
