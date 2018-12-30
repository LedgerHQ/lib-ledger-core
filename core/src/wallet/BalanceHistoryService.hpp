#pragma once

#include <chrono>
#include <map>
#include <math/BigInt.h>

namespace ledger {
    namespace core {
        class BalanceHistoryService {
        public:
            virtual ~BalanceHistoryService() = default;
            // Returns only point that we have in blockhain, adjusted to the end of period
            virtual std::map<std::chrono::system_clock::time_point, BigInt> getBalanceHistory(
                const std::chrono::system_clock::time_point start,
                const std::chrono::system_clock::time_point finish,
                const std::chrono::system_clock::duration period) = 0;
        };
    }