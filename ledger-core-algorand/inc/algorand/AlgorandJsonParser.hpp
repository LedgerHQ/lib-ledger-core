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

// TODO Tests

#ifndef LEDGER_CORE_ALGORANDJSONPARSER_H
#define LEDGER_CORE_ALGORANDJSONPARSER_H

#include <algorand/AlgorandAddress.hpp>
#include <algorand/model/transactions/AlgorandTransaction.hpp>
#include <algorand/model/transactions/AlgorandAssetParams.hpp>
#include <algorand/model/AlgorandAssetAmount.hpp>
#include <algorand/utils/B64String.hpp>

#include <core/math/BigInt.hpp>
#include <core/math/BaseConverter.hpp>
#include <core/utils/Hex.hpp>

namespace ledger {
namespace core {
namespace algorand {
namespace constants {

    // Json objects keys
    static const std::string hash = "hash";
    static const std::string timestamp = "timestamp";
    static const std::string round = "round";

    static const std::string creator = "creator";
    static const std::string amount = "amount";
    static const std::string frozen = "frozen";
    static const std::string total = "total";
    static const std::string decimals = "decimals";
    static const std::string unitName = "unitname";
    static const std::string assetName = "assetname";
    static const std::string managerKey = "managerkey";
    static const std::string freezeAddr = "freezeaddr";
    static const std::string clawbackAddr = "clawbackaddr";
    static const std::string reserveAddr = "reserveaddr";
    static const std::string metadataHash = "metadatahash";
    static const std::string defaultFrozen = "defaultfrozen";
    static const std::string url = "url";

    static const std::string address = "address";
    static const std::string pendingRewards = "pendingrewards";
    static const std::string amountWithoutPendingRewards = "amountwithoutpendingrewards";
    static const std::string rewards = "rewards";
    static const std::string status = "status";
    static const std::string assets = "assets";
    static const std::string thisAssetTotal = "thisassettotal";
    static const std::string participation = "participation";

    static const std::string to = "to";
    static const std::string toRewards = "torewards";
    static const std::string close = "close";
    static const std::string closeAmount = "closeamount";
    static const std::string closeRewards = "closerewards";

    static const std::string selkey = "selkey";
    static const std::string votefst = "votefst";
    static const std::string votekd = "votekd";
    static const std::string votekey = "votekey";
    static const std::string votelst = "votelst";

    static const std::string id = "id";
    static const std::string params = "params";

    static const std::string amt = "amt";
    static const std::string rcv = "rcv";
    static const std::string snd = "snd";
    static const std::string closeTo = "closeto";

    static const std::string acct = "acct";
    static const std::string freeze = "freeze";

    static const std::string type = "type";
    static const std::string tx = "tx";
    static const std::string from = "from";
    static const std::string fee = "fee";
    static const std::string firstRound = "first-round";
    static const std::string lastRound = "last-round";
    static const std::string noteB64 = "noteb64";
    static const std::string fromRewards = "fromrewards";
    static const std::string genesisId = "genesisID";
    static const std::string genesisHashB64 = "genesishashb64";
    static const std::string group = "group";
    static const std::string lease = "lease";

    static const std::string payment = "payment";
    static const std::string keyregs = "keyreg";
    static const std::string curcfg = "curcfg";
    static const std::string curxfer = "curxfer";
    static const std::string curfrz = "curfrz";

    static const std::string transactions = "transactions";
    static const std::string txId = "txId";
    static const std::string minFee = "fee";
    static const std::string consensusVersion = "consensusVersion";

} // namespace constants

    class JsonParser {

    public:

        template <class T>
        static void parseBlock(const T& node, const std::string& currencyName, api::Block & block) {
            block.currencyName = currencyName;
            getMandatoryStringField(node, constants::hash, block.blockHash);

            // FIXME Test this!
            block.time = std::chrono::time_point<std::chrono::system_clock>(std::chrono::seconds(node[constants::timestamp.c_str()].GetUint64()));
            //block.time = DateUtils::fromJSON(node[constants::timestampField].GetUint64());
            //getMandatoryUint64Field(node, constants::timestampField, block.time);

            // FIXME Test this!
            uint64_t blockHeight = 0;
            getMandatoryUint64Field(node, constants::round, blockHeight);
            if (blockHeight > std::numeric_limits<int64_t>::max()) {
                throw make_exception(api::ErrorCode::OUT_OF_RANGE, "Block height exceeds maximum value");
            }
            block.height = (int64_t) blockHeight;
            //block.height = BigInt::fromScalar<uint64_t>(node[constants::round].GetUint64()).toInt64();
        }

        template <class T>
        static void parseAssetAmount(const T& node, model::AssetAmount & assetAmount) {
            getMandatoryAddressField(node, constants::creator, assetAmount.creatorAddress);
            getMandatoryUint64Field(node, constants::amount, assetAmount.amount);
            getMandatoryBoolField(node, constants::frozen, assetAmount.frozen);
        }

        template <class T>
        static void parseAssetsAmounts(const T& node, std::map<uint64_t, model::AssetAmount> & assetsAmounts) {
            for (rapidjson::Value::ConstMemberIterator child = node.MemberBegin(); child != node.MemberEnd(); ++child) {
                model::AssetAmount assetAmount;
                const auto coinId = child->name.GetUint64();
                parseAssetAmount(child->value, assetAmount);
                assetsAmounts[coinId] = assetAmount;
            }
        }

        template <typename T>
        static void parseAssetParams(const T& node, model::AssetParams & assetParams) {

            getMandatoryAddressField(node, constants::creator, *assetParams.creatorAddr);
            getMandatoryUint64Field(node, constants::total, *assetParams.total);
            assert(node.HasMember(constants::decimals.c_str()));
            *assetParams.decimals = node[constants::decimals.c_str()].GetUint();
            getMandatoryBoolField(node, constants::defaultFrozen, *assetParams.defaultFrozen);
            getMandatoryStringField(node, constants::unitName, *assetParams.unitName);
            getMandatoryStringField(node, constants::assetName, *assetParams.assetName);

            getOptionalAddressField(node, constants::managerKey, *assetParams.managerAddr);
            getOptionalAddressField(node, constants::freezeAddr, *assetParams.freezeAddr);
            getOptionalAddressField(node, constants::clawbackAddr, *assetParams.clawbackAddr);
            getOptionalAddressField(node, constants::reserveAddr, *assetParams.reserveAddr);
            getOptionalBinaryField(node, constants::metadataHash, *assetParams.metaDataHash);
            getOptionalStringField(node, constants::url, *assetParams.url);
        }

        template <class T>
        static void parseAssetsParams(const T& node, std::map<uint64_t, model::AssetParams> & assetsParams) {
            for (rapidjson::Value::ConstMemberIterator child = node.MemberBegin(); child != node.MemberEnd(); ++child) {
                model::AssetParams assetParams;
                const auto assetId = child->name.GetUint64();
                parseAssetParams(child->value, assetParams);
                assetsParams[assetId] = assetParams;
            }
        }

        template <class T>
        static void parsePaymentInfo(const T& node, model::PaymentTxnFields & details) {
            getMandatoryAddressField(node, constants::to, details.receiverAddr);
            getMandatoryUint64Field(node, constants::amount, details.amount);

            getOptionalAddressField(node, constants::close, *details.closeAddr);
            getOptionalUint64Field(node, constants::closeAmount, *details.closeAmount);
            getOptionalUint64Field(node, constants::closeRewards, *details.closeRewards);
            getOptionalUint64Field(node, constants::toRewards, *details.receiverRewards);
        }

        // WARNING This has not been tested
        template <class T>
        static void parseParticipationInfo(const T& node, model::KeyRegTxnFields & details) {
            getMandatoryStringField(node, constants::votekey , details.selectionPk);
            getMandatoryStringField(node, constants::selkey, details.votePk);
            getMandatoryUint64Field(node, constants::votekd, details.voteKeyDilution);
            getMandatoryUint64Field(node, constants::votefst, details.voteFirst);
            getMandatoryUint64Field(node, constants::votelst, details.voteLast);
        }

        template <class T>
        static void parseAssetConfigurationInfo(const T& node, model::AssetConfigTxnFields & details) {
            getMandatoryUint64Field(node, constants::id, *details.assetId);

            assert(node.HasMember(constants::params.c_str()));
            parseAssetParams(node[constants::params.c_str()].GetObject(), *details.assetParams);
        }

        template <class T>
        static void parseAssetTransferInfo(const T& node, model::AssetTransferTxnFields & details) {
            getMandatoryUint64Field(node, constants::id, details.assetId);
            getMandatoryAddressField(node, constants::rcv, details.assetReceiver);
            getMandatoryUint64Field(node, constants::amt, *details.assetAmount);

            getOptionalAddressField(node, constants::closeTo, *details.assetCloseTo);
            getOptionalAddressField(node, constants::snd, *details.assetSender);
        }

        // WARNING This has not been tested
        template <class T>
        static void parseAssetFreezeInfo(const T& node, model::AssetFreezeTxnFields & details) {
            getMandatoryUint64Field(node, constants::id, details.assetId);
            getMandatoryAddressField(node, constants::acct, details.frozenAddress);
            getMandatoryBoolField(node, constants::freeze, details.assetFrozen);
        }

        template <class T>
        static void parseTransaction(const T& node, model::Transaction & tx) {

            getMandatoryStringField(node, constants::type, tx.header.type);
            getMandatoryStringField(node, constants::tx, *tx.header.id);
            getMandatoryAddressField(node, constants::from, tx.header.sender);
            getMandatoryUint64Field(node, constants::firstRound, tx.header.firstValid);
            getMandatoryUint64Field(node, constants::lastRound, tx.header.lastValid);
            getMandatoryUint64Field(node, constants::round, *tx.header.round);
            getMandatoryUint64Field(node, constants::fee, tx.header.fee);
            getMandatoryB64StringField(node, constants::genesisHashB64, tx.header.genesisHash);

            getOptionalStringField(node, constants::genesisId, *tx.header.genesisId);
            getOptionalBinaryField(node, constants::noteB64, *tx.header.note);
            getOptionalUint64Field(node, constants::fromRewards, *tx.header.fromRewards);
            getOptionalBinaryField(node, constants::group, *tx.header.group);
            getOptionalBinaryField(node, constants::lease, *tx.header.lease);

            if (tx.header.type == model::constants::pay) {
                    assert((node.HasMember(constants::payment.c_str())));
                    tx.details = model::PaymentTxnFields();
                    parsePaymentInfo(node[constants::payment.c_str()].GetObject(),
                                        boost::get<model::PaymentTxnFields>(tx.details));
            } else if (tx.header.type == model::constants::keyreg) {
                    assert((node.HasMember(constants::keyregs.c_str())));
                    tx.details = model::KeyRegTxnFields();
                    parseParticipationInfo(node[constants::keyregs.c_str()].GetObject(),
                                        boost::get<model::KeyRegTxnFields>(tx.details));
            } else if (tx.header.type == model::constants::acfg) {
                    assert((node.HasMember(constants::curcfg.c_str())));
                    tx.details = model::AssetConfigTxnFields();
                    parseAssetConfigurationInfo(node[constants::curcfg.c_str()].GetObject(),
                                        boost::get<model::AssetConfigTxnFields>(tx.details));
            } else if (tx.header.type == model::constants::axfer) {
                    assert((node.HasMember(constants::curxfer.c_str())));
                    tx.details = model::AssetTransferTxnFields();
                    parseAssetTransferInfo(node[constants::curxfer.c_str()].GetObject(),
                                        boost::get<model::AssetTransferTxnFields>(tx.details));
            } else if (tx.header.type == model::constants::afreeze) {
                    assert((node.HasMember(constants::curfrz.c_str())));
                    tx.details = model::AssetFreezeTxnFields();
                    parseAssetFreezeInfo(node[constants::curfrz.c_str()].GetObject(),
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
            getMandatoryStringField(node, constants::genesisId, txParams.genesisID);
            getMandatoryStringField(node, constants::genesisHashB64, txParams.genesisHash);
            getMandatoryUint64Field(node, constants::lastRound, txParams.lastRound);
            getMandatoryUint64Field(node, constants::fee, txParams.suggestedFeePerByte);
            getMandatoryUint64Field(node, constants::minFee, txParams.minFee);
            getMandatoryStringField(node, constants::consensusVersion, txParams.consensusVersion);
        }

        template <class T>
        static void parseAccount(const T& node, model::Account & account) {
            getMandatoryUint64Field(node, constants::round, account.round);
            getMandatoryStringField(node, constants::address, account.address);
            account.pubKeyHex = hex::toString(Address::toPublicKey(account.address));
            getMandatoryUint64Field(node, constants::amount, account.amount);
            getMandatoryUint64Field(node, constants::pendingRewards, account.pendingRewards);
            getMandatoryUint64Field(node, constants::amountWithoutPendingRewards, account.amountWithoutPendingRewards);
            getMandatoryUint64Field(node, constants::rewards, account.rewards);

            getOptionalStringField(node, constants::status, account.status);

            if (node.HasMember(constants::assets.c_str())) {
                parseAssetsAmounts(node[constants::assets.c_str()].GetObject(), account.assetsAmounts);
            }

            if (node.HasMember(constants::thisAssetTotal.c_str())) {
                parseAssetsParams(node[constants::thisAssetTotal.c_str()].GetObject(), account.createdAssets);
            }

            if (node.HasMember(constants::participation.c_str())) {
                parseParticipationInfo(node[constants::participation.c_str()].GetObject(), *account.participation);
            }
        }

    private:

        template <class T>
        static void getMandatoryStringField(const T & node, const std::string & fieldName, std::string & field) {
            assert(node.HasMember(fieldName.c_str()));
            field = node[fieldName.c_str()].GetString();
        }

        template <class T>
        static void getMandatoryUint64Field(const T & node, const std::string & fieldName, uint64_t & field) {
            assert(node.HasMember(fieldName.c_str()));
            field = node[fieldName.c_str()].GetUint64();
        }

        template <class T>
        static void getMandatoryBoolField(const T & node, const std::string & fieldName, bool & field) {
            assert(node.HasMember(fieldName.c_str()));
            field = node[fieldName.c_str()].GetBool();
        }

        template <class T>
        static void getMandatoryAddressField(const T & node, const std::string & fieldName, Address & field) {
            assert(node.HasMember(fieldName.c_str()));
            auto addr = node[fieldName.c_str()].GetString();
            // FIXME Should be set to wallet currency instead of hardcoded here
            field = Address(currencies::algorand(), addr);
        }

        template <class T>
        static void getMandatoryB64StringField(const T & node, const std::string & fieldName, B64String & field) {
            assert(node.HasMember(fieldName.c_str()));
            field = B64String(node[fieldName.c_str()].GetString());
        }

        template <class T>
        static void getMandatoryBinaryField(const T & node, const std::string & fieldName, std::vector<uint8_t> & field) {
            assert(node.HasMember(fieldName.c_str()));
            BaseConverter::decode(node[fieldName.c_str()].GetString(), BaseConverter::BASE64_RFC4648, field);
        }

        // ~~~

        template <class T>
        static void getOptionalStringField(const T & node, const std::string & fieldName, std::string & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetString();
            }
        }

        template <class T>
        static void getOptionalUint64Field(const T & node, const std::string & fieldName, uint64_t & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetUint64();
            }
        }

        template <class T>
        static void getOptionalBoolField(const T & node, const std::string & fieldName, bool & field) {
            if (node.HasMember(fieldName.c_str())) {
                field = node[fieldName.c_str()].GetBool();
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
        static void getOptionalB64StringField(const T & node, const std::string & fieldName, B64String & field) {
            if (node.HasMember(fieldName.c_str())) {
                auto addr = node[fieldName.c_str()].GetString();
                field = B64String(node[fieldName.c_str()].GetString());
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
