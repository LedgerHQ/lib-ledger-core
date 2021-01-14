/*
 *
 * AlgorandOperationsDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Habib LAFET on 12/01/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Ledger
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

#include "AlgorandOperationsDatabaseHelper.hpp"
#include <database/PreparedStatement.hpp>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <api/BigInt.hpp>
#include <crypto/SHA256.hpp>
#include <wallet/algorand/database/AlgorandTransactionDatabaseHelper.hpp>
#include <wallet/algorand/model/transactions/AlgorandTransaction.hpp>
#include <unordered_set>
#include <database/soci-backend-utils.h>
#include <debug/Benchmarker.h>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>
#include <wallet/algorand/AlgorandExplorerConstants.hpp>

using namespace soci;

namespace {
    using namespace ledger::core::algorand;

    // Algorand operations
    struct AlgorandOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> txUid;
        std::vector<std::string> txHash;

        void update(const std::string& opUid, const std::string& transactionUid,
                const std::string& transactionHash) {
            uid.push_back(opUid);
            txUid.push_back(transactionUid);
            txHash.push_back(transactionHash);
        }

        void clear() {
            uid.clear();
            txUid.clear();
            txHash.clear();
        }
    };

    const auto UPSERT_ALGORAND_OPERATION = ledger::core::db::stmt<AlgorandOperationBinding>(
            "INSERT INTO algorand_operations VALUES(:uid, :tx_uid, :tx_hash) "
            "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, use(b.uid), use(b.txUid), use(b.txHash);
            });

    // Algorand transactions: generic class
    struct TransactionBinding {
        std::vector<std::string> uid;
        std::vector<boost::optional<std::string>> txHash;
        std::vector<std::string> type;
        std::vector<boost::optional<uint64_t>> round;
        std::vector<boost::optional<uint64_t>> timestamp;
        std::vector<uint64_t> firstValid;
        std::vector<uint64_t> lastValid;
        std::vector<boost::optional<std::string>> genesisId;
        std::vector<std::string> genesisHash;
        std::vector<std::string> sender;
        std::vector<uint64_t> fee;
        std::vector<boost::optional<std::string>> note;
        std::vector<boost::optional<std::string>> group;
        std::vector<boost::optional<std::string>> lease;
        std::vector<boost::optional<uint64_t>> senderRewards;
        std::vector<boost::optional<uint64_t>> receiverRewards;
        std::vector<boost::optional<uint64_t>> closeRewards;

        void update(const std::string& txUid, const model::Transaction & tx) {
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
            
            uid.push_back(txUid);
            txHash.push_back(headerId);
            type.push_back(tx.header.type);
            round.push_back(headerRound);
            timestamp.push_back(headerTimestamp);
            firstValid.push_back(tx.header.firstValid);
            lastValid.push_back(tx.header.lastValid);
            genesisId.push_back(headerGenesisId);
            genesisHash.push_back(tx.header.genesisHash.getRawString());
            sender.push_back(tx.header.sender.toString());
            fee.push_back(tx.header.fee);
            note.push_back(headerNote);
            group.push_back(headerGroup);
            lease.push_back(headerLease);
            senderRewards.push_back(headerSenderRewards);
            receiverRewards.push_back(headerReceiverRewards);
            closeRewards.push_back(headerCloseRewards);
        }

        void clear() {
            uid.clear();
            txHash.clear();
            type.clear();
            round.clear();
            timestamp.clear();
            firstValid.clear();
            lastValid.clear();
            genesisId.clear();
            genesisHash.clear();
            sender.clear();
            fee.clear();
            note.clear();
            group.clear();
            lease.clear();
            senderRewards.clear();
            receiverRewards.clear();
            closeRewards.clear();
        }

    };

    // Algorand transactions: payment
    struct PaymentTransactionBinding : public TransactionBinding {
        std::vector<uint64_t> amount;
        std::vector<std::string> receiverAddr;
        std::vector<boost::optional<std::string>> closeAddr;
        std::vector<boost::optional<uint64_t>> closeAmount;

        void update(const std::string& txUid, const model::Transaction & tx) {
            TransactionBinding::update(txUid, tx);
            auto& payment = boost::get<model::PaymentTxnFields>(tx.details);
            auto paymentCloseAddr = optionalValueWithTransform<Address, std::string>(payment.closeAddr, addrToString);
            auto paymentCloseAmount = optionalValue<uint64_t>(payment.closeAmount);

            amount.push_back(payment.amount);
            receiverAddr.push_back(payment.receiverAddr.toString());
            closeAddr.push_back(paymentCloseAddr);
            closeAmount.push_back(paymentCloseAmount);
        }

        void clear() {
            TransactionBinding::clear();
            amount.clear();
            receiverAddr.clear();
            closeAddr.clear();
            closeAmount.clear();
        }

    };
 
    const auto UPSERT_ALGORAND_TRANSACTION_PAYMENT = ledger::core::db::stmt<PaymentTransactionBinding>(
            "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "pay_amount, pay_receiver_address, pay_close_address, pay_close_amount) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":amount, :receiver_addr, :close_addr, :close_amount) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                use(b.uid),
                use(b.txHash),
                use(b.type),
                use(b.round),
                use(b.timestamp),
                use(b.firstValid),
                use(b.lastValid),
                use(b.genesisId),
                use(b.genesisHash),
                use(b.sender),
                use(b.fee),
                use(b.note),
                use(b.group),
                use(b.lease),
                use(b.senderRewards),
                use(b.receiverRewards),
                use(b.closeRewards),
                use(b.amount),
                use(b.receiverAddr),
                use(b.closeAddr),
                use(b.closeAmount);
            });


    // Algorand transactions: keyreg
    struct KeyregTransactionBinding : public TransactionBinding {

        std::vector<boost::optional<int32_t>> nonParticipation;
        std::vector<std::string> selectionPk;
        std::vector<std::string> votePk;
        std::vector<uint64_t> voteKeyDilution;
        std::vector<uint64_t> voteFirst;
        std::vector<uint64_t> voteLast;

        void update(const std::string& txUid, const model::Transaction & tx) {
            TransactionBinding::update(txUid, tx);
            
            auto& keyreg = boost::get<model::KeyRegTxnFields>(tx.details);
            auto keyreg_nonParticipation = optionalValueWithTransform<bool, int32_t>(keyreg.nonParticipation, boolToNum);

            nonParticipation.push_back(keyreg_nonParticipation);
            selectionPk.push_back(keyreg.selectionPk);
            votePk.push_back(keyreg.votePk);
            voteKeyDilution.push_back(keyreg.voteKeyDilution);
            voteFirst.push_back(keyreg.voteFirst);
            voteLast.push_back(keyreg.voteLast);
        }

        void clear() {
            TransactionBinding::clear();
            nonParticipation.clear();
            selectionPk.clear();
            votePk.clear();
            voteKeyDilution.clear();
            voteFirst.clear();
            voteLast.clear();
        }
    };

        const auto UPSERT_ALGORAND_TRANSACTION_KEYREG = ledger::core::db::stmt<KeyregTransactionBinding>(
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "keyreg_non_participation, keyreg_selection_pk, keyreg_vote_pk, keyreg_vote_key_dilution, keyreg_vote_first, keyreg_vote_last) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":non_part, :selection_pk, :vote_pk, :vote_key_dilution, :vote_first, :vote_last) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                use(b.uid),
                use(b.txHash),
                use(b.type),
                use(b.round),
                use(b.timestamp),
                use(b.firstValid),
                use(b.lastValid),
                use(b.genesisId),
                use(b.genesisHash),
                use(b.sender),
                use(b.fee),
                use(b.note),
                use(b.group),
                use(b.lease),
                use(b.senderRewards),
                use(b.receiverRewards),
                use(b.closeRewards),
                use(b.nonParticipation),
                use(b.selectionPk),
                use(b.votePk),
                use(b.voteKeyDilution),
                use(b.voteFirst),
                use(b.voteLast);
            });
 
    // Algorand transactions: AssetConfig
    struct AssetConfigTransactionBinding : public TransactionBinding {

        std::vector<boost::optional<uint64_t>> assetConfigId;
        std::vector<boost::optional<std::string>> assetConfigName;
        std::vector<boost::optional<std::string>> assetConfigUnitname;
        std::vector<boost::optional<uint64_t>> assetConfigTotal;
        std::vector<boost::optional<uint32_t>> assetConfigDecimals;
        std::vector<boost::optional<int32_t>> assetConfigFrozen;
        std::vector<boost::optional<std::string>> assetConfigCreator;
        std::vector<boost::optional<std::string>> assetConfigManager;
        std::vector<boost::optional<std::string>> assetConfigReserve;
        std::vector<boost::optional<std::string>> assetConfigFreeze;
        std::vector<boost::optional<std::string>> assetConfigClawback;
        std::vector<boost::optional<std::string>> assetConfigMetadata;
        std::vector<boost::optional<std::string>> assetConfigUrl;

        void update(const std::string& txUid, const model::Transaction & tx) {
            TransactionBinding::update(txUid, tx);

            auto& assetConfig = boost::get<model::AssetConfigTxnFields>(tx.details);
            auto& assetParams = *assetConfig.assetParams;
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

            assetConfigId.push_back(assetId);
            assetConfigName.push_back(assetName);
            assetConfigUnitname.push_back(assetUnitname);
            assetConfigTotal.push_back(assetTotal);
            assetConfigDecimals.push_back(assetDecimals);
            assetConfigFrozen.push_back(assetFrozen);
            assetConfigCreator.push_back(assetCreator);
            assetConfigManager.push_back(assetManager);
            assetConfigReserve.push_back(assetReserve);
            assetConfigFreeze.push_back(assetFreeze);
            assetConfigClawback.push_back(assetClawback);
            assetConfigMetadata.push_back(assetMetadata);
            assetConfigUrl.push_back(assetUrl);
        }

        void clear() {
            TransactionBinding::clear();
            assetConfigId.clear();
            assetConfigName.clear();
            assetConfigUnitname.clear();
            assetConfigTotal.clear();
            assetConfigDecimals.clear();
            assetConfigFrozen.clear();
            assetConfigCreator.clear();
            assetConfigManager.clear();
            assetConfigReserve.clear();
            assetConfigFreeze.clear();
            assetConfigClawback.clear();
            assetConfigMetadata.clear();
            assetConfigUrl.clear();
        }
    };

    const auto UPSERT_ALGORAND_TRANSACTION_ASSETCONFIG = ledger::core::db::stmt<AssetConfigTransactionBinding>(
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "acfg_asset_id, acfg_asset_name, acfg_unit_name, acfg_total, acfg_decimals, acfg_default_frozen, "
                "acfg_creator_address, acfg_manager_address, acfg_reserve_address, acfg_freeze_address, acfg_clawback_address, "
                "acfg_metadata_hash, acfg_url) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":asset_id, :asset_name, :unit_name, :total, :decimals, :default_frozen, :creator_addr, "
                ":manager_addr, :reserve_addr, :freeze_addr, :clawback_addr, :metadata_hash, :url) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                use(b.uid),
                use(b.txHash),
                use(b.type),
                use(b.round),
                use(b.timestamp),
                use(b.firstValid),
                use(b.lastValid),
                use(b.genesisId),
                use(b.genesisHash),
                use(b.sender),
                use(b.fee),
                use(b.note),
                use(b.group),
                use(b.lease),
                use(b.senderRewards),
                use(b.receiverRewards),
                use(b.closeRewards),
                use(b.assetConfigId),
                use(b.assetConfigName),
                use(b.assetConfigUnitname),
                use(b.assetConfigTotal),
                use(b.assetConfigDecimals),
                use(b.assetConfigFrozen),
                use(b.assetConfigCreator),
                use(b.assetConfigManager),
                use(b.assetConfigReserve),
                use(b.assetConfigFreeze),
                use(b.assetConfigClawback),
                use(b.assetConfigMetadata),
                use(b.assetConfigUrl);
            });


    // Algorand transactions: AssetTransfer
    struct AssetTransferTransactionBinding : public TransactionBinding {
       
            std::vector<uint64_t> assetTransferId;
            std::vector<boost::optional<uint64_t>> assetTransferAmount;
            std::vector<std::string> assetTransferReceiver;
            std::vector<boost::optional<std::string>> assetTransferCloseTo;
            std::vector<boost::optional<uint64_t>> assetTransferCloseAmount;
            std::vector<boost::optional<std::string>> assetTransferSender;

        void update(const std::string& txUid, const model::Transaction & tx) {
            TransactionBinding::update(txUid, tx);
            auto& assetTransfer = boost::get<model::AssetTransferTxnFields>(tx.details);
            auto assetAmount = optionalValue<uint64_t>(assetTransfer.assetAmount);
            auto assetCloseTo = optionalValueWithTransform<Address, std::string>(assetTransfer.assetCloseTo, addrToString);
            auto assetCloseAmount = optionalValue(assetTransfer.closeAmount);
            auto assetSender = optionalValueWithTransform<Address, std::string>(assetTransfer.assetSender, addrToString);

            assetTransferId.push_back(assetTransfer.assetId);
            assetTransferAmount.push_back(assetAmount);
            assetTransferReceiver.push_back(assetTransfer.assetReceiver.toString());
            assetTransferCloseTo.push_back(assetCloseTo);
            assetTransferCloseAmount.push_back(assetCloseAmount);
            assetTransferSender.push_back(assetSender);
        }

        void clear() {
            TransactionBinding::clear();
            assetTransferId.clear();
            assetTransferAmount.clear();
            assetTransferReceiver.clear();
            assetTransferCloseTo.clear();
            assetTransferCloseAmount.clear();
            assetTransferSender.clear();
        }
    };

    const auto UPSERT_ALGORAND_TRANSACTION_ASSETTRANSFER = ledger::core::db::stmt<AssetTransferTransactionBinding>(
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "axfer_asset_id, axfer_asset_amount, axfer_receiver_address, axfer_close_address, axfer_close_amount, axfer_sender_address) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":assetId, :amount, :receiver_addr, :close_addr, :close_amount, :sender_addr) "
        "ON CONFLICT DO NOTHING",
            [] (auto& s, auto&  b) {
                s, 
                use(b.uid),
                use(b.txHash),
                use(b.type),
                use(b.round),
                use(b.timestamp),
                use(b.firstValid),
                use(b.lastValid),
                use(b.genesisId),
                use(b.genesisHash),
                use(b.sender),
                use(b.fee),
                use(b.note),
                use(b.group),
                use(b.lease),
                use(b.senderRewards),
                use(b.receiverRewards),
                use(b.closeRewards),
                use(b.assetTransferId),
                use(b.assetTransferAmount),
                use(b.assetTransferReceiver),
                use(b.assetTransferCloseTo),
                use(b.assetTransferCloseAmount),
                use(b.assetTransferSender);
            });



// Algorand transactions: AssetFreeze
    struct AssetFreezeTransactionBinding : public TransactionBinding {
       
        std::vector<uint64_t> assetFreezeId;
        std::vector<int32_t> assetFreezeFrozen;
        std::vector<std::string> assetFreezeAddres;

        void update(const std::string& txUid, const model::Transaction & tx) {
            TransactionBinding::update(txUid, tx);

            auto& assetFreeze = boost::get<model::AssetFreezeTxnFields>(tx.details);
            assetFreezeId.push_back(assetFreeze.assetId);
            assetFreezeFrozen.push_back(static_cast<int32_t>(assetFreeze.assetFrozen));
            assetFreezeAddres.push_back(assetFreeze.frozenAddress.toString());
        }

        void clear() {
            TransactionBinding::clear();

            assetFreezeId.clear();
            assetFreezeFrozen.clear();
            assetFreezeAddres.clear();
        }
    };

    const auto UPSERT_ALGORAND_TRANSACTION_ASSETFREEZE = ledger::core::db::stmt<AssetFreezeTransactionBinding>(
        "INSERT INTO algorand_transactions ("
                "uid, hash, type, round, timestamp, first_valid, last_valid, genesis_id, genesis_hash, "
                "sender, fee, note, groupVal, leaseVal, sender_rewards, receiver_rewards, close_rewards, "
                "afrz_asset_id, afrz_frozen, afrz_frozen_address) "
        "VALUES(:tx_uid, :hash, :tx_type, :round, :timestamp, :first_valid, :last_valid, :genesis_id, :genesis_hash, "
                ":sender, :fee, :note, :group, :lease, :sender_rewards, :receiver_rewards, :close_rewards, "
                ":asset_id, :frozen, :frozen_addr) "
        "ON CONFLICT DO NOTHING",    
            [] (auto& s, auto&  b) {
                s, 
                use(b.uid),
                use(b.txHash),
                use(b.type),
                use(b.round),
                use(b.timestamp),
                use(b.firstValid),
                use(b.lastValid),
                use(b.genesisId),
                use(b.genesisHash),
                use(b.sender),
                use(b.fee),
                use(b.note),
                use(b.group),
                use(b.lease),
                use(b.senderRewards),
                use(b.receiverRewards),
                use(b.closeRewards),
                use(b.assetFreezeId),
                use(b.assetFreezeFrozen),
                use(b.assetFreezeAddres);
            });
}

namespace ledger {
    namespace core {
        namespace algorand {

            void OperationsDatabaseHelper::bulkInsert(soci::session &sql,
                    const std::vector<Operation> &operations) {
                if (operations.empty())
                    return;
                Benchmarker rawInsert("raw_db_insert_algorand", nullptr);
                rawInsert.start();
                PreparedStatement<OperationBinding> operationStmt;
                PreparedStatement<BlockBinding> blockStmt;
                PreparedStatement<AlgorandOperationBinding> algorandOpStmt;
                PreparedStatement<PaymentTransactionBinding> paymentTransactionStmt;
                PreparedStatement<KeyregTransactionBinding> keyregTransactionStmt;
                PreparedStatement<AssetConfigTransactionBinding> assetConfigTransactionStmt;
                PreparedStatement<AssetTransferTransactionBinding> assetTransferTransactionStmt;
                PreparedStatement<AssetFreezeTransactionBinding> assetFreezeTransactionStmt;

                BulkInsertDatabaseHelper::UPSERT_OPERATION(sql, operationStmt);
                BulkInsertDatabaseHelper::UPSERT_BLOCK(sql, blockStmt);
                UPSERT_ALGORAND_OPERATION(sql, algorandOpStmt);     
                UPSERT_ALGORAND_TRANSACTION_PAYMENT(sql, paymentTransactionStmt);
                UPSERT_ALGORAND_TRANSACTION_KEYREG(sql, keyregTransactionStmt);
                UPSERT_ALGORAND_TRANSACTION_ASSETCONFIG(sql, assetConfigTransactionStmt);
                UPSERT_ALGORAND_TRANSACTION_ASSETTRANSFER(sql, assetTransferTransactionStmt);
                UPSERT_ALGORAND_TRANSACTION_ASSETFREEZE(sql, assetFreezeTransactionStmt);

                for (const auto& op : operations) {
                    if (op.getBackend().block.hasValue()) {
                        blockStmt.bindings.update(op.getBackend().block.getValue());
                    }
                    // Upsert operation
                    operationStmt.bindings.update(op.getBackend());

                    // Upsert transaction
                    auto& tx = op.getTransactionData(); 
                    const auto txUid = *tx.header.id;
                    if (tx.header.type == constants::xPay) {
                        paymentTransactionStmt.bindings.update(txUid, tx);
                    } else if (tx.header.type == constants::xKeyregs) {
                        keyregTransactionStmt.bindings.update(txUid, tx);
                    } else if (tx.header.type == constants::xAcfg) {
                        assetConfigTransactionStmt.bindings.update(txUid, tx);
                    } else if (tx.header.type == constants::xAxfer) {
                        assetTransferTransactionStmt.bindings.update(txUid, tx);
                    } else if (tx.header.type == constants::xAfreeze) {
                        assetFreezeTransactionStmt.bindings.update(txUid, tx);
                    }

                    // Algorand operation
                    const auto txHash = op.getTransaction()->getId();
                    algorandOpStmt.bindings.update(op.getBackend().uid, txUid, txHash);  
                }
                //1- block
                if (!blockStmt.bindings.uid.empty())
                    blockStmt.execute();

                //2- operations 
                operationStmt.execute();

                //3- algorand transactions 
                if (!paymentTransactionStmt.bindings.uid.empty())
                    paymentTransactionStmt.execute();
                if (!keyregTransactionStmt.bindings.uid.empty())
                    keyregTransactionStmt.execute();
                if (!assetConfigTransactionStmt.bindings.uid.empty())
                    assetConfigTransactionStmt.execute();
                if (!assetTransferTransactionStmt.bindings.uid.empty())
                    assetTransferTransactionStmt.execute();
                if (!assetFreezeTransactionStmt.bindings.uid.empty())
                    assetFreezeTransactionStmt.execute();

                //4- algorand operations
                algorandOpStmt.execute();

                rawInsert.stop();
            }
        }
    }
}