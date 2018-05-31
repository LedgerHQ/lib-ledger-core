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
                SIGHASH_ALL = 1,
                SIGHASH_NONE = 2,
                SIGHASH_SINGLE = 3,
                SIGHASH_FORKID = 0x40,
                SIGHASH_ANYONECANPAY = 0x80
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
                    {sigHashType::SIGHASH_ALL | sigHashType::SIGHASH_FORKID}
            );


            const api::BitcoinLikeNetworkParameters BITCOIN_GOLD(
                    "btg",
                    {0x26},//{0x00}, TODO: check if tx use old or new
                    {0x17},//{0x05},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    5430,
                    "Bitcoin gold signed message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL | sigHashType::SIGHASH_FORKID}
            );

            const api::BitcoinLikeNetworkParameters ZCASH(
                    "zec",
                    {0x1C, 0xB8},
                    {0x1C, 0xBD},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "Zcash Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters ZENCASH(
                    "zen",
                    {0x20, 0x89},
                    {0x20, 0x96},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "Zencash Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters LITECOIN(
                    "ltc",
                    {0x30},
                    {0x32},
                    {0x01, 0x9D, 0xA4, 0x62},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "Litecoin Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters PEERCOIN(
                    "ppc",
                    {0x37},
                    {0x75},
                    {0xE6, 0xE8, 0xE9, 0xE5},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "PPCoin Signed Message:\n",
                    true,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters DIGIBYTE(
                    "dgb",
                    {0x1E},
                    {0x05},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "DigiByte Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters HCASH(
                    "hsr",
                    {0x28},
                    {0x78},
                    {0x04, 0x88, 0xC2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "HShare Signed Message:\n",
                    true,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters QTUM(
                    "qtum",
                    {0x3A},
                    {0x32},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "Qtum Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters STEALTHCOIN(
                    "xst",
                    {0x3E},
                    {0x55},
                    {0x8F, 0x62, 0x4B, 0x66},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "StealthCoin Signed Message:\n",
                    true,
                    15,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters VERTCOIN(
                    "vtc",
                    {0x47},
                    {0x05},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "VertCoin Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters VIACOIN(
                    "via",
                    {0x47},
                    {0x21},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "ViaCoin Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters DASH(
                    "dash",
                    {0x4C},
                    {0x01},
                    {0x02, 0xFE, 0x52, 0xF8},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "DarkCoin Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters DOGECOIN(
                    "dogecoin",
                    {0x1E},
                    {0x16},
                    {0x02, 0xFA, 0xCA, 0xFD},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "DogeCoin Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters STRATIS(
                    "stratis",
                    {0x3F},
                    {0x7D},
                    {0x04, 0x88, 0xC2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "Stratis Signed Message:\n",
                    true,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters KOMODO(
                    "komodo",
                    {0x3C},
                    {0x55},
                    {0xF9, 0xEE, 0xE4, 0x8D},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "Komodo Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters POSWALLET(
                    "poswallet",
                    {0x37},
                    {0x55},
                    {0x04, 0x88, 0xB2, 0x1E},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "PosWallet Signed Message:\n",
                    true,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const api::BitcoinLikeNetworkParameters PIVX(
                    "pivx",
                    {0x1E},
                    {0x0D},
                    {0x02, 0x2D, 0x25, 0x33},
                    api::BitcoinLikeFeePolicy::PER_BYTE,
                    10000,
                    "DarkNet Signed Message:\n",
                    false,
                    0,
                    {sigHashType::SIGHASH_ALL}
            );

            const std::vector<api::BitcoinLikeNetworkParameters> ALL
            ({
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