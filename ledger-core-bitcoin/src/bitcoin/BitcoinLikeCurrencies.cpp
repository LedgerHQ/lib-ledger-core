/*
 *
 * RippleLikeCurrencies
 *
 * Created by Alexis Le Provost on 2019/12/12
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

#include <core/wallet/CurrencyBuilder.hpp>

#include <bitcoin/BitcoinLikeCoinID.hpp>
#include <bitcoin/BitcoinLikeCurrencies.hpp>

namespace ledger {
    namespace core {
        namespace currencies {
                api::Currency bitcoin() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("bitcoin")
                                        .bip44(BITCOIN_COIN_ID)
                                        .paymentUri("bitcoin")
                                        .unit("satoshi", 0, "satoshi")
                                        .unit("bitcoin", 8, "BTC")
                                        .unit("milli-bitcoin", 5, "mBTC")
                                        .unit("micro-bitcoin", 2, "μBTC");

                        return CURRENCY;
                }

                api::Currency bitcoin_testnet() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("bitcoin_testnet")
                                .bip44(BITCOIN_TESTNET_COIN_ID)
                                .paymentUri("bitcoin")
                                .unit("satoshi", 0, "satoshi")
                                .unit("bitcoin", 8, "BTC")
                                .unit("milli-bitcoin", 5, "mBTC")
                                .unit("micro-bitcoin", 2, "μBTC");

                        return CURRENCY;
                }

                api::Currency bitcoin_cash() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("bitcoin_cash")
                                .bip44(BITCOIN_CASH_COIN_ID)
                                .paymentUri("bitcoin_cash")
                                .unit("satoshi", 0, "satoshi")
                                .unit("bitcoin cash", 8, "BCH")
                                .unit("mBCH", 5, "mBCH")
                                .unit("bit", 2, "bit");

                        return CURRENCY;
                }

                api::Currency bitcoin_gold() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("bitcoin_gold")
                                .bip44(BITCOIN_GOLD_COIN_ID)
                                .paymentUri("bitcoin_gold")
                                .unit("satoshi", 0, "satoshi")
                                .unit("bitcoin gold", 8, "BTG")
                                .unit("mBTG", 5, "mBTG")
                                .unit("bit", 2, "bit");

                        return CURRENCY;
                }

                api::Currency zcash() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("zcash")
                                .bip44(133)
                                .paymentUri("zcash")
                                .unit("satoshi", 0, "satoshi")
                                .unit("zcash", 8, "ZEC");

                        return CURRENCY;
                }

                api::Currency zencash() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("zencash")
                                .bip44(ZCASH_COIN_ID)
                                .paymentUri("zencash")
                                .unit("satoshi", 0, "satoshi")
                                .unit("zencash", 8, "ZEN");

                        return CURRENCY;
                }

                api::Currency litecoin() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("litecoin")
                                .bip44(LITE_COIN_ID)
                                .paymentUri("litecoin")
                                .unit("satoshi", 0, "satoshi")
                                .unit("litecoin", 8, "LTC")
                                .unit("milli-litecoin", 5, "mLTC")
                                .unit("micro-litecoin", 2, "μLTC");

                        return CURRENCY;
                }

                api::Currency peercoin() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("peercoin")
                                .bip44(PEERCOIN_COIN_ID)
                                .paymentUri("peercoin")
                                .unit("satoshi", 0, "satoshi")
                                .unit("peercoin", 6, "PPC")
                                .unit("milli-peercoin", 3, "mPPC");

                        return CURRENCY;
                }

                api::Currency digibyte() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("digibyte")
                                .bip44(DIGIBYTE_COIN_ID)
                                .paymentUri("digibyte")
                                .unit("satoshi", 0, "satoshi")
                                .unit("digibyte", 8, "DGB");

                        return CURRENCY;
                }

                api::Currency hcash() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("hcash")
                                .bip44(HCASH_COIN_ID)
                                .paymentUri("hcash")
                                .unit("satoshi", 0, "satoshi")
                                .unit("hshare", 8, "HSR");

                        return CURRENCY;
                }

                api::Currency qtum() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("qtum")
                                .bip44(QTUM_COIN_ID)
                                .paymentUri("qtum")
                                .unit("satoshi", 0, "satoshi")
                                .unit("qtum", 8, "QTUM");

                        return CURRENCY;
                }

                api::Currency stealthcoin() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("stealthcoin")
                                .bip44(STEALTHCOIN_COIN_ID)
                                .paymentUri("stealthcoin")
                                .unit("satoshi", 0, "satoshi")
                                .unit("stealthcoin", 6, "XST")
                                .unit("milli-stealthcoin", 3, "mXST");

                        return CURRENCY;
                }

                api::Currency vertcoin() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("vertcoin")
                                .bip44(VERTCOIN_COIN_ID)
                                .paymentUri("vertcoin")
                                .unit("satoshi", 0, "satoshi")
                                .unit("vertcoin", 8, "VTC")
                                .unit("milli-vertcoin", 5, "mVTC");

                        return CURRENCY;
                }

                api::Currency viacoin() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("viacoin")
                                .bip44(VIACOIN_COIN_ID)
                                .paymentUri("viacoin")
                                .unit("satoshi", 0, "satoshi")
                                .unit("viacoin", 8, "VIA")
                                .unit("milli-viacoin", 5, "mVIA");

                        return CURRENCY;
                }

                api::Currency dash() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("dash")
                                .bip44(DASH_COIN_ID)
                                .paymentUri("dash")
                                .unit("satoshi", 0, "satoshi")
                                .unit("dash", 8, "DASH");

                        return CURRENCY;
                }

                api::Currency dogecoin() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("dogecoin")
                                .bip44(DOGECOIN_COIN_ID)
                                .paymentUri("dogecoin")
                                .unit("satoshi", 0, "satoshi")
                                .unit("dogecoin", 8, "DOGE")
                                .unit("milli-dogecoin", 5, "mDOGE");

                        return CURRENCY;
                }

                api::Currency stratis() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("stratis")
                                .bip44(STRATIS_COIN_ID)
                                .paymentUri("stratis")
                                .unit("satoshi", 0, "satoshi")
                                .unit("stratis", 8, "STRAT");

                        return CURRENCY;
                }

                api::Currency komodo() {    
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("komodo")
                                .bip44(KOMODO_COIN_ID)
                                .paymentUri("komodo")
                                .unit("satoshi", 0, "satoshi")
                                .unit("komodo", 8, "KMD");

                        return CURRENCY;
                }

                api::Currency poswallet() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("poswallet")
                                .bip44(POSWALLET_COIN_ID)
                                .paymentUri("poswallet")
                                .unit("satoshi", 0, "satoshi")
                                .unit("poswallet", 8, "POSW");

                        return CURRENCY;
                }

                api::Currency pivx() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("pivx")
                                .bip44(PIVX_COIN_ID)
                                .paymentUri("pivx")
                                .unit("satoshi", 0, "satoshi")
                                .unit("pivx", 8, "PIVX")
                                .unit("milli-pivx", 5, "mPIVX");

                        return CURRENCY;
                }

                api::Currency clubcoin() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("clubcoin")
                                .bip44(CLUBCOIN_COIN_ID)
                                .paymentUri("clubcoin")
                                .unit("satoshi", 0, "satoshi")
                                .unit("clubcoin", 8, "CLUB");

                        return CURRENCY;
                }

                api::Currency decred() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("decred")
                                .bip44(DECRED_COIN_ID)
                                .paymentUri("decred")
                                .unit("satoshi", 0, "satoshi")
                                .unit("decred", 8, "DCR")
                                .unit("milli-decred", 5, "mDCR");

                        return CURRENCY;
                }

                api::Currency stakenet() {
                        static api::Currency const CURRENCY =
                        CurrencyBuilder("stakenet")
                                .bip44(STAKENET_COIN_ID)
                                .paymentUri("stakenet")
                                .unit("satoshi", 0, "satoshi")
                                .unit("stakenet", 8, "XSN");

                        return CURRENCY;
                }
       }
   }
}