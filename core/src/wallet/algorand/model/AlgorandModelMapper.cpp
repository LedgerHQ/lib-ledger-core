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

#include "AlgorandModelMapper.hpp"

#include "../AlgorandAddress.hpp"

#include <utils/Option.hpp>

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace ledger {
namespace core {

    template<typename T>
    Option<T> makeOption(const std::experimental::optional<T>& t)
    {
        return Option<T>(t);
    }

namespace algorand {
namespace model {

    namespace {

        Address addressFrom(const std::string& address)
        {
            return Address(address);
        }

        const std::string& addressTo(const Address& address)
        {
            return address.toString();
        }

        uint32_t u32From(const std::string& i)
        {
            return std::stoul(i);
        }

        std::string u32To(uint32_t i)
        {
            return std::to_string(i);
        }

        uint64_t u64From(const std::string& i)
        {
            return std::stoull(i);
        }

        std::string u64To(uint64_t i)
        {
            return std::to_string(i);
        }

        std::vector<uint8_t> toBinary(const std::string& binary)
        {
            return { std::begin(binary), std::end(binary) };
        }

        std::string fromBinary(const std::vector<uint8_t>& binary)
        {
            return { std::begin(binary), std::end(binary) };
        }

        template<typename T, typename UnaryOperation>
        auto mapOption(const Option<T>& option, UnaryOperation uop)
        {
            // FIXME(remibarjon): use std::invoke_result_t (C++17)
            // using U = std::invoke_result_t<UnaryOperation, T>;
            using U = std::result_of_t<UnaryOperation(T)>;

            return option.template map<std::decay_t<U>>(uop);
        }

        template<typename TModel>
        auto toOptionAPI(const Option<TModel>& model)
        {
            return mapOption(
                    model,
                    [](const TModel& model) {
                        return toAPI(model);
                    });
        }

        template<typename TApi>
        auto fromOptionAPI(const Option<TApi>& api)
        {
            return mapOption(
                    api,
                    [](const TApi& api) {
                        return fromAPI(api);
                    });
        }

    } // namespace

    api::AlgorandAssetAmount toAPI(const AssetAmount& amount)
    {
        return api::AlgorandAssetAmount(
                addressTo(amount.creatorAddress),
                u64To(amount.amount),
                amount.frozen,
                u64To(amount.assetId)
        );
    }

    AssetAmount fromAPI(const api::AlgorandAssetAmount& amount)
    {
        return AssetAmount(
                addressFrom(amount.creatorAddress),
                u64From(amount.amount),
                amount.frozen,
                u64From(amount.assetId)
        );
    }

    api::AlgorandAssetParams toAPI(const AssetParams& params)
    {
        return api::AlgorandAssetParams(
                mapOption(params.creatorAddr, addressTo).toOptional(),
                params.assetName.toOptional(),
                params.unitName.toOptional(),
                params.url.toOptional(),
                params.defaultFrozen.getValueOr(false),
                mapOption(params.total, u64To).toOptional(),
                mapOption(params.decimals, u32To).toOptional(),
                mapOption(params.creatorAddr, addressTo).toOptional(),
                mapOption(params.managerAddr, addressTo).toOptional(),
                mapOption(params.freezeAddr, addressTo).toOptional(),
                mapOption(params.clawbackAddr, addressTo).toOptional(),
                mapOption(params.reserveAddr, addressTo).toOptional(),
                mapOption(params.metaDataHash, fromBinary).toOptional()
        );
    }

    AssetParams fromAPI(const api::AlgorandAssetParams& params)
    {
        return [params]() {
            auto assetParams = AssetParams(
                    mapOption(makeOption(params.metadataHash), toBinary),
                    makeOption(params.assetName),
                    makeOption(params.url),
                    mapOption(makeOption(params.clawbackAddress), addressFrom),
                    mapOption(makeOption(params.decimals), u32From),
                    params.defaultFrozen,
                    mapOption(makeOption(params.freezeAddress), addressFrom),
                    mapOption(makeOption(params.managerAddress), addressFrom),
                    mapOption(makeOption(params.reserveAddress), addressFrom),
                    mapOption(makeOption(params.total), u64From),
                    makeOption(params.unitName)
            );

            assetParams.creatorAddr =
                    mapOption(makeOption(params.creatorAddress), addressFrom);

            return assetParams;
        }();
    }

    api::AlgorandAssetConfigurationInfo toAPI(const AssetConfigTxnFields& fields)
    {
        return api::AlgorandAssetConfigurationInfo(
                mapOption(fields.assetId, u64To).toOptional(),
                toOptionAPI(fields.assetParams).toOptional()
        );
    }

    AssetConfigTxnFields fromAPI(const api::AlgorandAssetConfigurationInfo& info)
    {
        return AssetConfigTxnFields(
                fromOptionAPI(makeOption(info.assetParams)),
                mapOption(makeOption(info.assetId), u64From)
        );
    }

    api::AlgorandAssetFreezeInfo toAPI(const AssetFreezeTxnFields& fields)
    {
        return api::AlgorandAssetFreezeInfo(
                u64To(fields.assetId),
                addressTo(fields.frozenAddress),
                fields.assetFrozen
        );
    }

    AssetFreezeTxnFields fromAPI(const api::AlgorandAssetFreezeInfo& info)
    {
        return AssetFreezeTxnFields(
                info.frozen,
                addressFrom(info.frozenAddress),
                u64From(info.assetId)
        );
    }

    api::AlgorandAssetTransferInfo toAPI(const AssetTransferTxnFields& fields)
    {
        return api::AlgorandAssetTransferInfo(
                u64To(fields.assetId),
                mapOption(fields.assetAmount, u64To).toOptional(),
                addressTo(fields.assetReceiver),
                mapOption(fields.assetCloseTo, addressTo).toOptional(),
                mapOption(fields.assetSender, addressTo).toOptional(),
                mapOption(fields.closeAmount, u64To).toOptional()
        );
    }

    AssetTransferTxnFields fromAPI(const api::AlgorandAssetTransferInfo& info)
    {
        return [info]() {
            auto fields = AssetTransferTxnFields(
                    mapOption(makeOption(info.amount), u64From),
                    mapOption(makeOption(info.closeAddress), addressFrom),
                    addressFrom(info.recipientAddress),
                    mapOption(makeOption(info.clawedBackAddress), addressFrom),
                    u64From(info.assetId)
            );

            fields.closeAmount =
                mapOption(makeOption(info.closeAmount), u64From);

            return fields;
        }();
    }

    api::AlgorandParticipationInfo toAPI(const KeyRegTxnFields& fields)
    {
        return api::AlgorandParticipationInfo(
                fields.votePk,
                fields.selectionPk,
                u64To(fields.voteKeyDilution),
                u64To(fields.voteFirst),
                u64To(fields.voteLast)
        );
    }

    KeyRegTxnFields fromAPI(const api::AlgorandParticipationInfo& info)
    {
        return KeyRegTxnFields(
                {},
                info.vrfPublicKey,
                u64From(info.voteFirstRound),
                u64From(info.voteKeyDilution),
                info.rootPublicKey,
                u64From(info.voteLastRound)
        );
    }

    api::AlgorandPaymentInfo toAPI(const PaymentTxnFields& fields)
    {
        return api::AlgorandPaymentInfo(
                addressTo(fields.receiverAddr),
                u64To(fields.amount),
                mapOption(fields.closeAddr, addressTo).toOptional(),
                mapOption(fields.closeAmount, u64To).toOptional()
        );
    }

    PaymentTxnFields fromAPI(const api::AlgorandPaymentInfo& info)
    {
        return [info]() {
            auto fields = PaymentTxnFields(
                    u64From(info.amount),
                    mapOption(makeOption(info.closeAddress), addressFrom),
                    addressFrom(info.recipientAddress)
            );

            fields.closeAmount =
                mapOption(makeOption(info.closeAmount), u64From);

            return fields;
        }();
    }

} // namespace model
} // namespace algorand
} // namespace core
} // namespace ledger

