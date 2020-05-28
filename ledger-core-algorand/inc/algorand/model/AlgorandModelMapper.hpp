/*
 * AlgorandModelMapper
 *
 * Created by Rémi Barjon on 22/05/2020.
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

#include <algorand/model/AlgorandAssetAmount.hpp>
#include <algorand/model/transactions/AlgorandAsset.hpp>
#include <algorand/model/transactions/AlgorandAssetParams.hpp>
#include <algorand/model/transactions/AlgorandKeyreg.hpp>
#include <algorand/model/transactions/AlgorandPayment.hpp>

#include <algorand/api/AlgorandAssetAmount.hpp>
#include <algorand/api/AlgorandAssetParams.hpp>
#include <algorand/api/AlgorandAssetConfigurationInfo.hpp>
#include <algorand/api/AlgorandAssetFreezeInfo.hpp>
#include <algorand/api/AlgorandAssetTransferInfo.hpp>
#include <algorand/api/AlgorandParticipationInfo.hpp>
#include <algorand/api/AlgorandPaymentInfo.hpp>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    api::AlgorandAssetAmount toAPI(const AssetAmount& amount);
    AssetAmount fromAPI(const api::AlgorandAssetAmount& amount);

    api::AlgorandAssetParams toAPI(const AssetParams& params);
    AssetParams fromAPI(const api::AlgorandAssetParams& params);

    api::AlgorandAssetConfigurationInfo toAPI(const AssetConfigTxnFields& fields);
    AssetConfigTxnFields fromAPI(const api::AlgorandAssetConfigurationInfo& info);

    api::AlgorandAssetFreezeInfo toAPI(const AssetFreezeTxnFields& fields);
    AssetFreezeTxnFields fromAPI(const api::AlgorandAssetFreezeInfo& info);

    api::AlgorandAssetTransferInfo toAPI(const AssetTransferTxnFields& fields);
    AssetTransferTxnFields fromAPI(const api::AlgorandAssetTransferInfo& info);

    api::AlgorandParticipationInfo toAPI(const KeyRegTxnFields& fields);
    KeyRegTxnFields fromAPI(const api::AlgorandParticipationInfo& info);

    api::AlgorandPaymentInfo toAPI(const PaymentTxnFields& fields);
    PaymentTxnFields fromAPI(const api::AlgorandPaymentInfo& info);

} // namespace model
} // namespace algorand
} // namespace core
} // namespace ledger

