/*
 * AlgorandAccount
 *
 * Created by Rémi Barjon on 11/05/2020.
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

#pragma once

#include "AlgorandAssetAmount.hpp"
#include "transactions/AlgorandAssetParams.hpp"
#include "transactions/AlgorandKeyreg.hpp"

#include <utils/Option.hpp>

#include <cstdint>
#include <map>
#include <string>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    struct Account {
        // The round for which this account information is relevant
        uint64_t round;
        // The account address
        std::string address;
        // The total number of MicroAlgos in the account
        uint64_t amount;
        // The amount of MicroAlgos in the account, without the pending rewards
        uint64_t amountWithoutPendingRewards;
        // The amount of MicroAlgos of pending rewards in this account
        uint64_t pendingRewards;
        // The total rewards of MicroAlgos the account has received, including pending rewards
        uint64_t rewards;
        // The amounts of assets this account contains, indexed by asset ID
        std::map<uint64_t, AssetAmount> assetsAmounts;
        // The parameters of assets created by this account, indexed by asset ID
        std::map<uint64_t, AssetParams> createdAssets;
        // Indicates the status of the account (offline|online)
        std::string status;
        // Information about the participation of this account to the blockchain consensus
        Option<KeyRegTxnFields> participation;
    };

} // namespace model
} // namespace ledger
} // namespace core
} // namespace algorand
