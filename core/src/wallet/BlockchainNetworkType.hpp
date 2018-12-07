#pragma once
#include <vector>
#include <utility>

namespace ledger {
    namespace core {

        template<typename Block_, typename Transaction_>
        class FilledBlock {
        public:
            Block_ header;
            std::vector<Transaction_> transactions;
            template <class Archive>
            void serialize(Archive & ar) {
                ar(header, transactions);
            }
        };

        template<
            typename Block_,
            typename Transaction_,
            typename Input_,
            typename Output_,
            typename TransactionToBroadcast_>
            class BlockchainNetworkType {
            public:
                typedef Block_ Block;
                typedef Transaction_ Transaction;
                typedef FilledBlock<Block_, Transaction_> FilledBlock;
                typedef Input_ Input;
                typedef Output_ Output;
                typedef TransactionToBroadcast_ TransactionToBroadcast;
        };
    }
}