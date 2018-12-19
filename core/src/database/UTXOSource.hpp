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
#include <set>
#include <string>
#include <utility>

#include <api/ExecutionContext.hpp>
#include <async/Future.hpp>
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
            typedef std::pair<std::string, uint32_t> Key;

            /// An UTXO value, giving the amount of satoshis received on a given address.
            struct Value {
                /// Amount.
                BigInt amount;
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
                std::set<Key> spent; ///< Spent UTXOs we don’t know / can’t resolve (yet).

                SourceList(std::map<Key, Value>&& available, std::set<Key>&& spent);
            };

            virtual ~UTXOSource() = default;

            /// Get the list of UTXOs from this source.
            ///
            /// The set of string is a list of all known addresses (both input and outputs) that
            /// were used in past transactions.
            virtual Future<SourceList> getUTXOs(std::shared_ptr<api::ExecutionContext> ctx) = 0;
        };
    }
}
