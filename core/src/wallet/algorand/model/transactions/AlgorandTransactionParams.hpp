/*
 * AlgorandTransactionParams
 *
 * Created by Rémi Barjon on 11/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include <cstdint>
#include <string>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    struct TransactionParams {
        // Genesis ID of the node
        std::string genesisID;
        // Genesis hash (32 bytes in Base64) of the node
        std::string genesisHash;
        // The last round seen by the node
        uint64_t lastRound;
        // The consensus protocol version (as of lastRound) applied by the node
        std::string consensusVersion;
        // The suggested transaction fee, in micro-Algos per byte
        uint64_t suggestedFeePerByte;
        // The minimum transaction fee (not per byte) required in the current network protocol
        uint64_t minFee;
    };

} // namespace model
} // namespace algorand
} // namespace core
} // namespace ledger

