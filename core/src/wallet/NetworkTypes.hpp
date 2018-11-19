#pragma once

#include <wallet/BlockchainNetworkType.hpp>
#include <wallet/bitcoin/Bitcoin.hpp>

namespace ledger {
	namespace core {
		typedef BlockchainNetworkType<bitcoin::Block, bitcoin::Transaction, bitcoin::Input, bitcoin::Output> BitcoinLikeNetwork;
	};
};