/*
 * AlgorandTransactionDatabaseHelper
 *
 * Created by Hakim Aammar on 18/05/2020.
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

#include "AlgorandTransactionDatabaseHelper.hpp"
#include "../AlgorandExplorerConstants.hpp"
#include "../model/transactions/AlgorandPayment.hpp"
#include "../model/transactions/AlgorandKeyreg.hpp"
#include "../model/transactions/AlgorandAsset.hpp"
#include "../utils/B64String.hpp"

#include <crypto/SHA256.hpp>
#include <math/BaseConverter.hpp>

#include <fmt/format.h>

namespace ledger {
namespace core {
namespace algorand {

    static constexpr auto COL_TX_HASH = 1;
    static constexpr auto COL_TX_TYPE = 2;
    static constexpr auto COL_TX_ROUND = 3;
    static constexpr auto COL_TX_TIMESTAMP = 4;
    static constexpr auto COL_TX_FIRST_VALID = 5;
    static constexpr auto COL_TX_LAST_VALID = 6;
    static constexpr auto COL_TX_GENESIS_ID = 7;
    static constexpr auto COL_TX_GENESIS_HASH = 8;
    static constexpr auto COL_TX_SENDER = 9;
    static constexpr auto COL_TX_FEE = 10;
    static constexpr auto COL_TX_NOTE = 11;
    static constexpr auto COL_TX_GROUP = 12;
    static constexpr auto COL_TX_LEASE = 13;

    static constexpr auto COL_TX_PAY_AMOUNT = 14;
    static constexpr auto COL_TX_PAY_FROM_REWARDS = 15;
    static constexpr auto COL_TX_PAY_RECEIVER_ADDRESS = 16;
    static constexpr auto COL_TX_PAY_RECEIVER_REWARDS = 17;
    static constexpr auto COL_TX_PAY_CLOSE_ADDRESS = 18;
    static constexpr auto COL_TX_PAY_CLOSE_AMOUNT = 19;
    static constexpr auto COL_TX_PAY_CLOSE_REWARDS = 20;

    static constexpr auto COL_TX_KEYREG_NONPART = 21;
    static constexpr auto COL_TX_KEYREG_SELECTION_PK = 22;
    static constexpr auto COL_TX_KEYREG_VOTE_PK = 23;
    static constexpr auto COL_TX_KEYREG_VOTE_KEY_DILUTION = 24;
    static constexpr auto COL_TX_KEYREG_VOTE_FIRST = 25;
    static constexpr auto COL_TX_KEYREG_VOTE_LAST = 26;

    static constexpr auto COL_TX_ACFG_ASSET_ID = 27;
    static constexpr auto COL_TX_ACFG_ASSET_NAME = 28;
    static constexpr auto COL_TX_ACFG_UNIT_NAME = 29;
    static constexpr auto COL_TX_ACFG_TOTAL = 30;
    static constexpr auto COL_TX_ACFG_DECIMALS = 31;
    static constexpr auto COL_TX_ACFG_DEFAULT_FROZEN = 32;
    static constexpr auto COL_TX_ACFG_CREATOR_ADDRESS = 33;
    static constexpr auto COL_TX_ACFG_MANAGER_ADDRESS = 34;
    static constexpr auto COL_TX_ACFG_RESERVE_ADDRESS = 35;
    static constexpr auto COL_TX_ACFG_FREEZE_ADDRESS = 36;
    static constexpr auto COL_TX_ACFG_CLAWBACK_ADDRESS = 37;
    static constexpr auto COL_TX_ACFG_METADATA_HASH = 38;
    static constexpr auto COL_TX_ACFG_URL = 39;

    static constexpr auto COL_TX_AXFER_ASSET_ID = 40;
    static constexpr auto COL_TX_AXFER_ASSET_AMOUNT = 41;
    static constexpr auto COL_TX_AXFER_RECEIVER_ADDRESS = 42;
    static constexpr auto COL_TX_AXFER_CLOSE_ADDRESS = 43;
    static constexpr auto COL_TX_AXFER_SENDER_ADDRESS = 44;

    static constexpr auto COL_TX_AFRZ_ASSET_ID = 45;
    static constexpr auto COL_TX_AFRZ_FROZEN = 46;
    static constexpr auto COL_TX_AFRZ_FROZEN_ADDRESS = 47;


    // Helpers to deal with Option<> types,
    // because SOCI only understands boost::optional<>...

    template<class Raw>
    static auto optionalValue(Option<Raw> opt) {
        return opt.hasValue() ? *opt : boost::optional<Raw>();
    }

    template<class Raw, class Transformed>
    static auto optionalValueWithTransform(Option<Raw> opt, const std::function<Transformed(Raw)> & transform) {
        return opt.hasValue() ? transform(*opt) : boost::optional<Transformed>();
    }

    static std::string getString(const soci::row & row, const int32_t colId) {
        return row.get<std::string>(colId);
    }

    static Option<std::string> getOptionalString(const soci::row & row, const int32_t colId) {
        return row.get_indicator(colId) != soci::i_null ? Option<std::string>(row.get<std::string>(colId)) : Option<std::string>::NONE;
    }

    template<class Transformed>
    static Option<Transformed> getOptionalStringWithTransform(const soci::row & row, const int32_t colId, const std::function<Transformed(std::string)> & transform) {
        return row.get_indicator(colId) != soci::i_null ? Option<Transformed>(transform(row.get<std::string>(colId))) : Option<Transformed>::NONE;
    }

    static uint64_t getNumber(const soci::row & row, const int32_t colId) {
        return soci::get_number<uint64_t>(row, colId);
    }

    static Option<uint64_t> getOptionalNumber(const soci::row & row, const int32_t colId) {
        return row.get_indicator(colId) != soci::i_null ? Option<uint64_t>(soci::get_number<uint64_t>(row, colId)) : Option<uint64_t>::NONE;
    }

    template<class Transformed>
    static Option<Transformed> getOptionalNumberWithTransform(const soci::row & row, const int32_t colId, const std::function<Transformed(uint64_t)> & transform) {
        return row.get_indicator(colId) != soci::i_null ? Option<Transformed>(transform(soci::get_number<uint64_t>(row, colId))) : Option<Transformed>::NONE;
    }

    static const auto numToBool = [] (const uint64_t& num) { return !! num; };
    static const auto boolToNum = [] (const bool& b) { return static_cast<int32_t>(b); };
    static const auto stringToAddr = [] (const std::string& addr) { return Address(addr); };
    static const auto addrToString = [] (const Address& addr) { return addr.toString(); };
    static const auto b64toBytes = [] (const std::string& b64) {
        auto bytes = std::vector<uint8_t>();
        BaseConverter::decode(b64, BaseConverter::BASE64_RFC4648, bytes);
        return bytes;
    };
    static const auto bytesToB64 = [] (const std::vector<uint8_t>& bytes) {
        return BaseConverter::encode(bytes, BaseConverter::BASE64_RFC4648);
    };


    static void putPaymentTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& payment = boost::get<model::PaymentTxnFields>(tx.details);

        auto header_id = optionalValue<std::string>(tx.header.id);
        auto header_genesisId = optionalValue<std::string>(tx.header.genesisId);
        auto header_round = optionalValue<uint64_t>(tx.header.round);
        auto header_timestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto header_note = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto header_group = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto header_lease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto payment_fromrewards = optionalValue<uint64_t>(payment.fromRewards);
        auto payment_receiverRewards = optionalValue<uint64_t>(payment.receiverRewards);
        auto payment_closeAddr = optionalValueWithTransform<Address, std::string>(payment.closeAddr, addrToString);
        auto payment_closeAmount = optionalValue<uint64_t>(payment.closeAmount);
        auto payment_closeRewards = optionalValue<uint64_t>(payment.closeRewards);

        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, from_rewards, note, groupVal, leaseVal, "
                "pay_amount, pay_receiver_address, pay_receiver_rewards, pay_close_address, pay_close_amount, pay_close_rewards) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :from_rewards, :note, :group, :lease, "
                ":amount, :receiver_addr, :receiver_rewards, :close_addr, :close_amount, :close_rewards)",
            soci::use(txUid),
            soci::use(header_id),
            soci::use(tx.header.type),
            soci::use(header_round),
            soci::use(header_timestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(header_genesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(payment_fromrewards),
            soci::use(header_note),
            soci::use(header_group),
            soci::use(header_lease),
            soci::use(payment.amount),
            soci::use(payment.receiverAddr.toString()),
            soci::use(payment_receiverRewards),
            soci::use(payment_closeAddr),
            soci::use(payment_closeAmount),
            soci::use(payment_closeRewards);
    }

    // NOTE This has not beed tested
    static void putKeyRegTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& keyreg = boost::get<model::KeyRegTxnFields>(tx.details);
        auto header_id = optionalValue<std::string>(tx.header.id);
        auto header_genesisId = optionalValue<std::string>(tx.header.genesisId);
        auto header_round = optionalValue<uint64_t>(tx.header.round);
        auto header_timestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto header_note = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto header_group = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto header_lease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto keyreg_nonParticipation = optionalValueWithTransform<bool, int32_t>(keyreg.nonParticipation, boolToNum);

        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, "
                "keyreg_non_participation, keyreg_selection_pk, keyreg_vote_pk, keyreg_vote_key_dilution, keyreg_vote_first, keyreg_vote_last) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, "
                ":non_part, :selection_pk, :vote_pk, :vote_key_dilution, :vote_first, :vote_last)",
            soci::use(txUid),
            soci::use(header_id),
            soci::use(tx.header.type),
            soci::use(header_round),
            soci::use(header_timestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(header_genesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(header_note),
            soci::use(header_group),
            soci::use(header_lease),
            soci::use(keyreg_nonParticipation),
            soci::use(keyreg.selectionPk),
            soci::use(keyreg.votePk),
            soci::use(keyreg.voteKeyDilution),
            soci::use(keyreg.voteFirst),
            soci::use(keyreg.voteLast);
    }

    static void putAssetConfigTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& assetConfig = boost::get<model::AssetConfigTxnFields>(tx.details);
        auto& assetParams = *assetConfig.assetParams;
        auto header_id = optionalValue<std::string>(tx.header.id);
        auto header_genesisId = optionalValue<std::string>(tx.header.genesisId);
        auto header_round = optionalValue<uint64_t>(tx.header.round);
        auto header_timestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto header_note = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto header_group = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto header_lease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto asset_id = optionalValue<uint64_t>(assetConfig.assetId);
        auto asset_name = optionalValue<std::string>(assetParams.assetName);
        auto asset_unitname = optionalValue<std::string>(assetParams.unitName);
        auto asset_total = optionalValue<uint64_t>(assetParams.total);
        auto asset_decimals = optionalValue<uint32_t>(assetParams.decimals);
        auto asset_frozen = optionalValueWithTransform<bool, int32_t>(assetParams.defaultFrozen, boolToNum);
        auto asset_creator = optionalValueWithTransform<Address, std::string>(assetParams.creatorAddr, addrToString);
        auto asset_manager = optionalValueWithTransform<Address, std::string>(assetParams.managerAddr, addrToString);
        auto asset_reserve = optionalValueWithTransform<Address, std::string>(assetParams.reserveAddr, addrToString);
        auto asset_freeze = optionalValueWithTransform<Address, std::string>(assetParams.freezeAddr, addrToString);
        auto asset_clawback = optionalValueWithTransform<Address, std::string>(assetParams.clawbackAddr, addrToString);
        auto asset_metadata = optionalValueWithTransform<std::vector<uint8_t>, std::string>(assetParams.metaDataHash, bytesToB64);
        auto asset_url = optionalValue<std::string>(assetParams.url);


        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, "
                "acfg_asset_id, acfg_asset_name, acfg_unit_name, acfg_total, acfg_decimals, acfg_default_frozen, "
                "acfg_creator_address, acfg_manager_address, acfg_reserve_address, acfg_freeze_address, acfg_clawback_address, "
                "acfg_metadata_hash, acfg_url) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, "
                ":asset_id, :asset_name, :unit_name, :total, :decimals, :default_frozen, :creator_addr, "
                ":manager_addr, :reserve_addr, :freeze_addr, :clawback_addr, :metadata_hash, :url)",
            soci::use(txUid),
            soci::use(header_id),
            soci::use(tx.header.type),
            soci::use(header_round),
            soci::use(header_timestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(header_genesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(header_note),
            soci::use(header_group),
            soci::use(header_lease),
            soci::use(asset_id),
            soci::use(asset_name),
            soci::use(asset_unitname),
            soci::use(asset_total),
            soci::use(asset_decimals),
            soci::use(asset_frozen),
            soci::use(asset_creator),
            soci::use(asset_manager),
            soci::use(asset_reserve),
            soci::use(asset_freeze),
            soci::use(asset_clawback),
            soci::use(asset_metadata),
            soci::use(asset_url);
    }

    static void putAssetTransferTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& assetTransfer = boost::get<model::AssetTransferTxnFields>(tx.details);
        auto header_id = optionalValue<std::string>(tx.header.id);
        auto header_genesisId = optionalValue<std::string>(tx.header.genesisId);
        auto header_round = optionalValue<uint64_t>(tx.header.round);
        auto header_timestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto header_note = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto header_group = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto header_lease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto asset_amount = optionalValue<uint64_t>(assetTransfer.assetAmount);
        auto asset_closeto = optionalValueWithTransform<Address, std::string>(assetTransfer.assetCloseTo, addrToString);
        auto asset_sender = optionalValueWithTransform<Address, std::string>(assetTransfer.assetSender, addrToString);

        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, "
                "axfer_asset_id, axfer_asset_amount, axfer_receiver_address, axfer_close_address, axfer_sender_address) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, "
                ":asset_id, :amount, :receiver_addr, :close_addr, :sender_addr)",
            soci::use(txUid),
            soci::use(header_id),
            soci::use(tx.header.type),
            soci::use(header_round),
            soci::use(header_timestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(header_genesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(header_note),
            soci::use(header_group),
            soci::use(header_lease),
            soci::use(assetTransfer.assetId),
            soci::use(asset_amount),
            soci::use(assetTransfer.assetReceiver.toString()),
            soci::use(asset_closeto),
            soci::use(asset_sender);
    }

    // NOTE This has not beed tested
    static void putAssetFreezeTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& assetFreeze = boost::get<model::AssetFreezeTxnFields>(tx.details);
        auto header_id = optionalValue<std::string>(tx.header.id);
        auto header_genesisId = optionalValue<std::string>(tx.header.genesisId);
        auto header_round = optionalValue<uint64_t>(tx.header.round);
        auto header_timestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto header_note = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto header_group = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto header_lease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);

        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, "
                "afrz_asset_id, afrz_frozen, afrz_frozen_address) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, "
                ":asset_id, :frozen, :frozen_addr)",
            soci::use(txUid),
            soci::use(header_id),
            soci::use(tx.header.type),
            soci::use(header_round),
            soci::use(header_timestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(header_genesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(header_note),
            soci::use(header_group),
            soci::use(header_lease),
            soci::use(assetFreeze.assetId),
            soci::use(static_cast<int32_t>(assetFreeze.assetFrozen)),
            soci::use(assetFreeze.frozenAddress.toString());
    }

    static void inflateTransaction(const soci::row & row, model::Transaction & tx) {

        tx.header.id = getOptionalString(row, COL_TX_HASH);
        tx.header.type = getString(row, COL_TX_TYPE);
        tx.header.round = getOptionalNumber(row, COL_TX_ROUND);
        tx.header.timestamp = getOptionalNumber(row, COL_TX_TIMESTAMP);
        tx.header.firstValid = getNumber(row, COL_TX_FIRST_VALID);
        tx.header.lastValid = getNumber(row, COL_TX_LAST_VALID);
        tx.header.genesisId = getOptionalString(row, COL_TX_GENESIS_ID);
        tx.header.genesisHash = B64String(getString(row, COL_TX_GENESIS_HASH));
        tx.header.sender = Address(getString(row, COL_TX_SENDER)); // FIXME Currency not specified: defaults to ALGO
        tx.header.fee = getNumber(row, COL_TX_FEE);
        tx.header.note = getOptionalStringWithTransform<std::vector<uint8_t>>(row, COL_TX_NOTE, b64toBytes);
        tx.header.group = getOptionalStringWithTransform<std::vector<uint8_t>>(row, COL_TX_GROUP, b64toBytes);
        tx.header.lease = getOptionalStringWithTransform<std::vector<uint8_t>>(row, COL_TX_LEASE, b64toBytes);

        if (tx.header.type == constants::xPay) {

            model::PaymentTxnFields paymentDetails;

            paymentDetails.amount = getNumber(row, COL_TX_PAY_AMOUNT);
            paymentDetails.receiverAddr = Address(getString(row, COL_TX_PAY_RECEIVER_ADDRESS));
            paymentDetails.receiverRewards = getOptionalNumber(row, COL_TX_PAY_RECEIVER_REWARDS);
            paymentDetails.closeAddr = getOptionalStringWithTransform<Address>(row, COL_TX_PAY_CLOSE_ADDRESS, stringToAddr);
            paymentDetails.closeAmount = getOptionalNumber(row, COL_TX_PAY_CLOSE_AMOUNT);
            paymentDetails.closeRewards = getOptionalNumber(row, COL_TX_PAY_CLOSE_REWARDS);
            paymentDetails.fromRewards = getOptionalNumber(row, COL_TX_PAY_FROM_REWARDS);

            tx.details = paymentDetails;

        } else if (tx.header.type == constants::xKeyregs) {
            // NOTE This has not beed tested

            model::KeyRegTxnFields keyregDetails;

            keyregDetails.nonParticipation = getOptionalNumberWithTransform<bool>(row, COL_TX_KEYREG_NONPART, numToBool);
            keyregDetails.selectionPk = getString(row, COL_TX_KEYREG_SELECTION_PK);
            keyregDetails.votePk = getString(row, COL_TX_KEYREG_VOTE_PK);
            keyregDetails.voteKeyDilution = getNumber(row, COL_TX_KEYREG_VOTE_KEY_DILUTION);
            keyregDetails.voteFirst = getNumber(row, COL_TX_KEYREG_VOTE_FIRST);
            keyregDetails.voteLast = getNumber(row, COL_TX_KEYREG_VOTE_LAST);

            tx.details = keyregDetails;

        } else if (tx.header.type == constants::xAcfg) {

            model::AssetConfigTxnFields assetConfigDetails;

            assetConfigDetails.assetId = getOptionalNumber(row, COL_TX_ACFG_ASSET_ID);
            if (row.get_indicator(COL_TX_ACFG_TOTAL) != soci::i_null) {
                model::AssetParams assetParams;

                assetParams.assetName = getOptionalString(row, COL_TX_ACFG_ASSET_NAME);
                assetParams.unitName = getOptionalString(row, COL_TX_ACFG_UNIT_NAME);
                assetParams.total = getOptionalNumber(row, COL_TX_ACFG_TOTAL);
                assetParams.decimals = row.get_indicator(COL_TX_ACFG_DECIMALS) != soci::i_null ? Option<uint32_t>(soci::get_number<uint32_t>(row, COL_TX_ACFG_DECIMALS)) : Option<uint32_t>::NONE;
                assetParams.defaultFrozen = getOptionalNumberWithTransform<bool>(row, COL_TX_ACFG_DEFAULT_FROZEN, numToBool);
                assetParams.creatorAddr = getOptionalStringWithTransform<Address>(row, COL_TX_ACFG_CREATOR_ADDRESS, stringToAddr);
                assetParams.managerAddr = getOptionalStringWithTransform<Address>(row, COL_TX_ACFG_MANAGER_ADDRESS, stringToAddr);
                assetParams.reserveAddr = getOptionalStringWithTransform<Address>(row, COL_TX_ACFG_RESERVE_ADDRESS, stringToAddr);
                assetParams.freezeAddr = getOptionalStringWithTransform<Address>(row, COL_TX_ACFG_FREEZE_ADDRESS, stringToAddr);
                assetParams.clawbackAddr = getOptionalStringWithTransform<Address>(row, COL_TX_ACFG_CLAWBACK_ADDRESS, stringToAddr);
                assetParams.url = getOptionalString(row, COL_TX_ACFG_URL);
                assetParams.metaDataHash = getOptionalStringWithTransform<std::vector<uint8_t>>(row, COL_TX_ACFG_METADATA_HASH, b64toBytes);

                assetConfigDetails.assetParams = assetParams;
            }

            tx.details = assetConfigDetails;

        } else if (tx.header.type == constants::xAxfer) {

            model::AssetTransferTxnFields assetTransferDetails;

            assetTransferDetails.assetId = getNumber(row, COL_TX_AXFER_ASSET_ID);
            assetTransferDetails.assetAmount = getOptionalNumber(row, COL_TX_AXFER_ASSET_AMOUNT);
            assetTransferDetails.assetReceiver = Address(getString(row, COL_TX_AXFER_RECEIVER_ADDRESS));
            assetTransferDetails.assetCloseTo = getOptionalStringWithTransform<Address>(row, COL_TX_AXFER_CLOSE_ADDRESS, stringToAddr);
            assetTransferDetails.assetSender = getOptionalStringWithTransform<Address>(row, COL_TX_AXFER_SENDER_ADDRESS, stringToAddr);

            tx.details = assetTransferDetails;

        } else if (tx.header.type == constants::xAfreeze) {
            // NOTE This has not beed tested

            model::AssetFreezeTxnFields assetFreezeDetails;

            assetFreezeDetails.assetId = getNumber(row, COL_TX_AFRZ_ASSET_ID);
            assetFreezeDetails.assetFrozen = !! getNumber(row, COL_TX_AFRZ_FROZEN);
            assetFreezeDetails.frozenAddress = Address(getString(row, COL_TX_AFRZ_FROZEN_ADDRESS));

            tx.details = assetFreezeDetails;
        }
    }

    bool TransactionDatabaseHelper::transactionExists(soci::session & sql, const std::string & txUid) {
        int32_t count = 0;
        sql << "SELECT COUNT(*) FROM algorand_transactions WHERE transaction_uid = :txUid", soci::use(txUid), soci::into(count);
        return count == 1;
    }

    bool TransactionDatabaseHelper::getTransactionByHash(soci::session & sql,
                                                         const std::string & hash,
                                                         model::Transaction & tx) {

        soci::rowset<soci::row> rows = (sql.prepare <<
                "SELECT *"
                "FROM algorand_transactions AS tx "
                "WHERE tx.hash = :hash", soci::use(hash));

        for (auto &row : rows) {
            inflateTransaction(row, tx);
            return true;
        }

        return false;
    }

    std::string TransactionDatabaseHelper::createTransactionUid(const std::string & accountUid, const std::string & txHash) {
        return SHA256::stringToHexHash(fmt::format("uid:{}+{}", accountUid, txHash));
    }

    std::string TransactionDatabaseHelper::putTransaction(soci::session & sql,
                                                          const std::string & accountUid,
                                                          const model::Transaction & tx) {
        auto txUid = createTransactionUid(accountUid, *tx.header.id);

        if (tx.header.type == constants::xPay) {
            putPaymentTransaction(sql, txUid, tx);
        } else if (tx.header.type == constants::xKeyregs) {
            putKeyRegTransaction(sql, txUid, tx);
        } else if (tx.header.type == constants::xAcfg) {
            putAssetConfigTransaction(sql, txUid, tx);
        } else if (tx.header.type == constants::xAxfer) {
            putAssetTransferTransaction(sql, txUid, tx);
        } else if (tx.header.type == constants::xAfreeze) {
            putAssetFreezeTransaction(sql, txUid, tx);
        }

        return txUid;
    }

    std::vector<model::Transaction> TransactionDatabaseHelper::queryAssetTransferTransactionsInvolving(
            soci::session& sql,
            uint64_t assetId,
            const std::string& address)
    {
        soci::rowset<soci::row> rows = (sql.prepare <<
                "SELECT *"
                "FROM algorand_transactions AS tx "
                "WHERE tx.type = :type "
                "AND tx.axfer_asset_id = :axfer_asset_id "
                "AND (tx.axfer_receiver_address = :axfer_receiver_address "
                "OR tx.axfer_close_address = :axfer_close_address "
                "OR tx.axfer_sender_address = :axfer_sender_address "
                "OR tx.sender = :sender)",
                soci::use(std::string(model::constants::axfer)), soci::use(assetId),
                soci::use(address), soci::use(address),
                soci::use(address), soci::use(address));

        return query(rows);
    }

    std::vector<model::Transaction> TransactionDatabaseHelper::queryTransactionsInvolving(
            soci::session& sql,
            const std::string& address)
    {
        soci::rowset<soci::row> rows = (sql.prepare <<
                "SELECT *"
                "FROM algorand_transactions AS tx "
                "WHERE tx.sender = :sender "
                "OR tx.pay_receiver_address = :pay_receiver_address "
                "OR tx.pay_close_address = :pay_close_address",
                soci::use(address), soci::use(address), soci::use(address));

        return query(rows);
    }

    std::vector<model::Transaction> TransactionDatabaseHelper::query(const soci::rowset<soci::row>& rows)
    {
        auto transactions = std::vector<model::Transaction>();
        for (const auto& row : rows) {
            const auto tx = [&row]() {
                auto tx = model::Transaction();
                inflateTransaction(row, tx);
                return tx;
            }();
            transactions.push_back(tx);
        }
        return transactions;
    }

} // namespace algorand
} // namespace core
} // namespace ledger
