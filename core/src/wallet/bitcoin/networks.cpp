/*
 *
 * networks
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/01/2017.
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
#include "networks.hpp"

namespace ledger {
    namespace core {

        namespace networks {

            enum sigHashType : uint8_t {
                SIGHASH_ALL = 0x01,
                SIGHASH_NONE = 0x02,
                SIGHASH_SINGLE = 0x03,
                SIGHASH_ANYONECANPAY = 0x80,
                SIGHASH_BTC_FORKS = 0x41
            };

            const api::BitcoinLikeNetworkParameters BITCOIN(
                    "btc",
                    {0x00},
                    {0x05},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    546,
                    "Bitcoin signed message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters BITCOIN_TESTNET(
                    "btc_testnet",
                    {0x6F},
                    {0xC4},
                    {0x04, 0x35, 0x87, 0xCF},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    546,
                    "Bitcoin signed message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters BITCOIN_CASH(
                    "abc",
                    {0x00},
                    {0x05},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    5430,
                    "Bitcoin signed message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters BITCOIN_CASH(
                    "abc",
                    {0x00},
                    {0x05},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    5430,
                    "Bitcoin signed message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_BTC_FORKS}
            );

            const api::BitcoinLikeNetworkParameters BITCOIN_GOLD(
                    "btg",
                    {0x00},
                    {0x05},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    5430,
                    "Bitcoin gold signed message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_BTC_FORKS}
            );

            const std::vector<api::BitcoinLikeNetworkParameters> ALL
            ({
                BITCOIN,
                BITCOIN_TESTNET,
                BITCOIN_CASH,
                BITCOIN_GOLD
            });
        }
    }
}