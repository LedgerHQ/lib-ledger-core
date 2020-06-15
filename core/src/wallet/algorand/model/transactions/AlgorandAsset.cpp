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

#include "AlgorandAsset.hpp"

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    AssetConfigTxnFields AssetConfigTxnFields::create(AssetParams assetParams)
    {
        return AssetConfigTxnFields(assetParams, Option<uint64_t>());
    }

    AssetConfigTxnFields AssetConfigTxnFields::reconfigure(AssetParams assetParams, uint64_t assetId)
    {
        return AssetConfigTxnFields(assetParams, assetId);
    }

    AssetConfigTxnFields AssetConfigTxnFields::destroy(uint64_t assetId)
    {
        return AssetConfigTxnFields(Option<AssetParams>(), assetId);
    }

    AssetConfigTxnFields::AssetConfigTxnFields(
            Option<AssetParams> assetParams,
            Option<uint64_t> assetId)
        : assetParams(std::move(assetParams))
        , assetId(std::move(assetId))
    {}


    AssetTransferTxnFields AssetTransferTxnFields::transfer(
            uint64_t assetAmount,
            Option<Address> assetCloseTo,
            Address assetReceiver,
            uint64_t assetId)
    {
        return AssetTransferTxnFields(
                assetAmount,
                std::move(assetCloseTo),
                std::move(assetReceiver),
                Option<Address>(),
                assetId);
    }

    AssetTransferTxnFields AssetTransferTxnFields::clawback(
            uint64_t assetAmount,
            Option<Address> assetCloseTo,
            Address assetReceiver,
            Address assetSender,
            uint64_t assetId)
    {
        return AssetTransferTxnFields(
                assetAmount,
                std::move(assetCloseTo),
                std::move(assetReceiver),
                std::move(assetSender),
                assetId);
    }

    AssetTransferTxnFields AssetTransferTxnFields::optIn(
            Address assetReceiver,
            uint64_t assetId)
    {
        return AssetTransferTxnFields(
                Option<uint64_t>(),
                Option<Address>(),
                std::move(assetReceiver),
                Option<Address>(),
                assetId);
    }

    AssetTransferTxnFields::AssetTransferTxnFields(
            Option<uint64_t> assetAmount,
            Option<Address> assetCloseTo,
            Address assetReceiver,
            Option<Address> assetSender,
            uint64_t assetId)
        : assetAmount(std::move(assetAmount))
        , assetCloseTo(std::move(assetCloseTo))
        , assetReceiver(std::move(assetReceiver))
        , assetSender(std::move(assetSender))
        , assetId(assetId)
    {}

    AssetFreezeTxnFields::AssetFreezeTxnFields(
            bool assetFrozen,
            Address frozenAddress,
            uint64_t assetId)
        : assetFrozen(assetFrozen)
        , frozenAddress(std::move(frozenAddress))
        , assetId(assetId)
    {}

} // namespace model
} // namespace algorand
} // namespace core
} // namespace ledger

