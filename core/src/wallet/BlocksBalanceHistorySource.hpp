#pragma once
#include <map>
#include <math/BigInt.h>

namespace ledger {
    namespace core {
        /// Source of the Balance History,
        /// this is the interface that on request return a map with the key is the block height and
        /// value is balance of all the transactions that this Source is responsible for at the end of this block
        /// For example we have a blockchain like this
        ///   block height    transaction amount
        ///         1               +100
        ///                         +200
        ///         2               -120
        ///         10              -30
        ///         30              -10
        ///                         +50
        ///         32              +1000
        /// And BlocksBalanceHistorySource is responsible for blocks range [5, 31]
        /// then on request about balance history from blocks 1 to 1000 this object will return
        /// a map {(10, -30), {30, 10}}, so it ignores everything that he is not responsible for

        class BlocksBalanceHistorySource {
        public:
            virtual ~BlocksBalanceHistorySource() = default;
            virtual std::map<uint32_t, BigInt> getHistory() = 0;
        };
    }
}