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

            const std::vector<api::Currency> ALL({
                BITCOIN,
                BITCOIN_TESTNET,
                BITCOIN_CASH,
                BITCOIN_GOLD,
                ZCASH,
                ZENCASH
            });
        }
    }
}
