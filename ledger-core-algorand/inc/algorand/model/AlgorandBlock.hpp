/*
 * AlgorandBlock
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

#include <algorand/model/transactions/AlgorandTransaction.hpp>

#include <cstdint>
#include <string>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    struct Block {
        // TimeStamp in seconds since epoch
        uint64_t timestamp;
        // The current round on which this block was appended to the chain
        uint64_t round;
        // The period on which the block was confirmed
        uint64_t period;
        // The address of this block proposer
        std::string proposer;
        // The current block hash
        std::string hash;
        // The previous block hash
        std::string previousBlockHash;

        // List of transactions contained in this block
        Option<std::vector<Transaction>> txns;

        // The current protocol
        std::string currentProtocol;
        // The next proposed protocol
        std::string nextProtocol;
        // The number of blocks which approved the protocol upgrade
        uint64_t nextProtocolApprovals;
        // The round on which the protocol upgrade will take effect
        uint64_t nextProtocolSwitchOn;
        // The deadline round for this protocol upgrade (No votes will be considered after this round)
        uint64_t nextProtocolVoteBefore;
        // Indicates a proposed upgrade
        std::string upgradePropose;
        // Indicates a yes vote for the current proposal
        bool upgradeApprove;

        // The sortition seed
        std::string seed;
        // Root of the Merkle tree of all transactions contained in this block.
        // Allows to authenticate the set of transactions in this block.
        std::string txnRoot;
    };

} // namespace model
} // namespace ledger
} // namespace core
} // namespace algorand
