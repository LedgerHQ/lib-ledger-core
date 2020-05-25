/*
 * AlgorandAsset
 *
 * Created by Rémi Barjon on 04/05/2020.
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

#include "../../AlgorandAddress.hpp"
#include "AlgorandAssetParams.hpp"

#include <core/utils/Option.hpp>

#include <cstdint>
#include <string>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    class AssetConfigTxnFields
    {
    public:
        static AssetConfigTxnFields create(AssetParams assetParams);
        static AssetConfigTxnFields reconfigure(AssetParams assetParams, uint64_t assetId);
        static AssetConfigTxnFields destroy(uint64_t assetId);

        AssetConfigTxnFields() = default;

    private:
        AssetConfigTxnFields(Option<AssetParams> assetParams, Option<uint64_t> assetId);

    public:
        Option<AssetParams> assetParams;
        Option<uint64_t> assetId;
    };

    class AssetTransferTxnFields
    {
    public:
        static AssetTransferTxnFields transfer(uint64_t assetAmount,
                                               Option<Address> assetCloseTo,
                                               Address assetReceiver,
                                               uint64_t assetId);

        static AssetTransferTxnFields clawback(uint64_t assetAmount,
                                               Option<Address> assetCloseTo,
                                               Address assetReceiver,
                                               Address assetSender,
                                               uint64_t assetId);

        static AssetTransferTxnFields optIn(Address assetReceiver,
                                            uint64_t assetId);

        AssetTransferTxnFields() = default;

    private:
        AssetTransferTxnFields(Option<uint64_t> assetAmount,
                               Option<Address> assetCloseTo,
                               Address assetReceiver,
                               Option<Address> assetSender,
                               uint64_t assetId);
    public:
        Option<uint64_t> assetAmount;
        Option<Address> assetCloseTo;
        Address assetReceiver;
        Option<Address> assetSender;
        uint64_t assetId;
    };

    class AssetFreezeTxnFields
    {
    public:
        AssetFreezeTxnFields() = default;
        AssetFreezeTxnFields(bool assetFrozen,
                             Address frozenAddress,
                             uint64_t assetId);

        bool assetFrozen;
        Address frozenAddress;
        uint64_t assetId;
    };

} // namespace model
} // namespace ledger
} // namespace core
} // namespace algorand

