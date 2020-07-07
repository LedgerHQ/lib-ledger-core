/*
 * AlgorandJsonParser
 *
 * Created by Hakim Aammar on 11/05/2020.
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

#ifndef LEDGER_CORE_ALGORANDJSONPARSER_H
#define LEDGER_CORE_ALGORANDJSONPARSER_H

#include <algorand/AlgorandAddress.hpp>
#include <algorand/AlgorandExplorerConstants.hpp>
#include <algorand/model/transactions/AlgorandTransaction.hpp>
#include <algorand/model/transactions/AlgorandAssetParams.hpp>
#include <algorand/model/transactions/AlgorandTransactionParams.hpp>
#include <algorand/model/AlgorandAccount.hpp>
#include <algorand/model/AlgorandAssetAmount.hpp>

#include <core/math/BigInt.hpp>
#include <core/math/BaseConverter.hpp>
#include <core/utils/Exception.hpp>
#include <core/utils/Hex.hpp>
#include <core/api/Block.hpp>
#include <core/api/ErrorCode.hpp>

#include <rapidjson/document.h>

#include <limits>

namespace ledger {
namespace core {
namespace algorand {

    class JsonParser {

    public:

        template <class T>
        static void parseBlock(const T& node, api::Block & block) {
            assertWithMessage(node, constants::xBlock);
            const auto& jsonBlock = node[constants::xBlock.c_str()].GetObject();

            block.time =
                std::chrono::system_clock::time_point(
                        std::chrono::seconds(getMandatoryUint64Field(jsonBlock, constants::xTs)));

            const auto blockHeight = getMandatoryUint64Field(jsonBlock, constants::xRnd);
            if (blockHeight > std::numeric_limits<int64_t>::max()) {
                throw make_exception(api::ErrorCode::OUT_OF_RANGE, "Block height exceeds maximum value");
            }
            block.height = static_cast<int64_t>(blockHeight);

            /// In Algorand implementation, we use the block height (aka round) as block hash
            block.blockHash = std::to_string(block.height);
        }

        template <class T>
        static void parseAssetAmount(const T& node, model::AssetAmount & assetAmount) {
            assetAmount.creatorAddress = getMandatoryAddressField(node, constants::xCreator);
            assetAmount.amount = getMandatoryUint64Field(node, constants::xAmount);
            assetAmount.frozen = getMandatoryBoolField(node, constants::xFrozen);
            assetAmount.assetId = getMandatoryUint64Field(node, constants::xAssetId);
        }

        template <class T>
        static void parseAssetsAmounts(const T& node, std::map<uint64_t, model::AssetAmount> & assetsAmounts) {
            for (const auto& child : node) {
                model::AssetAmount assetAmount;
                parseAssetAmount(child, assetAmount);
                assetsAmounts[assetAmount.assetId] = assetAmount;
            }
        }

        template <typename T>
        static void parseAssetParams(const T& node, model::AssetParams & assetParams) {
            assetParams.creatorAddr = getMandatoryAddressField(node, constants::xCreator);
            assetParams.total = getMandatoryUint64Field(node, constants::xTotal);
            assert(node.HasMember(constants::xDecimal.c_str()));
            assetParams.decimals = node[constants::xDecimal.c_str()].GetUint();

            assetParams.defaultFrozen = getOptionalBoolField(node, constants::xDefaultFrozen);
            assetParams.unitName = getOptionalStringField(node, constants::xUnitName);
            assetParams.assetName = getOptionalStringField(node, constants::xAssetName);
            assetParams.managerAddr = getOptionalAddressField(node, constants::xManagerKey);
            assetParams.freezeAddr = getOptionalAddressField(node, constants::xFreezeAddr);
            assetParams.clawbackAddr = getOptionalAddressField(node, constants::xClawbackAddr);
            assetParams.reserveAddr = getOptionalAddressField(node, constants::xReserveAddr);
            assetParams.metaDataHash = getOptionalBinaryField(node, constants::xMetadataHash);
            assetParams.url = getOptionalStringField(node, constants::xUrl);
        }

        template <class T>
        static void parseAssetsParams(const T& node, std::map<uint64_t, model::AssetParams> & assetsParams) {
            for (const auto& child : node) {
                model::AssetParams assetParams;
                const auto id = getMandatoryUint64Field(child, constants::xIndex);
                parseAssetParams(child[constants::xParams.c_str()].GetObject(), assetParams);
                assetsParams[id] = assetParams;
            }
        }

        template <class T>
        static void parsePaymentInfo(const T& node, model::PaymentTxnFields & details) {
            details.receiverAddr = getMandatoryAddressField(node, constants::xReceiver);
            details.amount = getMandatoryUint64Field(node, constants::xAmount);
            details.closeAddr = getOptionalAddressField(node, constants::xCloseRemainderTo);
            details.closeAmount = getOptionalUint64Field(node, constants::xCloseAmount);
        }

        // WARNING This has not been tested
        template <class T>
        static void parseParticipationInfo(const T& node, model::KeyRegTxnFields & details) {
            details.selectionPk = getMandatoryStringField(node, constants::xVotekey );
            details.votePk = getMandatoryStringField(node, constants::xSelkey);
            details.voteKeyDilution = getMandatoryUint64Field(node, constants::xVotekd);
            details.voteFirst = getMandatoryUint64Field(node, constants::xVotefst);
            details.voteLast = getMandatoryUint64Field(node, constants::xVotelst);

            details.nonParticipation = getOptionalBoolField(node, constants::xNonParticipation);
        }

        template <class T>
        static void parseAssetConfigurationInfo(const T& node, model::AssetConfigTxnFields & details) {
            details.assetId = getMandatoryUint64Field(node, constants::xAssetId);

            assert(node.HasMember(constants::xParams.c_str()));
            details.assetParams = model::AssetParams();
            parseAssetParams(node[constants::xParams.c_str()].GetObject(), *details.assetParams);
        }

        template <class T>
        static void parseAssetTransferInfo(const T& node, model::AssetTransferTxnFields & details) {
            details.assetId = getMandatoryUint64Field(node, constants::xAssetId);
            details.assetReceiver = getMandatoryAddressField(node, constants::xReceiver);
            details.assetAmount = getMandatoryUint64Field(node, constants::xAmount);

            details.assetCloseTo = getOptionalAddressField(node, constants::xCloseTo);
            details.closeAmount = getOptionalUint64Field(node, constants::xCloseAmount);
            details.assetSender = getOptionalAddressField(node, constants::xSender);
        }

        // WARNING This has not been tested
        template <class T>
        static void parseAssetFreezeInfo(const T& node, model::AssetFreezeTxnFields & details) {
            details.assetId = getMandatoryUint64Field(node, constants::xAssetId);
            details.frozenAddress = getMandatoryAddressField(node, constants::xAddress);
            details.assetFrozen = getMandatoryBoolField(node, constants::xNewFreezeStatus);
        }

        template <class T>
        static void parseTransaction(const T& node, model::Transaction & tx) {

            tx.header.type = getMandatoryStringField(node, constants::xTxType);
            tx.header.id = getMandatoryStringField(node, constants::xId);
            tx.header.sender = getMandatoryAddressField(node, constants::xSender);
            tx.header.firstValid = getMandatoryUint64Field(node, constants::xFirstValid);
            tx.header.lastValid = getMandatoryUint64Field(node, constants::xLastValid);
            tx.header.fee = getMandatoryUint64Field(node, constants::xFee);
            tx.header.genesisHash = getMandatoryB64StringField(node, constants::xGenesisHash);

            tx.header.timestamp = getOptionalUint64Field(node, constants::xRoundTime);
            tx.header.round = getOptionalUint64Field(node, constants::xConfirmedRound);
            tx.header.genesisId = getOptionalStringField(node, constants::xGenesisId);
            tx.header.note = getOptionalBinaryField(node, constants::xNote);
            tx.header.group = getOptionalBinaryField(node, constants::xGroup);
            tx.header.lease = getOptionalBinaryField(node, constants::xLease);
            tx.header.senderRewards = getOptionalUint64Field(node, constants::xSenderRewards);
            tx.header.receiverRewards = getOptionalUint64Field(node, constants::xReceiverRewards);
            tx.header.closeRewards = getOptionalUint64Field(node, constants::xCloseRewards);

            if (tx.header.type == constants::xPay) {
                    assert((node.HasMember(constants::xPayment.c_str())));
                    auto details = model::PaymentTxnFields();
                    parsePaymentInfo(node[constants::xPayment.c_str()].GetObject(), details);
                    tx.details = details;
            } else if (tx.header.type == constants::xKeyreg) {
                    assert((node.HasMember(constants::xKeyregs.c_str())));
                    auto details = model::KeyRegTxnFields();
                    parseParticipationInfo(node[constants::xKeyregs.c_str()].GetObject(), details);
                    tx.details = details;
            } else if (tx.header.type == constants::xAcfg) {
                    assert((node.HasMember(constants::xCurcfg.c_str())));
                    auto details = model::AssetConfigTxnFields();
                    parseAssetConfigurationInfo(node[constants::xCurcfg.c_str()].GetObject(), details);
                    tx.details = details;
            } else if (tx.header.type == constants::xAxfer) {
                    assert((node.HasMember(constants::xCurxfer.c_str())));
                    auto details = model::AssetTransferTxnFields();
                    parseAssetTransferInfo(node[constants::xCurxfer.c_str()].GetObject(), details);
                    tx.details = details;
            } else if (tx.header.type == constants::xAfreeze) {
                    assert((node.HasMember(constants::xCurfrz.c_str())));
                    auto details = model::AssetFreezeTxnFields();
                    parseAssetFreezeInfo(node[constants::xCurfrz.c_str()].GetObject(), details);
                    tx.details = details;
            }
        }

        template <class T>
        static void parseTransactions(const T& array, std::vector<model::Transaction> & txs) {
            txs.assign((std::size_t) array.Size(), model::Transaction());
            auto index = 0;
            for (const auto& node : array) {
                parseTransaction(node.GetObject(), txs[index++]);
            }
        }

        template <class T>
        static void parseTransactionParams(const T& node, model::TransactionParams & txParams) {
            txParams.genesisID = getMandatoryStringField(node, constants::xGenesisId);
            txParams.genesisHash = getMandatoryStringField(node, constants::xGenesisHash);
            txParams.lastRound = getMandatoryUint64Field(node, constants::xLastRoundParam);
            txParams.suggestedFeePerByte = getMandatoryUint64Field(node, constants::xFee);
            txParams.minFee = getMandatoryUint64Field(node, constants::xMinFee);
            txParams.consensusVersion = getMandatoryStringField(node, constants::xConsensusVersion);
        }

        template <class T>
        static void parseAccount(const T& node, model::Account & account) {
            account.address = getMandatoryStringField(node, constants::xAddress);
            account.amount = getMandatoryUint64Field(node, constants::xAmount);
            account.amountWithoutPendingRewards = getMandatoryUint64Field(node, constants::xAmountWithoutPendingRewards);

            if (node.HasMember(constants::xAssets.c_str())) {
                parseAssetsAmounts(node[constants::xAssets.c_str()].GetArray(), account.assetsAmounts);
            }

            if (node.HasMember(constants::xParticipation.c_str())) {
                account.participation = model::KeyRegTxnFields();
                parseParticipationInfo(node[constants::xParticipation.c_str()].GetObject(), *account.participation);
            }

            account.pendingRewards = getMandatoryUint64Field(node, constants::xPendingRewards);
            account.rewards = getMandatoryUint64Field(node, constants::xRewards);
            account.round = getMandatoryUint64Field(node, constants::xRound);
            account.status = getOptionalStringField(node, constants::xStatus);

            if (node.HasMember(constants::xCreatedAssets.c_str())) {
                parseAssetsParams(node[constants::xCreatedAssets.c_str()].GetArray(), account.createdAssets);
            }
        }

    private:

        template <class T>
        static void assertWithMessage(const T & node, const std::string & fieldName) {
            if (!node.HasMember(fieldName.c_str())) {
                throw make_exception(api::ErrorCode::NO_SUCH_ELEMENT, fmt::format("Missing '{}' field in JSON.", fieldName));
            }
        }

        template<class T>
        static std::string getMandatoryStringField(const T& node, const std::string& fieldName)
        {
            assertWithMessage(node, fieldName);
            return node[fieldName.c_str()].GetString();
        }

        template<class T>
        static Option<std::string> getOptionalStringField(const T& node, const std::string& fieldName)
        {
            if (node.HasMember(fieldName.c_str())) {
                return getMandatoryStringField(node, fieldName);
            }
            return {};
        }

        template<class T>
        static uint64_t getMandatoryUint64Field(const T& node, const std::string& fieldName)
        {
            assertWithMessage(node, fieldName);
            return node[fieldName.c_str()].GetUint64();
        }

        template<class T>
        static Option<uint64_t> getOptionalUint64Field(const T& node, const std::string& fieldName)
        {
            if (node.HasMember(fieldName.c_str())) {
                return getMandatoryUint64Field(node, fieldName);
            }
            return {};
        }

        template<class T>
        static bool getMandatoryBoolField(const T& node, const std::string& fieldName)
        {
            assertWithMessage(node, fieldName);
            return node[fieldName.c_str()].GetBool();
        }

        template<class T>
        static Option<bool> getOptionalBoolField(const T& node, const std::string& fieldName)
        {
            if (node.HasMember(fieldName.c_str())) {
                return getMandatoryBoolField(node, fieldName);
            }
            return {};
        }

        template<class T>
        static Address getMandatoryAddressField(const T& node, const std::string& fieldName)
        {
            assertWithMessage(node, fieldName);
            return Address(getMandatoryStringField(node, fieldName));
        }

        template<class T>
        static Option<Address> getOptionalAddressField(const T& node, const std::string& fieldName)
        {
            if (node.HasMember(fieldName.c_str())) {
                return getMandatoryAddressField(node, fieldName);
            }
            return {};
        }

        template<class T>
        static B64String getMandatoryB64StringField(const T& node, const std::string& fieldName)
        {
            assertWithMessage(node, fieldName);
            return B64String(getMandatoryStringField(node, fieldName));
        }

        template<class T>
        static Option<B64String> getOptionalB64StringField(const T& node, const std::string& fieldName)
        {
            if (node.HasMember(fieldName.c_str())) {
                return getMandatoryB64StringField(node, fieldName);
            }
            return {};
        }

        template<class T>
        static std::vector<uint8_t> getMandatoryBinaryField(const T& node, const std::string& fieldName)
        {
            assertWithMessage(node, fieldName);
            std::vector<uint8_t> newBinary;
            BaseConverter::decode(node[fieldName.c_str()].GetString(), BaseConverter::BASE64_RFC4648, newBinary);
            return newBinary;
        }

        template<class T>
        static Option<std::vector<uint8_t>> getOptionalBinaryField(const T& node, const std::string& fieldName)
        {
            if (node.HasMember(fieldName.c_str())) {
                return getMandatoryBinaryField(node, fieldName);
            }
            return {};
        }

    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDJSONPARSER_H
