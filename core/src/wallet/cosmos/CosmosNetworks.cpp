/*
 *
 * cosmos/config/Networks
 * ledger-core
 *
 * Created by Gerry Agbobada on 21/01/2020.
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

#include <api/ErrorCode.hpp>
#include <config/Networks.hpp>
#include <utils/Exception.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>

namespace ledger {
namespace core {
namespace networks {
const api::CosmosLikeNetworkParameters getCosmosLikeNetworkParameters(const std::string &chainID)
{
    return api::Networks::cosmos(chainID);
}

const std::vector<api::CosmosLikeNetworkParameters> ALL_COSMOS(
    {getCosmosLikeNetworkParameters("atom-cosmoshub-4"),
     getCosmosLikeNetworkParameters("stargate"),
     getCosmosLikeNetworkParameters("atom"),
     getCosmosLikeNetworkParameters("atom-cosmoshub-2")});
}  // namespace networks
}  // namespace core
}  // namespace ledger
