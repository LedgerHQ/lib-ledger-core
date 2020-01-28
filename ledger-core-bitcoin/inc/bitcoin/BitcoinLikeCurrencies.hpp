/*
 *
 * BitcoinLikeCurrencies
 *
 * Created Alexis Le Provost on 2019/12/12
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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
        #include <core/LibCoreExport.hpp>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

#include <core/api/Currency.hpp>

namespace ledger {
    namespace core {
        namespace currencies {
            api::Currency bitcoin();
            api::Currency bitcoin_testnet();
            api::Currency bitcoin_cash();
            api::Currency bitcoin_gold();
            api::Currency zcash();
            api::Currency zencash();
            api::Currency litecoin();
            api::Currency peercoin();
            api::Currency digibyte();
            api::Currency hcash();
            api::Currency qtum();
            api::Currency stealthcoin();
            api::Currency vertcoin();
            api::Currency viacoin();
            api::Currency dash();
            api::Currency dogecoin();
            api::Currency stratis();
            api::Currency komodo();
            api::Currency poswallet();
            api::Currency pivx();
            api::Currency clubcoin();
            api::Currency decred();
            api::Currency stakenet();
        }
    }
}