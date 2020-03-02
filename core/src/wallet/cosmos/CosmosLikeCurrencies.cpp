/*
 *
 * CosmosLikeCurrencies
 *
 * Created by Gerry Agbobada on 21/01/2020
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

#include <core/wallet/CurrencyBuilder.hpp>

#include <cosmos/CosmosLikeCoinID.hpp>
#include <cosmos/CosmosLikeCurrencies.hpp>
#include <cosmos/CosmosNetworks.hpp>

namespace ledger {
    namespace core {
        namespace currencies {
            // Minimum value is uatom according to
            // https://github.com/cosmos/gaia/blob/ba8d2b3177e1b891b72d6f40538fc2c6344bdeac/docs/delegators/delegator-guide-cli.md#sending-transactions
            api::Currency const ATOM =
                CurrencyBuilder("atom")
                .bip44(ATOM_COIN_ID)
                .paymentUri("cosmos")
                .unit("uatom", 0, "uatom")
                .unit("matom", 3, "matom")
                .unit("atom", 6, "atom");
        }
    }
}
