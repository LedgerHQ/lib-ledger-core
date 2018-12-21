/*
 *
 * UTXOSourceInMemory
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
#include <set>
#include <string>

#include <api/ExecutionContext.hpp>
#include <wallet/bitcoin/UTXOSource.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <wallet/NetworkTypes.hpp>

namespace ledger {
    namespace core {
        // forward declaration
        struct KeychainRegistry;

        namespace bitcoin {

            typedef ReadOnlyBlockchainDatabase<BitcoinLikeNetwork::FilledBlock> BlocksDB;
            
            /// An in-memory (map) implementation of [database::UTXOSource](@UTXOSource).
            class UTXOSourceInMemory: public UTXOSource, public std::enable_shared_from_this<UTXOSourceInMemory> {
                typedef std::map<UTXOKey, UTXOValue> UTXOMap;

                /// UTXOs.
                UTXOMap _cache;

                /// The keychain registry to check addresses against.
                std::shared_ptr<KeychainRegistry> _keychainRegistry;

                /// Blockchain database used to retreive UTXO.
                std::shared_ptr<BlocksDB> _blockDB;

                /// Lowest height block in which we can find our UTXOs. Lower means no UTXO for us.
                uint32_t _lowestHeight;

                /// Height of the last block in which we can find our UTXOs.
                uint32_t _lastHeight;

            public:
                /// Build an in-memory cache.
                UTXOSourceInMemory(
                    std::shared_ptr<BlocksDB> blockDB,
                    std::shared_ptr<KeychainRegistry> keychainRegistry
                );

                /// Build an in-memory cache by optimizing the number of blocks needed to recover the
                /// UTXO cache. You have to be sure that no transaction contains UTXO for your addresses
                /// prior to the block height you pass.
                UTXOSourceInMemory(
                    std::shared_ptr<BlocksDB> blockDB,
                    std::shared_ptr<KeychainRegistry> keychainRegistry,
                    uint32_t lowestHeight
                );

                ~UTXOSourceInMemory() = default;

                /// Get the cached UTXO.
                Future<UTXOSourceList> getUTXOs(std::shared_ptr<api::ExecutionContext> ctx) override;

                /// Invalidate the UTXO cache.
                void invalidate();
            };
        }
    }
}
