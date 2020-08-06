/*
 *
 * BitcoinLikeUtxo
 * ledger-core
 *
 * Created by Alexis Le Provost on 02/06/2020.
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

#ifndef __BITCOINLIKEUTXO_H_
#define __BITCOINLIKEUTXO_H_

#include <string>

#include <utils/Option.hpp>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <wallet/common/Amount.h>

namespace ledger {
    namespace core {
        struct BitcoinLikeUtxo {
            uint64_t index;
            std::string transactionHash;
            // HACK: Amount class lacks of constness so
            // to avoid to const_cast everywhere I select that
            // quick-win solution.
            // We should change our generated interfaces and qualify
            // at least all getters with const keyword
            mutable Amount value;
            Option<std::string> address;
            Option<std::string> accountUid;
            std::string script;
            Option<uint64_t> blockHeight;

            operator BitcoinLikeBlockchainExplorerOutput() const;
            BitcoinLikeUtxo() = default;
        };

        BitcoinLikeUtxo makeUtxo(BitcoinLikeBlockchainExplorerOutput const& output, api::Currency const& currency);
        BitcoinLikeBlockchainExplorerOutput toExplorerOutput(BitcoinLikeUtxo const& utxo);
    }
}

#endif // __BITCOINLIKEUTXO_H_
