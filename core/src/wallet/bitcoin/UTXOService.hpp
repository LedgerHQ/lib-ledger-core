#pragma once

#include <async/Future.hpp>
#include <map>
#include <wallet/bitcoin/UTXO.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            class UTXOService {
            public:
                virtual ~UTXOService() = default;
                virtual Future<std::map<UTXOKey, UTXOValue>> getUTXOs() = 0;
            };
        }
    }
}