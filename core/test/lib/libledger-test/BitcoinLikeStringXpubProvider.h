/*
 *
 * BitcoinLikeStringXpubProvider
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/06/2017.
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
#ifndef LEDGER_CORE_BITCOINLIKESTRINGXPUBPROVIDER_H
#define LEDGER_CORE_BITCOINLIKESTRINGXPUBPROVIDER_H

#include <src/api/BitcoinLikeBase58ExtendedPublicKeyProvider.hpp>
#include <vector>

class BitcoinLikeStringXpubProvider : public ledger::core::api::BitcoinLikeBase58ExtendedPublicKeyProvider {
public:
    BitcoinLikeStringXpubProvider(const std::vector<std::pair<std::string, std::string>>& xpubs);
    void get(const std::string &deviceId, const std::string &path,
             const ledger::core::api::BitcoinLikeNetworkParameters &params,
             const std::shared_ptr<ledger::core::api::StringCompletionBlock> &completion) override;

private:
    std::vector<std::pair<std::string, std::string>> _xpubs;
};


#endif //LEDGER_CORE_BITCOINLIKESTRINGXPUBPROVIDER_H
