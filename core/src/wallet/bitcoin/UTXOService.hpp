#pragma once

#include <map>
#include <wallet/bitcoin/UTXO.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            class UTXOService {
            public:
                virtual ~UTXOService() = default;
                virtual std::map<UTXOKey, UTXOValue> getUTXOs() = 0;
            };
        }
    }
}