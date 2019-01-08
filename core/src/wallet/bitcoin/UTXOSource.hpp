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

#include <wallet/bitcoin/UTXO.hpp>
#include <api/ExecutionContext.hpp>
#include <async/Future.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            /// An UTXO source (Bitcoin-like currencies only).
            ///
            /// Such a type represents a *source* of UTXO. That is, an abstracted way to get a set of
            /// UTXOs. It might take the set from a database, from a cache, invent them on the fly, etc.
            struct UTXOSource {
                virtual ~UTXOSource() = default;

                /// Get the list of UTXOs from this source.
                virtual Future<UTXOSourceList> getUTXOs(std::shared_ptr<api::ExecutionContext> ctx) = 0;
            };
        }
    }
}
