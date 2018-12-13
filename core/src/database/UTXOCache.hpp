/*
 *
 * UTXOCache
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

#include <utility>

namespace ledger {
    namespace core {
        /// An UTXO cache (Bitcoin-like currencies only).
        ///
        /// The Backend type template parameter must provide:
        ///
        ///   - A typename to represent iterators over UTXO in the implementation.
        template <class Backend>
        struct UTXOCache {
            typedef Backend::UTXOIterable UTXOIterable;

            /// A UTXO key, indexing a certain amount of satoshis (bitcoin fraction) in the blockchain.
            ///
            /// You typically find a UTXOKey attached (std::pair) with a BigValue representing the
            /// number of satoshis.
            struct Key {
                /// Hash of the transaction that output that UTXO.
                std::string hashTX;
                /// Index in the linked transaction of the UTXO in the output array.
                uint32_t index;

                Key(std::string hashTX, uint32_t index);
                ~Key() = default;

                // we forbid copying, cloning, and changing UTXO key
                Key(const Key&) = delete;
                Key operator=(const Key&) = delete;
            };

            UTXOCache(uint32_t lowestHeight);
            virtual ~UTXOCache();

            // we forbid copying cache
            UTXOCache(const UTXOCache&) = delete;
            UTXOCache(UTXOCache&&) = delete;
            UTXOCache operator=(const UTXOCache&) = delete;
            UTXOCache operator=(UTXOCache&&) = delete;

            /// Get the list of cached UTXOs.
            ///
            /// The vector of string is a list of all known addresses (both input and outputs) that
            /// were used in past transactions.
            virtual void getUTXOs(
                std::shared_ptr<api::ExecutionContext> ctx,
                const std::vector<std::string>& addresses,
                std::function<void (UTXOIterable)> onUTXOs
            ) = 0;

            /// Invalidate the whole UTXO cache.
            void invalidate() = 0;

            /// Get the last block we synchronized with.
            uint32_t getLastHeight();

            /// Get the lowest block we synchronize with. This asserts that no UTXO could be found
            /// in any previous blocks.
            uint32_t getLowestHeight();

        private:
            /// Lowest height block in which we can find our UTXOs. Lower means no UTXO for us.
            uint32_t _lowestHeight;

            /// Height of the last block in which we can find our UTXOs.
            uint32_t _lastHeight;

        protected:
            /// Update the last known block height.
            ///
            /// Call this function in derived implementations whenever you’re done synchronizing
            /// with blocks. This will help to create smarter blocks difference the next time a
            /// get is performed and is the key concept to implementing such a caching mechanism.
            ///
            /// If you provide a height that is less than the one already known, the height won’t be
            /// updated. If you intended to reset the cache, use the invalidate() function.
            void updateLastHeight(uint32_t lastHeight);
        }
    }
}
