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
#include "ethereum/ethereumNetworks.hpp"
#include "ripple/rippleNetworks.h"
#include "tezos/tezosNetworks.h"
#include "stellar/stellarNetworks.h"
#include <wallet/common/CurrencyBuilder.hpp>

namespace ledger {
    namespace core {
        namespace currencies {

            const api::Currency BITCOIN =
                Currency("bitcoin")
                .forkOfBitcoin(networks::getNetworkParameters("bitcoin"))
                .bip44(0)
                .paymentUri("bitcoin")
                    .unit("satoshi", 0, "satoshi")
                    .unit("bitcoin", 8, "BTC")
                    .unit("milli-bitcoin", 5, "mBTC")
                    .unit("micro-bitcoin", 2, "μBTC");

            const api::Currency BITCOIN_TESTNET =
                    Currency("bitcoin_testnet")
                            .forkOfBitcoin(networks::getNetworkParameters("bitcoin_testnet"))
                            .bip44(1)
                            .paymentUri("bitcoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("bitcoin", 8, "BTC")
                            .unit("milli-bitcoin", 5, "mBTC")
                            .unit("micro-bitcoin", 2, "μBTC");

            const api::Currency BITCOIN_CASH =
                    Currency("bitcoin_cash")
                            .forkOfBitcoin(networks::getNetworkParameters("bitcoin_cash"))
                            .bip44(145)
                            .paymentUri("bitcoin_cash")
                            .unit("satoshi", 0, "satoshi")
                            .unit("bitcoin cash", 8, "BCH")
                            .unit("mBCH", 5, "mBCH")
                            .unit("bit", 2, "bit");

            const api::Currency BITCOIN_GOLD =
                    Currency("bitcoin_gold")
                            .forkOfBitcoin(networks::getNetworkParameters("bitcoin_gold"))
                            .bip44(156)
                            .paymentUri("bitcoin_gold")
                            .unit("satoshi", 0, "satoshi")
                            .unit("bitcoin gold", 8, "BTG")
                            .unit("mBTG", 5, "mBTG")
                            .unit("bit", 2, "bit");

            const api::Currency ZCASH =
                    Currency("zcash")
                            .forkOfBitcoin(networks::getNetworkParameters("zcash"))
                            .bip44(133)
                            .paymentUri("zcash")
                            .unit("satoshi", 0, "satoshi")
                            .unit("zcash", 8, "ZEC");

            const api::Currency ZENCASH =
                    Currency("zencash")
                            .forkOfBitcoin(networks::getNetworkParameters("zencash"))
                            .bip44(121)
                            .paymentUri("zencash")
                            .unit("satoshi", 0, "satoshi")
                            .unit("zencash", 8, "ZEN");

            const api::Currency LITECOIN =
                    Currency("litecoin")
                            .forkOfBitcoin(networks::getNetworkParameters("litecoin"))
                            .bip44(2)
                            .paymentUri("litecoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("litecoin", 8, "LTC")
                            .unit("milli-litecoin", 5, "mLTC")
                            .unit("micro-litecoin", 2, "μLTC");

            const api::Currency PEERCOIN =
                    Currency("peercoin")
                            .forkOfBitcoin(networks::getNetworkParameters("peercoin"))
                            .bip44(6)
                            .paymentUri("peercoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("peercoin", 6, "PPC")
                            .unit("milli-peercoin", 3, "mPPC");

            const api::Currency DIGIBYTE =
                    Currency("digibyte")
                            .forkOfBitcoin(networks::getNetworkParameters("digibyte"))
                            .bip44(20)
                            .paymentUri("digibyte")
                            .unit("satoshi", 0, "satoshi")
                            .unit("digibyte", 8, "DGB");

            const api::Currency HCASH =
                    Currency("hcash")
                            .forkOfBitcoin(networks::getNetworkParameters("hcash"))
                            .bip44(171)
                            .paymentUri("hcash")
                            .unit("satoshi", 0, "satoshi")
                            .unit("hshare", 8, "HSR");

            const api::Currency QTUM =
                    Currency("qtum")
                            .forkOfBitcoin(networks::getNetworkParameters("qtum"))
                            .bip44(88)
                            .paymentUri("qtum")
                            .unit("satoshi", 0, "satoshi")
                            .unit("qtum", 8, "QTUM");

            const api::Currency STEALTHCOIN =
                    Currency("stealthcoin")
                            .forkOfBitcoin(networks::getNetworkParameters("stealthcoin"))
                            .bip44(125)
                            .paymentUri("stealthcoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("stealthcoin", 6, "XST")
                            .unit("milli-stealthcoin", 3, "mXST");

            const api::Currency VERTCOIN =
                    Currency("vertcoin")
                            .forkOfBitcoin(networks::getNetworkParameters("vertcoin"))
                            .bip44(128)
                            .paymentUri("vertcoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("vertcoin", 8, "VTC")
                            .unit("milli-vertcoin", 5, "mVTC");

            const api::Currency VIACOIN =
                    Currency("viacoin")
                            .forkOfBitcoin(networks::getNetworkParameters("viacoin"))
                            .bip44(14)
                            .paymentUri("viacoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("viacoin", 8, "VIA")
                            .unit("milli-viacoin", 5, "mVIA");

            const api::Currency DASH =
                    Currency("dash")
                            .forkOfBitcoin(networks::getNetworkParameters("dash"))
                            .bip44(5)
                            .paymentUri("dash")
                            .unit("satoshi", 0, "satoshi")
                            .unit("dash", 8, "DASH");

            const api::Currency DOGECOIN =
                    Currency("dogecoin")
                            .forkOfBitcoin(networks::getNetworkParameters("dogecoin"))
                            .bip44(3)
                            .paymentUri("dogecoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("dogecoin", 8, "DOGE")
                            .unit("milli-dogecoin", 5, "mDOGE");

            const api::Currency STRATIS =
                    Currency("stratis")
                            .forkOfBitcoin(networks::getNetworkParameters("stratis"))
                            .bip44(105)
                            .paymentUri("stratis")
                            .unit("satoshi", 0, "satoshi")
                            .unit("stratis", 8, "STRAT");

            const api::Currency KOMODO =
                    Currency("komodo")
                            .forkOfBitcoin(networks::getNetworkParameters("komodo"))
                            .bip44(141)
                            .paymentUri("komodo")
                            .unit("satoshi", 0, "satoshi")
                            .unit("komodo", 8, "KMD");

            const api::Currency POSWALLET =
                    Currency("poswallet")
                            .forkOfBitcoin(networks::getNetworkParameters("poswallet"))
                            .bip44(47)
                            .paymentUri("poswallet")
                            .unit("satoshi", 0, "satoshi")
                            .unit("poswallet", 8, "POSW");

            const api::Currency PIVX =
                    Currency("pivx")
                            .forkOfBitcoin(networks::getNetworkParameters("pivx"))
                            .bip44(77)
                            .paymentUri("pivx")
                            .unit("satoshi", 0, "satoshi")
                            .unit("pivx", 8, "PIVX")
                            .unit("milli-pivx", 5, "mPIVX");

            const api::Currency CLUBCOIN =
                    Currency("clubcoin")
                            .forkOfBitcoin(networks::getNetworkParameters("clubcoin"))
                            .bip44(79)
                            .paymentUri("clubcoin")
                            .unit("satoshi", 0, "satoshi")
                            .unit("clubcoin", 8, "CLUB");

            const api::Currency DECRED =
                    Currency("decred")
                            .forkOfBitcoin(networks::getNetworkParameters("decred"))
                            .bip44(42)
                            .paymentUri("decred")
                            .unit("satoshi", 0, "satoshi")
                            .unit("decred", 8, "DCR")
                            .unit("milli-decred", 5, "mDCR");

            const api::Currency STAKENET =
                    Currency("stakenet")
                            .forkOfBitcoin(networks::getNetworkParameters("stakenet"))
                            .bip44(384)
                            .paymentUri("stakenet")
                            .unit("satoshi", 0, "satoshi")
                            .unit("stakenet", 8, "XSN");

            //Reference for ETH coinTypes: https://github.com/LedgerHQ/ledger-live-common/blob/b0196ae9031447f41f8e641f0ec5d3e2b72be83c/src/data/cryptocurrencies.js
            const api::Currency ETHEREUM =
                    Currency("ethereum")
                            .bip44(60)
                            .forkOfEthereum(networks::getEthLikeNetworkParameters("ethereum"))
                            .paymentUri("ethereum")
                            .unit("wei", 0, "wei")
                            .unit("ether", 18, "ETH")
                            .unit("kwei", 3, "kwei")
                            .unit("mwei", 6, "mwei")
                            .unit("gwei", 9, "gwei");

            const api::Currency ETHEREUM_ROPSTEN =
                    Currency("ethereum_ropsten")
                            .bip44(1)
                            .forkOfEthereum(networks::getEthLikeNetworkParameters("ethereum_ropsten"))
                            .paymentUri("ethereum")
                            .unit("wei", 0, "wei")
                            .unit("ether", 18, "ETH")
                            .unit("kwei", 3, "kwei")
                            .unit("mwei", 6, "mwei")
                            .unit("gwei", 9, "gwei");

            const api::Currency ETHEREUM_CLASSIC =
                    Currency("ethereum_classic")
                            .bip44(61)
                            .forkOfEthereum(networks::getEthLikeNetworkParameters("ethereum_classic"))
                            .paymentUri("ethereum")
                            .unit("wei", 0, "wei")
                            .unit("ether", 18, "ETH")
                            .unit("kwei", 3, "kwei")
                            .unit("mwei", 6, "mwei")
                            .unit("gwei", 9, "gwei");

            const api::Currency RIPPLE =
                    Currency("ripple")
                            .bip44(144)
                            .forkOfRipple(networks::getRippleLikeNetworkParameters("ripple"))
                            .paymentUri("ripple")
                            .unit("drop", 0, "drop")
                            .unit("XRP", 6, "XRP");

            const api::Currency TEZOS =
                    Currency("tezos")
                            .bip44(1729)
                            .forkOfTezos(networks::getTezosLikeNetworkParameters("tezos"))
                            .paymentUri("tezos")
                            .unit("mXTZ", 0, "mXTZ")
                            .unit("XTZ", 3, "XTZ");

                const api::Currency STELLAR =
                    Currency("stellar")
                            .bip44(148)
                            .forkOfStellar(networks::getStellarLikeNetworkParameters("stellar"))
                            .paymentUri("stellar")
                            .unit("stroop", 0, "stroop")
                            .unit("XLM", 7, "XLM");


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
                PIVX,
                CLUBCOIN,
                DECRED,
                STAKENET,
                ETHEREUM,
                ETHEREUM_ROPSTEN,
                ETHEREUM_CLASSIC,
                RIPPLE,
                TEZOS,
                STELLAR
            });
        }
    }
}
