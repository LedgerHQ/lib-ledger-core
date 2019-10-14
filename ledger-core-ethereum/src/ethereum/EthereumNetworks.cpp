/*
 *
 * rippleNetworks
 *
 * Created by Alexis Le Provost on 05/01/2019.
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

#include <core/utils/Exception.hpp>

#include <ethereum/EthereumNetworks.hpp>

namespace ledger {
    namespace core {
        namespace networks {
            //Reference for chainIDs: https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md

            const api::EthereumLikeNetworkParameters getEthereumLikeNetworkParameters(const std::string &networkName) {
                if (networkName == "ethereum") {
                    static const api::EthereumLikeNetworkParameters ETHEREUM(
                            "eth",
                            "Ethereum signed message:\n",
                            "1",
                            {0x04, 0x88, 0xB2, 0x1E},
                            {},
                            0
                    );
                    return ETHEREUM;
                } else if (networkName == "ethereum_ropsten") {
                    static const api::EthereumLikeNetworkParameters ETHEREUM_ROPSTEN(
                            "eth_ropsten",
                            "Ethereum signed message:\n",
                            "3",
                            {0x04, 0x35, 0x87, 0xCF},
                            {},
                            0
                    );
                    return ETHEREUM_ROPSTEN;
                } else if (networkName == "ethereum_classic") {
                    static const api::EthereumLikeNetworkParameters ETHEREUM_CLASSIC(
                            "etc",
                            "Ethereum signed message:\n",
                            "61",
                            {0x04, 0x88, 0xB2, 0x1E},
                            {},
                            0
                    );
                    return ETHEREUM_CLASSIC;
                }

                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "No network parameters set for {}", networkName);
            }

            const std::vector<api::EthereumLikeNetworkParameters> ALL_ETHEREUM
            ({
                     getEthereumLikeNetworkParameters("ethereum"),
                     getEthereumLikeNetworkParameters("ethereum_ropsten"),
                     getEthereumLikeNetworkParameters("ethereum_classic")
            });
        }
    }
}
