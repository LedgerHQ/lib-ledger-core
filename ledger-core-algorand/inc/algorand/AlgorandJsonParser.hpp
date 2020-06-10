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

namespace ledger {
namespace core {
namespace algorand {

    class JsonParser {

    public:

        template <class T>
        static void parseBlock(const T& node, api::Block & block) {
            getMandatoryStringField(node, constants::xHash, block.blockHash);

            block.time = std::chrono::system_clock::time_point(std::chrono::seconds(node[constants::xTimestamp.c_str()].GetUint64()));

            uint64_t blockHeight = 0;
            getMandatoryUint64Field(node, constants::xRound, blockHeight);
            if (blockHeight > std::numeric_limits<int64_t>::max()) {
                throw make_exception(api::ErrorCode::OUT_OF_RANGE, "Block height exceeds maximum value");
            }
            block.height = static_cast<int64_t>(blockHeight);
        }

        template <class T>
        static void parseAssetAmount(const T& node, model::AssetAmount & assetAmount) {
            getMandatoryAddressField(node, constants::xCreator, assetAmount.creatorAddress);
            getMandatoryUint64Field(node, constants::xAmount, assetAmount.amount);
            getMandatoryBoolField(node, constants::xFrozen, assetAmount.frozen);
        }

        template <class T>
        static void parseAssetsAmounts(const T& node, std::map<uint64_t, model::AssetAmount> & assetsAmounts) {
            for (rapidjson::Value::ConstMemberIterator child = node.MemberBegin(); child != node.MemberEnd(); ++child) {
                model::AssetAmount assetAmount;
                const auto assetId = std::stoull(child->name.GetString());
                parseAssetAmount(child->value, assetAmount);
                assetsAmounts[assetId] = assetAmount;
            }
        }

        template <typename T>
        static void parseAssetParams(const T& node, model::AssetParams & assetParams) {

            getMandatoryAddressField(node, constants::xCreator, assetParams.creatorAddr);
            getMandatoryUint64Field(node, constants::xTotal, assetParams.total);
            assert(node.HasMember(constants::xDecimal.c_str()));
            assetParams.decimals = node[constants::xDecimal.c_str()].GetUint();
            getMandatoryBoolField(node, constants::xDefaultFrozen, assetParams.defaultFrozen);
            getMandatoryStringField(node, constants::xUnitName, assetParams.unitName);
            getMandatoryStringField(node, constants::xAssetName, assetParams.assetName);

            getOptionalAddressField(node, constants::xManagerKey, assetParams.managerAddr);
            getOptionalAddressField(node, constants::xFreezeAddr, assetParams.freezeAddr);
            getOptionalAddressField(node, constants::xClawbackAddr, assetParams.clawbackAddr);
            getOptionalAddressField(node, constants::xReserveAddr, assetParams.reserveAddr);
            getOptionalBinaryField(node, constants::xMetadataHash, assetParams.metaDataHash);
            getOptionalStringField(node, constants::xUrl, assetParams.url);
        }

        template <class T>
        static void parseAssetsParams(const T& node, std::map<uint64_t, model::AssetParams> & assetsParams) {
            for (rapidjson::Value::ConstMemberIterator child = node.MemberBegin(); child != node.MemberEnd(); ++child) {
                model::AssetParams assetParams;
                const auto assetId = std::stoull(child->name.GetString());
                parseAssetParams(child->value, assetParams);
                assetsParams[assetId] = assetParams;
            }
        }

        template <class T>
        static void parsePaymentInfo(const T& node, model::PaymentTxnFields & details) {
            getMandatoryAddressField(node, constants::xTo, details.receiverAddr);
            getMandatoryUint64Field(node, constants::xAmount, details.amount);

            getOptionalAddressField(node, constants::xClose, details.closeAddr);
            getOptionalUint64Field(node, constants::xCloseAmount, details.closeAmount);
            getOptionalUint64Field(node, constants::xCloseRewards, details.closeRewards);
            getOptionalUint64Field(node, constants::xToRewards, details.receiverRewards);
        }

        // WARNING This has not been tested
        template <class T>
        static void parseParticipationInfo(const T& node, model::KeyRegTxnFields & details) {
            getMandatoryStringField(node, constants::xVotekey , details.selectionPk);
            getMandatoryStringField(node, constants::xSelkey, details.votePk);
            getMandatoryUint64Field(node, constants::xVotekd, details.voteKeyDilution);
            getMandatoryUint64Field(node, constants::xVotefst, details.voteFirst);
            getMandatoryUint64Field(node, constants::xVotelst, details.voteLast);
        }

        template <class T>
        static void parseAssetConfigurationInfo(const T& node, model::AssetConfigTxnFields & details) {
            getMandatoryUint64Field(node, constants::xId, details.assetId);

            assert(node.HasMember(constants::xParams.c_str()));
            details.assetParams = model::AssetParams();
            parseAssetParams(node[constants::xParams.c_str()].GetObject(), *details.assetParams);
        }

        template <class T>
        static void parseAssetTransferInfo(const T& node, model::AssetTransferTxnFields & details) {
            getMandatoryUint64Field(node, constants::xId, details.assetId);
            getMandatoryAddressField(node, constants::xRcv, details.assetReceiver);
            getMandatoryUint64Field(node, constants::xAmt, details.assetAmount);

            getOptionalAddressField(node, constants::xCloseTo, details.assetCloseTo);
            getOptionalAddressField(node, constants::xSnd, details.assetSender);
        }

        // WARNING This has not been tested
        template <class T>
        static void parseAssetFreezeInfo(const T& node, model::AssetFreezeTxnFields & details) {
            getMandatoryUint64Field(node, constants::xId, details.assetId);
            getMandatoryAddressField(node, constants::xAcct, details.frozenAddress);
            getMandatoryBoolField(node, constants::xFreeze, details.assetFrozen);
        }

        template <class T>
        static void parseTransaction(const T& node, model::Transaction & tx) {

            getMandatoryStringField(node, constants::xType, tx.header.type);
            getMandatoryStringField(node, constants::xTx, tx.header.id);
            getMandatoryAddressField(node, constants::xFrom, tx.header.sender);
            //getMandatoryUint64Field(node, constants::xTimestamp, tx.header.timestamp); // FIXME Restore this when PureStake provides it
            getMandatoryUint64Field(node, constants::xFirstRound, tx.header.firstValid);
            getMandatoryUint64Field(node, constants::xLastRound, tx.header.lastValid);
            getMandatoryUint64Field(node, constants::xRound, tx.header.round);
            getMandatoryUint64Field(node, constants::xFee, tx.header.fee);
            getMandatoryB64StringField(node, constants::xGenesisHashB64, tx.header.genesisHash);

            getOptionalStringField(node, constants::xGenesisId, tx.header.genesisId);
            getOptionalBinaryField(node, constants::xNoteB64, tx.header.note);
            getOptionalUint64Field(node, constants::xFromRewards, tx.header.fromRewards);
            getOptionalBinaryField(node, constants::xGroup, tx.header.group);
            getOptionalBinaryField(node, constants::xLease, tx.header.lease);

            if (tx.header.type == constants::xPay) {
                    assert((node.HasMember(constants::xPayment.c_str())));
                    tx.details = model::PaymentTxnFields();
                    parsePaymentInfo(node[constants::xPayment.c_str()].GetObject(),
                                        boost::get<model::PaymentTxnFields>(tx.details));
            } else if (tx.header.type == constants::xKeyreg) {
                    assert((node.HasMember(constants::xKeyregs.c_str())));
                    tx.details = model::KeyRegTxnFields();
                    parseParticipationInfo(node[constants::xKeyregs.c_str()].GetObject(),
                                        boost::get<model::KeyRegTxnFields>(tx.details));
            } else if (tx.header.type == constants::xAcfg) {
                    assert((node.HasMember(constants::xCurcfg.c_str())));
                    tx.details = model::AssetConfigTxnFields();
                    parseAssetConfigurationInfo(node[constants::xCurcfg.c_str()].GetObject(),
                                        boost::get<model::AssetConfigTxnFields>(tx.details));
            } else if (tx.header.type == constants::xAxfer) {
                    assert((node.HasMember(constants::xCurxfer.c_str())));
                    tx.details = model::AssetTransferTxnFields();
                    parseAssetTransferInfo(node[constants::xCurxfer.c_str()].GetObject(),
                                        boost::get<model::AssetTransferTxnFields>(tx.details));
            } else if (tx.header.type == constants::xAfreeze) {
                    assert((node.HasMember(constants::xCurfrz.c_str())));
                    tx.details = model::AssetFreezeTxnFields();
                    parseAssetFreezeInfo(node[constants::xCurfrz.c_str()].GetObject(),
                                        boost::get<model::AssetFreezeTxnFields>(tx.details));
            }
        }

        template <class T>
        static void parseTransactions(const T& array, std::vector<model::Transaction> & txs) {
            txs.assign((std::size_t) array.Size(), model::Transaction());
            auto index = 0;
            for (const auto& node : array) {
                parseTransaction(node.GetObject(), txs[index]);
                index++;
            }
        }

        template <class T>
        static void parseTransactionParams(const T& node, model::TransactionParams & txParams) {
            getMandatoryStringField(node, constants::xGenesisId, txParams.genesisID);
            getMandatoryStringField(node, constants::xGenesisHashB64, txParams.genesisHash);
            getMandatoryUint64Field(node, constants::xLastRoundParam, txParams.lastRound);
            getMandatoryUint64Field(node, constants::xFee, txParams.suggestedFeePerByte);
            getMandatoryUint64Field(node, constants::xMinFee, txParams.minFee);
            getMandatoryStringField(node, constants::xConsensusVersion, txParams.consensusVersion);
        }

        template <class T>
        static void parseAccount(const T& node, model::Account & account) {
            getMandatoryUint64Field(node, constants::xRound, account.round);
            getMandatoryStringField(node, constants::xAddress, account.address);
            getMandatoryUint64Field(node, constants::xAmount, account.amount);
            getMandatoryUint64Field(node, constants::xPendingRewards, account.pendingRewards);
            getMandatoryUint64Field(node, constants::xAmountWithoutPendingRewards, account.amountWithoutPendingRewards);
            getMandatoryUint64Field(node, constants::xRewards, account.rewards);

            getOptionalStringField(node, constants::xStatus, account.status);

            if (node.HasMember(constants::xAssets.c_str())) {
                parseAssetsAmounts(node[constants::xAssets.c_str()].GetObject(), account.assetsAmounts);
            }

            if (node.HasMember(constants::xThisAssetTotal.c_str())) {
                parseAssetsParams(node[constants::xThisAssetTotal.c_str()].GetObject(), account.createdAssets);
            }

            if (node.HasMember(constants::xParticipation.c_str())) {
                account.participation = model::KeyRegTxnFields();
                parseParticipationInfo(node[constants::xParticipation.c_str()].GetObject(), *account.participation);
            }
        }

    private:

        template <class T>
        static void assertWithMessage(const T & node, const std::string & fieldName) {
            if (!node.HasMember(fieldName.c_str())) {
                throw make_exception(api::ErrorCode::NO_SUCH_ELEMENT, fmt::format("Missing '{}' field in JSON.", fieldName));
            }
        }

        template <class T>
        static void getMandatoryStringField(const T & node, const std::string & fieldName, Option<std::string> & field) {
            assertWithMessage(node, fieldName);
            field = node[fieldName.c_str()].GetString();
        }

        template <class T>
        static void getMandatoryStringField(const T & node, const std::string & fieldName, std::string & field) {
            assertWithMessage(node, fieldName);
            field = node[fieldName.c_str()].GetString();
        }

        template <class T>
        static void getMandatoryUint64Field(const T & node, const std::string & fieldName, Option<uint64_t> & field) {
            assertWithMessage(node, fieldName);
            field = node[fieldName.c_str()].GetUint64();
        }

        template <class T>
        static void getMandatoryUint64Field(const T & node, const std::string & fieldName, uint64_t & field) {
            assertWithMessage(node, fieldName);
            field = node[fieldName.c_str()].GetUint64();
        }

        template <class T>
        static void getMandatoryBoolField(const T & node, const std::string & fieldName, Option<bool> & field) {
            assertWithMessage(node, fieldName);
            field = node[fieldName.c_str()].GetBool();
        }

        template <class T>
        static void getMandatoryBoolField(const T & node, const std::string & fieldName, bool & field) {
            assertWithMessage(node, fieldName);
            field = node[fieldName.c_str()].GetBool();
        }

        template <class T>
        static void getMandatoryAddressField(const T & node, const std::string & fieldName, Option<Address> & field) {
            assertWithMessage(node, fieldName);
            auto addr = node[fieldName.c_str()].GetString();
            // FIXME Should be set to wallet currency instead of hardcoded here
            field = Address(currencies::algorand(), addr);
        }

        template <class T>
        static void getMandatoryAddressField(const T & node, const std::string & fieldName, Address & field) {
            assertWithMessage(node, fieldName);
            auto addr = node[fieldName.c_str()].GetString();
            // FIXME Should be set to wallet currency instead of hardcoded here
            field = Address(currencies::algorand(), addr);
        }

        template <class T>
        static void getMandatoryB64StringField(const T & node, const std::string & fieldName, Option<B64String> & field) {
            assertWithMessage(node, fieldName);
            field = B64String(node[fieldName.c_str()].GetString());
        }

        template <class T>
        static void getMandatoryB64StringField(const T & node, const std::string & fieldName, B64String & field) {
            assertWithMessage(node, fieldName);
            field = B64String(node[fieldName.c_str()].GetString());
        }

        template <class T>
        static void getMandatoryBinaryField(const T & node, const std::string & fieldName, Option<std::vector<uint8_t>> & field) {
            assertWithMessage(node, fieldName);
            std::vector<uint8_t> newBinary;
            BaseConverter::decode(node[fieldName.c_str()].GetString(), BaseConverter::BASE64_RFC4648, newBinary);
            field = newBinary;
        }

        template <class T>
        static void getMandatoryBinaryField(const T & node, const std::string & fieldName, std::vector<uint8_t> & field) {
            assertWithMessage(node, fieldName);
            BaseConverter::decode(node[fieldName.c_str()].GetString(), BaseConverter::BASE64_RFC4648, field);
        }

        // ~~~

        template <class T>
        static void getOptionalStringField(const T & node, const std::string & fieldName, Option<std::string> & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetString();
            }
        }

        template <class T>
        static void getOptionalStringField(const T & node, const std::string & fieldName, std::string & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetString();
            }
        }

        template <class T>
        static void getOptionalUint64Field(const T & node, const std::string & fieldName, Option<uint64_t> & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetUint64();
            }
        }

        template <class T>
        static void getOptionalUint64Field(const T & node, const std::string & fieldName, uint64_t & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetUint64();
            }
        }

        template <class T>
        static void getOptionalBoolField(const T & node, const std::string & fieldName, Option<bool> & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetBool();
            }
        }

        template <class T>
        static void getOptionalBoolField(const T & node, const std::string & fieldName, bool & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetBool();
            }
        }

        template <class T>
        static void getOptionalAddressField(const T & node, const std::string & fieldName, Option<Address> & field) {
            if (node.HasMember(fieldName.c_str())) {
                auto addr = node[fieldName.c_str()].GetString();
                // FIXME Should be set to wallet currency instead of hardcoded here
                field = Address(currencies::algorand(), addr);
            }
        }

        template <class T>
        static void getOptionalAddressField(const T & node, const std::string & fieldName, Address & field) {
            if (node.HasMember(fieldName.c_str())) {
                auto addr = node[fieldName.c_str()].GetString();
                // FIXME Should be set to wallet currency instead of hardcoded here
                field = Address(currencies::algorand(), addr);
            }
        }

        template <class T>
        static void getOptionalB64StringField(const T & node, const std::string & fieldName, Option<B64String> & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = B64String(node[fieldName.c_str()].GetString());
            }
        }

        template <class T>
        static void getOptionalB64StringField(const T & node, const std::string & fieldName, B64String & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = B64String(node[fieldName.c_str()].GetString());
            }
        }

        template <class T>
        static void getOptionalBinaryField(const T & node, const std::string & fieldName, Option<std::vector<uint8_t>> & field) {
            if (node.HasMember(fieldName.c_str())) {
                std::vector<uint8_t> newBinary;
                BaseConverter::decode(node[fieldName.c_str()].GetString(), BaseConverter::BASE64_RFC4648, newBinary);
                field = newBinary;
            }
        }

        template <class T>
        static void getOptionalBinaryField(const T & node, const std::string & fieldName, std::vector<uint8_t> & field) {
            if (node.HasMember(fieldName.c_str())) {
                BaseConverter::decode(node[fieldName.c_str()].GetString(), BaseConverter::BASE64_RFC4648, field);
            }
        }

    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDJSONPARSER_H
