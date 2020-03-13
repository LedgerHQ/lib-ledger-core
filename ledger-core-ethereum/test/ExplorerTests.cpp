/*
 *
 * ExplorerTests
 *
 * ledger-core-ethereum
 *
 * Created by Alexis Le Provost on 04/02/2020.
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

#include <gtest/gtest.h>

#include <ethereum/EthereumNetworks.hpp>
#include <ethereum/api/EthereumLikeNetworkParameters.hpp>
#include <ethereum/api/EthereumGasLimitRequest.hpp>
#include <ethereum/explorers/LedgerApiEthereumLikeBlockchainExplorer.hpp>

#include <integration/ExplorerFixture.hpp>

class EthereumExplorers : public ExplorerFixture<LedgerApiEthereumLikeBlockchainExplorer, api::EthereumLikeNetworkParameters> {
public:
    EthereumExplorers() {
        params = networks::getEthereumLikeNetworkParameters("ethereum_ropsten");
        explorerEndpoint = "http://eth-ropsten.explorers.dev.aws.ledger.fr";
    }
};

TEST_F(EthereumExplorers, GetGasPrice) {
    auto result = wait(explorer->getGasPrice());
    EXPECT_NE(result->toUint64(), 0);
}

TEST_F(EthereumExplorers, GetEstimatedGasLimit) {
    auto result = wait(explorer->getEstimatedGasLimit("0x57e8ba2a915285f984988282ab9346c1336a4e11"));
    EXPECT_GE(result->toUint64(), 10000);
}

TEST_F(EthereumExplorers, PostEstimatedGasLimit) {
  auto request = api::EthereumGasLimitRequest(
      optional<std::string>(), optional<std::string>(), optional<std::string>(),
      "0xa9059cbb00000000000000000000000013C5d95f25688f8A"
      "7544582D9e311f201A56de6300000000000000000000000000"
      "00000000000000000000000000000000000000",
      optional<std::string>(), optional<std::string>(), optional<double>(1.5));
  auto result = wait(explorer->getDryRunGasLimit(
      "0x57e8ba2a915285f984988282ab9346c1336a4e11", request));
  EXPECT_GE(result->toUint64(), 10000);
}