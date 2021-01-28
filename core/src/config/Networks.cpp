/*
 *
 * Networks
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2017.
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
#include "Networks.hpp"
#include <api/ErrorCode.hpp>
#include <utils/Exception.hpp>
namespace ledger {
    namespace core {
        namespace api {

            BitcoinLikeNetworkParameters Networks::bitcoin() {
                return BitcoinLikeNetworkParameters(
                    "bitcoin",
                    {0x00},
                    {0x05},
                    {0x04, 0x88, 0xB2, 0x1E},
                    BitcoinLikeFeePolicy::PER_BYTE,
                    5420,
                    "Bitcoin Signed Message:\n",
                    false,
                    0,
                    {0x01},
                    {}
                );
            }

            CosmosLikeNetworkParameters Networks::cosmos(const std::string& chainID) {
                    static const api::CosmosLikeNetworkParameters STARGATE_TESTNET_6(
                        // Stargate-6 testnet
                        "cosmos_testnet",
                        "MUON signed message:\n",
                        {0x04, 0x88, 0xB2, 0x1E},
                        {0xEB, 0x5A, 0xE9, 0x87},
                        {0x16, 0x24, 0xDE, 0x64},
                        {},
                        "stargate-6",
                        {}
                    );

                    static const api::CosmosLikeNetworkParameters STARGATE_TESTNET(
                        // Stargate testnet
                        "cosmos_testnet",
                        "MUON signed message:\n",
                        {0x04, 0x88, 0xB2, 0x1E},
                        {0xEB, 0x5A, 0xE9, 0x87},
                        {0x16, 0x24, 0xDE, 0x64},
                        {},
                        "stargate-5",
                        {}
                    );

                    static const api::CosmosLikeNetworkParameters COSMOSHUB_4(
                        // The current version of the chain has the "cosmos" identifer
                        // Stargate mainnet
                        "cosmos",
                        "ATOM signed message:\n",
                        {0x04, 0x88, 0xB2, 0x1E},
                        {0xEB, 0x5A, 0xE9, 0x87},
                        {0x16, 0x24, 0xDE, 0x64},
                        {},
                        "cosmoshub-4",
                        {}
                    );

                    static const api::CosmosLikeNetworkParameters COSMOSHUB_3(
                        "cosmos",
                        "ATOM signed message:\n",
                        {0x04, 0x88, 0xB2, 0x1E},
                        {0xEB, 0x5A, 0xE9, 0x87},
                        {0x16, 0x24, 0xDE, 0x64},
                        {},
                        "cosmoshub-3",
                        {}
                    );

                    static const api::CosmosLikeNetworkParameters COSMOSHUB_2(
                        "atom-cosmoshub2",
                        "ATOM signed message:\n",
                        {0x04, 0x88, 0xB2, 0x1E},
                        {0xEB, 0x5A, 0xE9, 0x87},
                        {0x16, 0x24, 0xDE, 0x64},
                        {},
                        "cosmoshub-2",
                        {}
                    );

                if (chainID == "cosmoshub-4") {
                    return COSMOSHUB_4;
                }
                if (chainID == "cosmoshub-3") {
                    return COSMOSHUB_3;
                }
                if (chainID == "cosmoshub-2") {
                    return COSMOSHUB_2;
                }
                if (chainID == "stargate-6") {
                    return STARGATE_TESTNET_6;
                }
                if (chainID == "stargate-5") {
                    return STARGATE_TESTNET;
                }
                throw make_exception(
                    api::ErrorCode::INVALID_ARGUMENT,
                    "No network parameters set for chain {}",
                    chainID);
            }


            EthereumLikeNetworkParameters Networks::ethereum() {
                return EthereumLikeNetworkParameters(
                        "eth",
                        "Ethereum signed message:\n",
                        {0x01},
                        {0x04, 0x88, 0xb2, 0x1e},
                        {},
                        0
                );
            }

            RippleLikeNetworkParameters Networks::ripple() {
                return RippleLikeNetworkParameters(
                        "xrp",
                        "XRP signed message:\n",
                        {0x04, 0x88, 0xB2, 0x1E},
                        {},
                        0
                );
            }

            TezosLikeNetworkParameters Networks::tezos() {
                return TezosLikeNetworkParameters(
                        "xtz",
                        "XTZ signed message:\n",
                        {0x04, 0x88, 0xB2, 0x1E},
                        {0x06, 0xA1, 0x9F},
                        {0x02, 0x5A, 0x79},
                        {},
                        0
                );
            }
        }
    }
}
