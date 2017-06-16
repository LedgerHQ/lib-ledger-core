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
#include "BitcoinLikeStringXpubProvider.h"
#include <src/utils/DerivationPath.hpp>
#include <src/api/StringCompletionBlock.hpp>
#include <src/api/ErrorCode.hpp>
#include <src/api/Error.hpp>
#include <src/utils/optional.hpp>

using namespace ledger::core;

void BitcoinLikeStringXpubProvider::get(const std::string &deviceId, const std::string &path,
                                        const ledger::core::api::BitcoinLikeNetworkParameters &params,
                                        const std::shared_ptr<ledger::core::api::StringCompletionBlock> &completion) {
    DerivationPath p(path);
    int index = 0;
    for (auto& item : _xpubs) {
        if (item.first == deviceId || deviceId.size() == 0) {
            if (index == (p.getNonHardenedLastChildNum())) {
                completion->complete(std::experimental::optional<std::string>(item.second), std::experimental::optional<api::Error>());
                return ;
            }
            index += 1;
        }
    }
    completion->complete(std::experimental::optional<std::string>(), std::experimental::optional<api::Error>(api::Error(api::ErrorCode::RUNTIME_ERROR, "XPUB not found")));
}

BitcoinLikeStringXpubProvider::BitcoinLikeStringXpubProvider(
        const std::vector<std::pair<std::string, std::string>> &xpubs) {
    _xpubs = xpubs;
}
