/*
 *
 * currencies
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/05/2017.
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
#ifndef LEDGER_CORE_CURRENCIES_HPP
#define LEDGER_CORE_CURRENCIES_HPP

#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER)
        #include <libcore_export.h>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

#include <api/Currency.hpp>

namespace ledger {
    namespace core {
        namespace currencies {
            extern LIBCORE_EXPORT const std::vector<api::Currency> ALL;
            extern LIBCORE_EXPORT const api::Currency BITCOIN;
            extern LIBCORE_EXPORT const api::Currency BITCOIN_TESTNET;
            extern LIBCORE_EXPORT const api::Currency BITCOIN_CASH;
            extern LIBCORE_EXPORT const api::Currency BITCOIN_GOLD;
            extern LIBCORE_EXPORT const api::Currency ZCASH;
            extern LIBCORE_EXPORT const api::Currency ZENCASH;
            extern LIBCORE_EXPORT const api::Currency LITECOIN;
            extern LIBCORE_EXPORT const api::Currency PEERCOIN;
            extern LIBCORE_EXPORT const api::Currency DIGIBYTE;
            extern LIBCORE_EXPORT const api::Currency HCASH;
            extern LIBCORE_EXPORT const api::Currency QTUM;
            extern LIBCORE_EXPORT const api::Currency STEALTHCOIN;
            extern LIBCORE_EXPORT const api::Currency VERTCOIN;
            extern LIBCORE_EXPORT const api::Currency VIACOIN;
            extern LIBCORE_EXPORT const api::Currency DASH;
            extern LIBCORE_EXPORT const api::Currency DOGECOIN;
            extern LIBCORE_EXPORT const api::Currency STRATIS;
            extern LIBCORE_EXPORT const api::Currency KOMODO;
            extern LIBCORE_EXPORT const api::Currency POSWALLET;
            extern LIBCORE_EXPORT const api::Currency PIVX;
            extern LIBCORE_EXPORT const api::Currency CLUBCOIN;
            extern LIBCORE_EXPORT const api::Currency DECRED;
            extern LIBCORE_EXPORT const api::Currency STAKENET;
            extern LIBCORE_EXPORT const api::Currency ETHEREUM;
            extern LIBCORE_EXPORT const api::Currency ETHEREUM_ROPSTEN;
            extern LIBCORE_EXPORT const api::Currency ETHEREUM_CLASSIC;
            extern LIBCORE_EXPORT const api::Currency RIPPLE;
            extern LIBCORE_EXPORT const api::Currency TEZOS;
        };
    }
}


#endif //LEDGER_CORE_CURRENCIES_HPP
