/*
 *
 * AlgorandNetworks
 *
 * Created by Hakim Aammar on 04/05/2020.
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

#include "AlgorandNetworks.hpp"

#include <utils/Exception.hpp>

namespace ledger {
namespace core {
namespace networks {

    const std::unordered_map<std::string, api::AlgorandNetworkParameters> ALGORAND_NETWORKS() {
        static const std::unordered_map<std::string, api::AlgorandNetworkParameters> ALL_ALGORAND({
            {"algorand", api::AlgorandNetworkParameters("mainnet-v1.0","wGHE2Pwdvd7S12BL5FaOP20EGYesN73ktiC1qzkkit8=")},
            {"algorand-testnet", api::AlgorandNetworkParameters("testnet-v1.0", "SGO1GKSzyE7IEPItTxCByw9x8FmnrCDexi9/cOUJOiI=")}
        });
        return ALL_ALGORAND;
    }

    const api::AlgorandNetworkParameters getAlgorandNetworkParameters(const std::string &networkName) {
        if (isAlgorandCurrency(networkName)) {
            return ALGORAND_NETWORKS().at(networkName);
        } else {
            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "No network parameters set for {}", networkName);
        }
    }

    const bool isAlgorandCurrency(const std::string &networkName) {
        auto algorand_networks = ALGORAND_NETWORKS();
        return algorand_networks.find(networkName) != algorand_networks.end();
    }

} // namespace networks
} // namespace core
} // namespace ledger
