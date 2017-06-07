/*
 *
 * BitcoinLikeAccount
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/04/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#ifndef LEDGER_CORE_BITCOINLIKEACCOUNT_HPP
#define LEDGER_CORE_BITCOINLIKEACCOUNT_HPP

#include "BitcoinLikeWallet.hpp"
#include <api/BitcoinLikeAccount.hpp>
#include "explorers/BitcoinLikeBlockchainExplorer.hpp"

namespace ledger {
    namespace core {
        class BitcoinLikeAccount : public api::BitcoinLikeAccount {
        public:
            static const int FLAG_NEW_TRANSACTION = 0x01;
            static const int FLAG_TRANSACTION_UPDATED = 0x01 << 1;
            static const int FLAG_TRANSACTION_IGNORED = 0x00;
            static const int FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS = 0x01 << 2;
            static const int FLAG_TRANSACTION_ON_USED_ADDRESS = 0x01 << 3;
            /**
             *
             * @param transaction
             * @return A flag indicating if the transaction was ignored, inserted
             */
            int putTransaction(const BitcoinLikeBlockchainExplorer::Transaction& transaction);
            /**
             *
             * @param block
             * @return true if the block wasn't already known.
             */
            bool putBlock(const BitcoinLikeBlockchainExplorer::Block block);

        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEACCOUNT_HPP
