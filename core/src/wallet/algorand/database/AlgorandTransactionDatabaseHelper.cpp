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
#include <fmt/format.h>

namespace ledger {
namespace core {
namespace algorand {

    static void putPaymentTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& payment = boost::get<model::PaymentTxnFields>(tx.details);

        auto headerId = optionalValue<std::string>(tx.header.id);
        auto headerGenesisId = optionalValue<std::string>(tx.header.genesisId);
        auto headerRound = optionalValue<uint64_t>(tx.header.round);
        auto headerTimestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto headerNote = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto headerGroup = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto headerLease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto headerSenderRewards = optionalValue(tx.header.senderRewards);
        auto headerReceiverRewards = optionalValue(tx.header.receiverRewards);
        auto headerCloseRewards = optionalValue(tx.header.closeRewards);
        auto paymentCloseAddr = optionalValueWithTransform<Address, std::string>(payment.closeAddr, addrToString);
        auto paymentCloseAmount = optionalValue<uint64_t>(payment.closeAmount);

        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "pay_amount, pay_receiver_address, pay_close_address, pay_close_amount) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":amount, :receiver_addr, :close_addr, :close_amount)",
            soci::use(txUid),
            soci::use(headerId),
            soci::use(tx.header.type),
            soci::use(headerRound),
            soci::use(headerTimestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(headerGenesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(headerNote),
            soci::use(headerGroup),
            soci::use(headerLease),
            soci::use(headerSenderRewards),
            soci::use(headerReceiverRewards),
            soci::use(headerCloseRewards),
            soci::use(payment.amount),
            soci::use(payment.receiverAddr.toString()),
            soci::use(paymentCloseAddr),
            soci::use(paymentCloseAmount);
    }

    // NOTE This has not been tested
    static void putKeyRegTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& keyreg = boost::get<model::KeyRegTxnFields>(tx.details);
        auto headerId = optionalValue<std::string>(tx.header.id);
        auto headerGenesisId = optionalValue<std::string>(tx.header.genesisId);
        auto headerRound = optionalValue<uint64_t>(tx.header.round);
        auto headerTimestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto headerNote = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto headerGroup = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto headerLease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto headerSenderRewards = optionalValue(tx.header.senderRewards);
        auto headerReceiverRewards = optionalValue(tx.header.receiverRewards);
        auto headerCloseRewards = optionalValue(tx.header.closeRewards);
        auto keyreg_nonParticipation = optionalValueWithTransform<bool, int32_t>(keyreg.nonParticipation, boolToNum);

        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "keyreg_non_participation, keyreg_selection_pk, keyreg_vote_pk, keyreg_vote_key_dilution, keyreg_vote_first, keyreg_vote_last) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":non_part, :selection_pk, :vote_pk, :vote_key_dilution, :vote_first, :vote_last)",
            soci::use(txUid),
            soci::use(headerId),
            soci::use(tx.header.type),
            soci::use(headerRound),
            soci::use(headerTimestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(headerGenesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(headerNote),
            soci::use(headerGroup),
            soci::use(headerLease),
            soci::use(headerSenderRewards),
            soci::use(headerReceiverRewards),
            soci::use(headerCloseRewards),
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
        auto headerId = optionalValue<std::string>(tx.header.id);
        auto headerGenesisId = optionalValue<std::string>(tx.header.genesisId);
        auto headerRound = optionalValue<uint64_t>(tx.header.round);
        auto headerTimestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto headerNote = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto headerGroup = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto headerLease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto headerSenderRewards = optionalValue(tx.header.senderRewards);
        auto headerReceiverRewards = optionalValue(tx.header.receiverRewards);
        auto headerCloseRewards = optionalValue(tx.header.closeRewards);
        auto assetId = optionalValue<uint64_t>(assetConfig.assetId);
        auto assetName = optionalValue<std::string>(assetParams.assetName);
        auto assetUnitname = optionalValue<std::string>(assetParams.unitName);
        auto assetTotal = optionalValue<uint64_t>(assetParams.total);
        auto assetDecimals = optionalValue<uint32_t>(assetParams.decimals);
        auto assetFrozen = optionalValueWithTransform<bool, int32_t>(assetParams.defaultFrozen, boolToNum);
        auto assetCreator = optionalValueWithTransform<Address, std::string>(assetParams.creatorAddr, addrToString);
        auto assetManager = optionalValueWithTransform<Address, std::string>(assetParams.managerAddr, addrToString);
        auto assetReserve = optionalValueWithTransform<Address, std::string>(assetParams.reserveAddr, addrToString);
        auto assetFreeze = optionalValueWithTransform<Address, std::string>(assetParams.freezeAddr, addrToString);
        auto assetClawback = optionalValueWithTransform<Address, std::string>(assetParams.clawbackAddr, addrToString);
        auto assetMetadata = optionalValueWithTransform<std::vector<uint8_t>, std::string>(assetParams.metaDataHash, bytesToB64);
        auto assetUrl = optionalValue<std::string>(assetParams.url);


        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "acfg_asset_id, acfg_asset_name, acfg_unit_name, acfg_total, acfg_decimals, acfg_default_frozen, "
                "acfg_creator_address, acfg_manager_address, acfg_reserve_address, acfg_freeze_address, acfg_clawback_address, "
                "acfg_metadata_hash, acfg_url) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":asset_id, :asset_name, :unit_name, :total, :decimals, :default_frozen, :creator_addr, "
                ":manager_addr, :reserve_addr, :freeze_addr, :clawback_addr, :metadata_hash, :url)",
            soci::use(txUid),
            soci::use(headerId),
            soci::use(tx.header.type),
            soci::use(headerRound),
            soci::use(headerTimestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(headerGenesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(headerNote),
            soci::use(headerGroup),
            soci::use(headerLease),
            soci::use(headerSenderRewards),
            soci::use(headerReceiverRewards),
            soci::use(headerCloseRewards),
            soci::use(assetId),
            soci::use(assetName),
            soci::use(assetUnitname),
            soci::use(assetTotal),
            soci::use(assetDecimals),
            soci::use(assetFrozen),
            soci::use(assetCreator),
            soci::use(assetManager),
            soci::use(assetReserve),
            soci::use(assetFreeze),
            soci::use(assetClawback),
            soci::use(assetMetadata),
            soci::use(assetUrl);
    }

    static void putAssetTransferTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& assetTransfer = boost::get<model::AssetTransferTxnFields>(tx.details);
        auto headerId = optionalValue<std::string>(tx.header.id);
        auto headerGenesisId = optionalValue<std::string>(tx.header.genesisId);
        auto headerRound = optionalValue<uint64_t>(tx.header.round);
        auto headerTimestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto headerNote = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto headerGroup = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto headerLease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto headerSenderRewards = optionalValue(tx.header.senderRewards);
        auto headerReceiverRewards = optionalValue(tx.header.receiverRewards);
        auto headerCloseRewards = optionalValue(tx.header.closeRewards);
        auto assetAmount = optionalValue<uint64_t>(assetTransfer.assetAmount);
        auto assetCloseTo = optionalValueWithTransform<Address, std::string>(assetTransfer.assetCloseTo, addrToString);
        auto assetCloseAmount = optionalValue(assetTransfer.closeAmount);
        auto assetSender = optionalValueWithTransform<Address, std::string>(assetTransfer.assetSender, addrToString);

        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "axfer_asset_id, axfer_asset_amount, axfer_receiver_address, axfer_close_address, axfer_close_amount, axfer_sender_address) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":assetId, :amount, :receiver_addr, :close_addr, :close_amount, :sender_addr)",
            soci::use(txUid),
            soci::use(headerId),
            soci::use(tx.header.type),
            soci::use(headerRound),
            soci::use(headerTimestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(headerGenesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(headerNote),
            soci::use(headerGroup),
            soci::use(headerLease),
            soci::use(headerSenderRewards),
            soci::use(headerReceiverRewards),
            soci::use(headerCloseRewards),
            soci::use(assetTransfer.assetId),
            soci::use(assetAmount),
            soci::use(assetTransfer.assetReceiver.toString()),
            soci::use(assetCloseTo),
            soci::use(assetCloseAmount),
            soci::use(assetSender);
    }

    // NOTE This has not been tested
    static void putAssetFreezeTransaction(soci::session & sql, const std::string & txUid, const model::Transaction & tx) {
        auto& assetFreeze = boost::get<model::AssetFreezeTxnFields>(tx.details);
        auto headerId = optionalValue<std::string>(tx.header.id);
        auto headerGenesisId = optionalValue<std::string>(tx.header.genesisId);
        auto headerRound = optionalValue<uint64_t>(tx.header.round);
        auto headerTimestamp = optionalValue<uint64_t>(tx.header.timestamp);
        auto headerNote = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.note, bytesToB64);
        auto headerGroup = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.group, bytesToB64);
        auto headerLease = optionalValueWithTransform<std::vector<uint8_t>, std::string>(tx.header.lease, bytesToB64);
        auto headerSenderRewards = optionalValue(tx.header.senderRewards);
        auto headerReceiverRewards = optionalValue(tx.header.receiverRewards);
        auto headerCloseRewards = optionalValue(tx.header.closeRewards);

        sql <<
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "afrz_asset_id, afrz_frozen, afrz_frozen_address) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":asset_id, :frozen, :frozen_addr)",
            soci::use(txUid),
            soci::use(headerId),
            soci::use(tx.header.type),
            soci::use(headerRound),
            soci::use(headerTimestamp),
            soci::use(tx.header.firstValid),
            soci::use(tx.header.lastValid),
            soci::use(headerGenesisId),
            soci::use(tx.header.genesisHash.getRawString()),
            soci::use(tx.header.sender.toString()),
            soci::use(tx.header.fee),
            soci::use(headerNote),
            soci::use(headerGroup),
            soci::use(headerLease),
            soci::use(headerSenderRewards),
            soci::use(headerReceiverRewards),
            soci::use(headerCloseRewards),
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
        tx.header.sender = Address(getString(row, COL_TX_SENDER));
        tx.header.fee = getNumber(row, COL_TX_FEE);
        tx.header.note = getOptionalStringWithTransform<std::vector<uint8_t>>(row, COL_TX_NOTE, b64toBytes);
        tx.header.group = getOptionalStringWithTransform<std::vector<uint8_t>>(row, COL_TX_GROUP, b64toBytes);
        tx.header.lease = getOptionalStringWithTransform<std::vector<uint8_t>>(row, COL_TX_LEASE, b64toBytes);
        tx.header.senderRewards = getOptionalNumber(row, COL_TX_SENDER_REWARDS);
        tx.header.receiverRewards = getOptionalNumber(row, COL_TX_RECEIVER_REWARDS);
        tx.header.closeRewards = getOptionalNumber(row, COL_TX_CLOSE_REWARDS);

        if (tx.header.type == constants::xPay) {

            model::PaymentTxnFields paymentDetails;

            paymentDetails.amount = getNumber(row, COL_TX_PAY_AMOUNT);
            paymentDetails.receiverAddr = Address(getString(row, COL_TX_PAY_RECEIVER_ADDRESS));
            paymentDetails.closeAddr = getOptionalStringWithTransform<Address>(row, COL_TX_PAY_CLOSE_ADDRESS, stringToAddr);
            paymentDetails.closeAmount = getOptionalNumber(row, COL_TX_PAY_CLOSE_AMOUNT);

            tx.details = paymentDetails;

        } else if (tx.header.type == constants::xKeyregs) {
            // NOTE This has not been tested

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
            assetTransferDetails.closeAmount = getOptionalNumber(row, COL_TX_AXFER_CLOSE_AMOUNT);
            assetTransferDetails.assetSender = getOptionalStringWithTransform<Address>(row, COL_TX_AXFER_SENDER_ADDRESS, stringToAddr);

            tx.details = assetTransferDetails;

        } else if (tx.header.type == constants::xAfreeze) {
            // NOTE This has not been tested

            model::AssetFreezeTxnFields assetFreezeDetails;

            assetFreezeDetails.assetId = getNumber(row, COL_TX_AFRZ_ASSET_ID);
            assetFreezeDetails.assetFrozen = !! getNumber(row, COL_TX_AFRZ_FROZEN);
            assetFreezeDetails.frozenAddress = Address(getString(row, COL_TX_AFRZ_FROZEN_ADDRESS));

            tx.details = assetFreezeDetails;
        }
    }

    bool TransactionDatabaseHelper::transactionExists(soci::session & sql, const std::string & txUid) {
        int32_t count = 0;
        sql << "SELECT COUNT(*) FROM algorand_transactions WHERE uid = :txUid", soci::use(txUid), soci::into(count);
        if (count > 1) {
            throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, "More than one transaction found with uid '{}'.", txUid);
        }
        return count == 1;
    }

    bool TransactionDatabaseHelper::getTransactionByHash(soci::session & sql,
                                                         const std::string & hash,
                                                         model::Transaction & tx) {

        int32_t count = 0;
        sql << "SELECT COUNT(*) FROM algorand_transactions WHERE hash = :hash", soci::use(hash), soci::into(count);
        if (count > 1) {
            throw make_exception(api::ErrorCode::DATABASE_EXCEPTION, "More than one transaction found with hash '{}'.", hash);
        }

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

    std::string TransactionDatabaseHelper::putTransaction(soci::session & sql,
                                                          const std::string & accountUid,
                                                          const model::Transaction & tx) {
        const auto txUid = *tx.header.id;

        if (!transactionExists(sql, txUid)) {
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
        }

        return txUid;
    }

    void TransactionDatabaseHelper::deleteAllTransactions(soci::session& sql)
    {
        sql<<"DELETE FROM algorand_transactions";
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
                "OR tx.sender = :sender) "
                "ORDER BY timestamp",
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
                "OR tx.pay_close_address = :pay_close_address "
                "ORDER BY timestamp",
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

