/*
 * AlgorandAssetParams
 *
 * Created by Rémi Barjon on 12/05/2020.
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

#include <algorand/AlgorandAddress.hpp>

#include <core/utils/Option.hpp>

#include <cstdint>
#include <string>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    class AssetParams
    {
    public:
        AssetParams() {}
        AssetParams(Option<std::vector<uint8_t>> metaDataHash,
                    Option<std::string> assetName,
                    Option<std::string> url,
                    Option<Address> clawbackAddr,
                    Option<uint32_t> decimals,
                    Option<bool> defaultFrozen,
                    Option<Address> freezeAddr,
                    Option<Address> managerAddr,
                    Option<Address> reserveAddr,
                    Option<uint64_t> total,
                    Option<std::string> unitName)
            : metaDataHash(std::move(metaDataHash))
            , assetName(std::move(assetName))
            , url(std::move(url))
            , clawbackAddr(std::move(clawbackAddr))
            , decimals(std::move(decimals))
            , defaultFrozen(std::move(defaultFrozen))
            , freezeAddr(std::move(freezeAddr))
            , managerAddr(std::move(managerAddr))
            , reserveAddr(std::move(reserveAddr))
            , total(std::move(total))
            , unitName(std::move(unitName))
        {}

        Option<std::vector<uint8_t>> metaDataHash;
        Option<std::string> assetName;
        Option<std::string> url;
        Option<Address> clawbackAddr;
        Option<uint32_t> decimals;
        Option<bool> defaultFrozen;
        Option<Address> freezeAddr;
        Option<Address> managerAddr;
        Option<Address> reserveAddr;
        Option<uint64_t> total;
        Option<std::string> unitName;

        // Additional fields retrieved from the blockchain
        Option<Address> creatorAddr;
    };

} // namespace model
} // namespace ledger
} // namespace core
} // namespace algorand

