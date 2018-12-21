#pragma once

#include <math/BigInt.h>
#include <async/Future.hpp>

namespace ledger {
    namespace core {
        class BalanceService {
        public:
            virtual ~BalanceService() = default;
            virtual Future<BigInt> getBalance() = 0;
        };
    }
}