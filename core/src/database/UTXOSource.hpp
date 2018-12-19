/*
 *
 * UTXOSource
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

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <api/ExecutionContext.hpp>
#include <math/BigInt.h>

namespace ledger {
    namespace core {
        /// An UTXO source (Bitcoin-like currencies only).
        ///
        /// Such a type represents a *source* of UTXO. That is, an abstracted way to get a set of
        /// UTXOs. It might take the set from a database, from a cache, invent them on the fly, etc.
        struct UTXOSource {
            /// An UTXO key, indexing a certain amount of satoshis (bitcoin fraction) in the
            /// blockchain.
            ///
            /// You typically find a UTXOKey attached (std::pair) with a UTXOValue.
            struct Key {
                /// Hash of the transaction that output that UTXO.
                std::string hashTX;
                /// Index in the linked transaction of the UTXO in the output array.
                uint32_t index;

                Key(std::string htx, uint32_t i);
                ~Key() = default;

                // it’s very likely this will be required by a lot of implementations, so we provide
                // it as-is
                std::size_t operator()(const Key& rhs) const noexcept {
                    auto h1 = std::hash<std::string>{}(rhs.hashTX);
                    auto h2 = std::hash<uint32_t>{}(rhs.index);

                    return h1 ^ (h2 << 1);
                }

                // for indexing in maps, sorting, etc.
                friend bool operator<(const Key& lhs, const Key& rhs) noexcept {
                    return std::tie(lhs.hashTX, lhs.index) < std::tie(rhs.hashTX, rhs.index);
                }
            };

            /// An UTXO value, giving the amount of satoshis received on a given address.
            struct Value {
                /// Amount of satoshis.
                BigInt satoshis;
                /// Address that was used.
                std::string address;

                Value(BigInt satoshis, const std::string& address);
            };

            /// An UTXO source list.
            ///
            /// Such a set will contain a list of UTXO that are available for use in this source and
            /// a list of UTXOs that have been sent in the source but are unknown (they might come
            /// from other sources).
            struct SourceList {
                std::map<Key, Value> available; ///< Available UTXOs.
                std::vector<Key> spent; ///< Spent UTXOs we don’t know / can’t resolve (yet).

                SourceList(std::map<Key, Value>&& available, std::vector<Key>&& spent);
            };

            virtual ~UTXOSource() = default;

            /// Get the list of UTXOs from this source.
            ///
            /// The vector of string is a list of all known addresses (both input and outputs) that
            /// were used in past transactions.
            virtual void getUTXOs(
                std::shared_ptr<api::ExecutionContext> ctx,
                const std::vector<std::string>& addresses,
                std::function<void (SourceList&&)> onUTXOs
            ) = 0;

            /// Invalidate the UTXO source.
            ///
            /// This method is provided for convenience and you don’t have to implement it. By
            /// default, it does nothing. You might implement it to “reset” your source if your
            /// source supports such a mechanism (e.g. a cache invalidation).
            virtual void invalidate();
        };
    }
}
