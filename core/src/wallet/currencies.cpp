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
#include "currencies.hpp"
#include "bitcoin/networks.hpp"
#include <wallet/common/CurrencyBuilder.hpp>

namespace ledger {
    namespace core {
        namespace currencies {

            const api::Currency BITCOIN =
                Currency("bitcoin")
                .forkOfBitcoin(networks::BITCOIN)
                .bip44(0)
                .paymentUri("bitcoin")
                    .unit("satoshi", 0, "satoshi")
                    .unit("bitcoin", 8, "BTC")
                    .unit("milli-bitcoin", 5, "mBTC")
                    .unit("micro-bitcoin", 2, "μBTC");

            const api::Currency BITCOIN_TESTNET =
                    Currency("bitcoin_testnet")
                            .forkOfBitcoin(networks::BITCOIN_TESTNET)
                            .bip44(1)
                            .paymentUri("bitcoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("bitcoin", 8, "BTC")
                            .unit("milli-bitcoin", 5, "mBTC")
                            .unit("micro-bitcoin", 2, "μBTC");

            const api::Currency BITCOIN_CASH =
                    Currency("bitcoin_cash")
                            .forkOfBitcoin(networks::BITCOIN_CASH)
                            .bip44(0)
                            .paymentUri("bitcoin_cash")
                            .unit("satoshi", 0, "satoshi")
                            .unit("bitcoin cash", 8, "BTH")
                            .unit("mBCH", 5, "mBCH")
                            .unit("bit", 2, "bit");

            const api::Currency BITCOIN_GOLD =
                    Currency("bitcoin_gold")
                            .forkOfBitcoin(networks::BITCOIN_GOLD)
                            .bip44(0)
                            .paymentUri("bitcoin_gold")
                            .unit("satoshi", 0, "satoshi")
                            .unit("bitcoin gold", 8, "BTG")
                            .unit("mBCG", 5, "mBCG")
                            .unit("bit", 2, "bit");

            const api::Currency ZCASH =
                    Currency("zcash")
                            .forkOfBitcoin(networks::ZCASH)
                            .bip44(133)
                            .paymentUri("zcash")
                            .unit("zatoshi", 0, "zatoshi")
                            .unit("zcash", 8, "ZEC");

            const api::Currency ZENCASH =
                    Currency("zencash")
                            .forkOfBitcoin(networks::ZENCASH)
                            .bip44(121)
                            .paymentUri("zencash")
                            .unit("zencash", 8, "ZEN");

            const api::Currency LITECOIN =
                    Currency("litecoin")
                            .forkOfBitcoin(networks::LITECOIN)
                            .bip44(2)
                            .paymentUri("litecoin")
                            .unit("litecoin", 8, "LTC");

            const api::Currency PEERCOIN =
                    Currency("peercoin")
                            .forkOfBitcoin(networks::PEERCOIN)
                            .bip44(6)
                            .paymentUri("peercoin")
                            .unit("peercoin", 6, "PPC")
                            .unit("milli-peercoin", 3, "mPPC");

            const api::Currency DIGIBYTE =
                    Currency("digibyte")
                            .forkOfBitcoin(networks::DIGIBYTE)
                            .bip44(20)
                            .paymentUri("digibyte")
                            .unit("digibyte", 8, "DGB");

            const api::Currency HCASH =
                    Currency("hcash")
                            .forkOfBitcoin(networks::HCASH)
                            .bip44(171)
                            .paymentUri("hcash")
                            .unit("hshare", 8, "HSR");

            const api::Currency QTUM =
                    Currency("hcash")
                            .forkOfBitcoin(networks::QTUM)
                            .bip44(88)
                            .paymentUri("qtum")
                            .unit("qtum", 8, "QTUM");

            const api::Currency STEALTHCOIN =
                    Currency("stealthcoin")
                            .forkOfBitcoin(networks::STEALTHCOIN)
                            .bip44(125)
                            .paymentUri("stealthcoin")
                            .unit("stealthcoin", 6, "XST")
                            .unit("milli-stealthcoin", 3, "mXST");

            const api::Currency VERTCOIN =
                    Currency("vertcoin")
                            .forkOfBitcoin(networks::VERTCOIN)
                            .bip44(128)
                            .paymentUri("vertcoin")
                            .unit("vertcoin", 8, "VTC")
                            .unit("milli-vertcoin", 5, "mVTC");

            const api::Currency VIACOIN =
                    Currency("viacoin")
                            .forkOfBitcoin(networks::VIACOIN)
                            .bip44(14)
                            .paymentUri("viacoin")
                            .unit("viacoin", 8, "VIA")
                            .unit("milli-viacoin", 5, "mVIA");

            const api::Currency DASH =
                    Currency("dash")
                            .forkOfBitcoin(networks::DASH)
                            .bip44(5)
                            .paymentUri("dash")
                            .unit("dash", 8, "DASH");

            const api::Currency DOGECOIN =
                    Currency("dogecoin")
                            .forkOfBitcoin(networks::DOGECOIN)
                            .bip44(3)
                            .paymentUri("dogecoin")
                            .unit("dogecoin", 8, "DOGE")
                            .unit("milli-dogecoin", 5, "mDOGE");

            const api::Currency STRATIS =
                    Currency("stratis")
                            .forkOfBitcoin(networks::STRATIS)
                            .bip44(105)
                            .paymentUri("stratis")
                            .unit("stratis", 8, "STRAT");

            const api::Currency KOMODO =
                    Currency("komodo")
                            .forkOfBitcoin(networks::KOMODO)
                            .bip44(141)
                            .paymentUri("komodo")
                            .unit("komodo", 8, "KMD");

            const api::Currency POSWALLET =
                    Currency("poswallet")
                            .forkOfBitcoin(networks::POSWALLET)
                            .bip44(47)
                            .paymentUri("poswallet")
                            .unit("poswallet", 8, "POSW");

            const api::Currency PIVX =
                    Currency("pivx")
                            .forkOfBitcoin(networks::PIVX)
                            .bip44(77)
                            .paymentUri("pivx")
                            .unit("pivx", 8, "PIVX")
                            .unit("milli-pivx", 5, "mPIVX");

            const std::vector<api::Currency> ALL({
                BITCOIN,
                BITCOIN_TESTNET,
                BITCOIN_CASH,
                BITCOIN_GOLD,
                ZCASH,
                ZENCASH,
                LITECOIN,
                PEERCOIN,
                DIGIBYTE,
                HCASH,
                QTUM,
                STEALTHCOIN,
                VERTCOIN,
                VIACOIN,
                DASH,
                DOGECOIN,
                STRATIS,
                KOMODO,
                POSWALLET,
                PIVX
            });
        }
    }
}
