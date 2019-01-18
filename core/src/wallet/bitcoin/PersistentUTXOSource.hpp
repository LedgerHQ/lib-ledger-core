/*
 *
 * PersistentUTXOSource
 * ledger-core
 *
 * Created by Dimitri Sabadie on 21/12/2018.
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

#include <api/ExecutionContext.hpp>
#include <async/Future.hpp>
#include <database/BlockchainDB.hpp>
#include <wallet/bitcoin/UTXOSource.hpp>
#include <wallet/bitcoin/UTXOSourceInMemory.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            class PersistentUTXOSource: public UTXOSource, public std::enable_shared_from_this<PersistentUTXOSource> {
                std::shared_ptr<db::BlockchainDB> _db; ///< Database used for UTXOs persistence.
                std::shared_ptr<UTXOSourceInMemory> _inMemorySource; ///< Memory cache.

            public:
                PersistentUTXOSource(
                    const std::shared_ptr<db::BlockchainDB>& blockchainDB,
                    const std::shared_ptr<UTXOSourceInMemory>& inMemorySource
                );

                ~PersistentUTXOSource() = default;

                Future<UTXOSourceList> getUTXOs(std::shared_ptr<api::ExecutionContext> ctx) override;
            };
        }
    }
}
