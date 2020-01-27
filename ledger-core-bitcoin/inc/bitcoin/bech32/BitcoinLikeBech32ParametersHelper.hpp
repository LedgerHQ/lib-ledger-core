/*
 *
 * BitcoinLikeBech32ParametersHelper
 *
 * Created by Gerry Agbobada on 2020/01/23
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

#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER)
        #include <libcore_export.h>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

#include <core/math/bech32/Bech32ParametersHelper.hpp>

namespace ledger {
    namespace core {
        struct BitcoinLikeBech32ParametersHelper : public Bech32ParametersHelper<BitcoinLikeBech32ParametersHelper> {
            using tBase = Bech32ParametersHelper<BitcoinLikeBech32ParametersHelper>;

            static const Bech32Parameters::Bech32Struct getCoinLikeBech32Params(const std::string &networkIdentifier);
            static bool insertCoinLikeParameters(soci::session& sql, const Bech32Parameters::Bech32Struct &params);
        };
    }
}
