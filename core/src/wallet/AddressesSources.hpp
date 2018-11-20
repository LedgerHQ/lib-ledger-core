#pragma once

#include <vector>
#include <async/Future.hpp>
#include <wallet/NetworkTypes.hpp>


namespace ledger {
    namespace core {
        // Synchonizer ask this class about addresses he want to sync
        // for blockchains with Account model (Ethereum) this class 
        // will always have one batch with one address, for UTXO model
        // it will derive a new batch of addresses if previous one was marked as used.
        // For each Network specialization should be created.
        template<typename NetworkType>
        class AddressSources {
        public:
            uint32_t GetNumberOfBatches();
            void MarkBatchAsUsed(uint32_t batchIndex);
            Future<std::vector<std::string>> GetBatch(uint32_t batchIndex);
        };
    }
}