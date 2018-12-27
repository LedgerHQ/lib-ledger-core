#pragma once

#include <async/Future.hpp>
#include <vector>

namespace ledger {
    namespace core {

        class Operation;

        class OperationService {
        public:
            virtual ~OperationService() = default;
            // get all operations in blocks [fromBlock, toBlock)
            virtual Future<std::vector<Operation>> getOperations(uint32_t fromBlock, uint32_t toBlock) = 0;
            virtual Future<std::vector<Operation>> getPendingOperations() = 0;
        };
    }
}