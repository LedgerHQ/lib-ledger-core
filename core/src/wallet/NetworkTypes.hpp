#pragma once

#include <wallet/bitcoin/Bitcoin.hpp>
#include <wallet/BlockchainNetworkType.hpp>
#include <api/BitcoinLikeTransaction.hpp>

namespace ledger {
    namespace core {
        typedef BlockchainNetworkType<bitcoin::Block, bitcoin::Transaction, bitcoin::Input, bitcoin::Output, api::BitcoinLikeTransaction> BitcoinLikeNetwork;
    };
};
