/*
 *
 * BitcoinLikeCache
 * ledger-core
 *
 * Created by Dimitri Sabadie on 10/12/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#pragma once

#include <api/BitcoinLikeOutput.hpp>
#include <api/ExecutionContext.hpp>
#include <map>

namespace ledger {
    namespace core {
        class BitcoinLikeAccount;

        struct UTXOKey {
            std::string hashTX;
            uint32_t index;

            UTXOKey(std::string hashTX, uint32_t index);
            ~UTXOKey() = default;
        };

        /// A database cache for Bitcoin-like currencies.
        ///
        /// This type exposes very simple and straight-forward caching for:
        ///
        ///   - Getting cached UTXO for a given account.
        ///   - Getting cached balance for a given account.
        class BitcoinLikeCache: public std::enable_shared_from_this<BitcoinLikeCache> {
            /// Height of the last block in which we can find our UTXOs.
            uint32_t _lastHeight;
            /// UTXOs.
            std::map<UTXOKey, BigInt> utxos;
            /// Blockchain database used to retreive UTXO.
            std::shared_ptr<ReadOnlyBlockchainDatabase> _blockDB;

        public:
            /// Build a BitcoinLikeCache.
            BitcoinLikeCache(std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> blockDB);
            /// Build a BitcoinLikeCache by optimizing the number of blocks needed to recover the
            /// UTXO cache. You have to be sure that no transaction contains UTXO for your addresses
            /// prior to the block height you pass.
            BitcoinLikeCache(
                std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> blockDB,
                uint32_t startHeight
            );

            // we forbid copying cache
            BitcoinLikeCache(const BitcoinLikeCache&) = delete;
            BitcoinLikeCache(const BitcoinLikeCache&&) = delete;
            BitcoinLikeCache operator=(const BitcoinLikeCache&) = delete;
            BitcoinLikeCache operator=(const BitcoinLikeCache&&) = delete;

            ~BitcoinLikeCache() = default;

            /// Get the cached UTXO.
            std::vector<std::pair<UTXOKey, BigValue>>
            getUTXOs(
                std::shared_ptr<api::ExcutionContext> ctx,
                const std::vector<std::string>& addresses
            );

            /// Invalidate the UTXO cache.
            void invalidateUTXO();
        };
    }
}
