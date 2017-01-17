/*
 *
 * BitcoinLikeKeyChain
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/01/2017.
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
#ifndef LEDGER_CORE_BITCOINLIKEKEYCHAIN_HPP
#define LEDGER_CORE_BITCOINLIKEKEYCHAIN_HPP

#include <set>
#include <string>
#include <vector>

namespace ledger {
    namespace core {
        enum BitcoinLikeKeyPurpose {
            SEND, RECEIVE
        };

        class BitcoinLikeKeyChain {
        public:
            /**
             * Marks the given keys as used.
             * @param keys The keys to mark as used
             * @return true if at least one of the keys was previously unused.
             */
            virtual bool markAsUsed(const std::set<std::string>& keys) = 0;
            virtual std::vector<std::string> getAllKeys() const = 0;
            virtual std::vector<std::string> getNextUnusedKeys(BitcoinLikeKeyPurpose purpose, unsigned int numberOfKeys) const = 0;
            virtual std::vector<std::string> getAllObservableKeys() const = 0;


        private:

        };
    }
}

#endif //LEDGER_CORE_BITCOINLIKEKEYCHAIN_HPP
