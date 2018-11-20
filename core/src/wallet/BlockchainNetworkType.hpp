#pragma once

namespace ledger {
	namespace core {
		template<
			typename Block_,
			typename Transaction_,
			typename Input_,
			typename Output_>
			class BlockchainNetworkType {
			public:
				typedef Block_ Block;
				typedef Transaction_ Transaction;
				typedef std::pair<Block, std::vector<Transaction>> FilledBlock;
				typedef Input_ Input;
				typedef Output_ Output;
		};
	}
}