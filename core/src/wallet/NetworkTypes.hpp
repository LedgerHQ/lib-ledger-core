#pragma once

#include <wallet/BlockchainNetworkType.hpp>
#include <wallet/bitcoin/Bitcoin.hpp>
#include <api/BitcoinLikeTransaction.hpp>

namespace ledger {
    namespace core {
        typedef BlockchainNetworkType<bitcoin::Block, bitcoin::Transaction, bitcoin::Input, bitcoin::Output, api::BitcoinLikeTransaction> BitcoinLikeNetwork;
    };
};