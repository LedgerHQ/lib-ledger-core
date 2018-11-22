#pragma once

#include <wallet/BlockchainNetworkType.hpp>
#include <wallet/bitcoin/Bitcoin.hpp>
#include <api/BitcoinLikeTransaction.hpp>
#include <wallet/common/Block.hpp>

namespace ledger {
    namespace core {
        typedef BlockchainNetworkType<Block, bitcoin::Transaction, bitcoin::Input, bitcoin::Output, api::BitcoinLikeTransaction> BitcoinLikeNetwork;
    };
};