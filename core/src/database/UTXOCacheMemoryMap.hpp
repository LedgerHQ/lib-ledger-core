/*
 *
 * UTXOCacheMemoryMap
 * ledger-core
 *
 * Created by Dimitri Sabadie on 13/12/2018.
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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <api/ExecutionContext.hpp>
#include <database/UTXOCache.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/NetworkTypes.hpp>

namespace ledger {
    namespace core {

        /// An in-memory (map) implementation of database::UTXOCache.
        class UTXOCacheMemoryMap: public UTXOCache, public std::enable_shared_from_this<UTXOCacheMemoryMap> {
            typedef std::map<UTXOCache::Key, UTXOCache::Value> UTXOMemoryMap;

            /// UTXOs.
            UTXOMemoryMap _cache;
            /// Blockchain database used to retreive UTXO.
            std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> _blockDB;

        public:
            /// Build an in-memory cache.
            UTXOCacheMemoryMap(std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> blockDB);

            /// Build an in-memory cache by optimizing the number of blocks needed to recover the
            /// UTXO cache. You have to be sure that no transaction contains UTXO for your addresses
            /// prior to the block height you pass.
            UTXOCacheMemoryMap(
                std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> blockDB,
                uint32_t lowestHeight
            );

            ~UTXOCacheMemoryMap() = default;

            /// Get the cached UTXO.
            void getUTXOs(
                std::shared_ptr<api::ExecutionContext> ctx,
                const std::vector<std::string>& addresses,
                std::function<void (std::vector<std::pair<UTXOCache::Key, UTXOCache::Value>>)> onUTXOs
            ) override;

            /// Invalidate the UTXO cache.
            void invalidate() override;
        };
    }
}
