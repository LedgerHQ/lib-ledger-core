/*
 * AlgorandTestFixtures
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
#ifndef LEDGER_TEST_ALGORANDTESTFIXTURES_H
#define LEDGER_TEST_ALGORANDTESTFIXTURES_H

#include <algorand/model/AlgorandAccount.hpp>
#include <algorand/model/transactions/AlgorandTransaction.hpp>
#include <algorand/model/transactions/AlgorandAssetParams.hpp>
#include <algorand/AlgorandNetworks.hpp>
#include <algorand/AlgorandJsonParser.hpp>

#include <core/math/BaseConverter.hpp>
#include <core/utils/Option.hpp>

#include <gtest/gtest.h>

using namespace ledger::core::algorand;

namespace ledger {
namespace testing {
namespace algorand {

    using ledger::core::BaseConverter;
    using ledger::core::Option;

    static const std::string OBELIX_ADDRESS = "RGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL56XZM";
    static const std::string TEST_ACCOUNT_ADDRESS = "6ENXFMQRRIF6KD7HXE47HUHCJXEUKGGRGR6LXSX7RRZBTMVI5NUDOQDTNE";

    static const std::string PAYMENT_TX_ID = "BGGXSAI5IOZXOVXCVR3ASC2RMYBLSR33XTOYL3M2FCLAS6QYBHRA";
    static const std::string ASSET_CONFIG_TX_ID = "GYF4N5DXRANS6AEJL4HZD53FHYSGL6AVYKLR4PG5DBSJJASWXD7Q";
    static const std::string ASSET_TRANSFER_TX_ID = "P3TBX7WYKO5SIZIHV2Z3GUCOD7B556RF6PSOPINFB6QN7EMTF6JQ";

    static const std::string TESTNET_GENESIS_ID = ledger::core::networks::getAlgorandNetworkParameters("algorand-testnet").genesisID;
    static const B64String   TESTNET_GENESIS_HASH(ledger::core::networks::getAlgorandNetworkParameters("algorand-testnet").genesisHash);

    static const model::AssetParams testAsset() {
        static model::AssetParams asset;

        if (asset.assetName.isEmpty()) {
            asset.assetName = "Obelix Coin";
            asset.unitName = "OBC";
            asset.total = 1000000;
            asset.decimals = 3;
            asset.defaultFrozen = false;
            auto obelixAddress = Address(OBELIX_ADDRESS);
            asset.creatorAddr = obelixAddress;
            asset.managerAddr = obelixAddress;
            asset.reserveAddr = obelixAddress;
            asset.freezeAddr = obelixAddress;
            asset.clawbackAddr = obelixAddress;
        }

        return asset;
    }

    static const model::Transaction paymentTransaction() {
        static model::Transaction tx;

        if (tx.header.id.isEmpty()) {
            tx.header.id = PAYMENT_TX_ID;
            tx.header.fee = 1000;
            tx.header.sender = Address(OBELIX_ADDRESS);
            tx.header.type = constants::xPay;
            tx.header.round = 6529864;
            tx.header.note = std::vector<uint8_t>();
            BaseConverter::decode("nI32ynz7r34=", BaseConverter::BASE64_RFC4648, *tx.header.note);
            tx.header.genesisId = TESTNET_GENESIS_ID;
            tx.header.genesisHash = TESTNET_GENESIS_HASH;
            tx.header.firstValid = 6529846;
            tx.header.lastValid = 6530846;
            tx.header.fromRewards = 0UL;
            tx.header.timestamp = 1588586190;

            tx.details = model::PaymentTxnFields();
            auto& details = boost::get<model::PaymentTxnFields>(tx.details);
            details.receiverAddr = Address(TEST_ACCOUNT_ADDRESS);
            details.amount = 1000;
            details.receiverRewards = 0UL;
            details.closeRewards = 0UL;
        }

        return tx;
    }

    static const model::Transaction assetConfigTransaction() {
        static model::Transaction tx;

        if (tx.header.id.isEmpty()) {
            tx.header.id = ASSET_CONFIG_TX_ID;
            tx.header.fee = 1000;
            tx.header.sender = Address(OBELIX_ADDRESS);
            tx.header.type = constants::xAcfg;
            tx.header.round = 6305889;
            tx.header.note = std::vector<uint8_t>();
            BaseConverter::decode("+72/UGvMiaA=", BaseConverter::BASE64_RFC4648, *tx.header.note);
            //tx.header.genesisId = TESTNET_GENESIS_ID; // genesis ID is left empty by the explorer for some reason
            tx.header.genesisHash = TESTNET_GENESIS_HASH;
            tx.header.firstValid = 6305874;
            tx.header.lastValid = 6306874;
            tx.header.fromRewards = 0UL;
            tx.header.timestamp = 1587641643;

            tx.details = model::AssetConfigTxnFields();
            auto& details = boost::get<model::AssetConfigTxnFields>(tx.details);
            details.assetId = 0UL;
            details.assetParams = model::AssetParams();
            auto& assetParams = *details.assetParams;
            assetParams.total = 1000000;
            assetParams.decimals = 3;
            assetParams.unitName = "DUM";
            assetParams.assetName = "Dummy Asset";
            assetParams.freezeAddr = OBELIX_ADDRESS;
            assetParams.managerAddr = OBELIX_ADDRESS;
            assetParams.reserveAddr = OBELIX_ADDRESS;
            assetParams.clawbackAddr = OBELIX_ADDRESS;
            assetParams.defaultFrozen = false;
        }

        return tx;
    }

    static const model::Transaction assetTransferTransaction() {
        static model::Transaction tx;

        if (tx.header.id.isEmpty()) {
            tx.header.id = ASSET_TRANSFER_TX_ID;
            tx.header.fee = 2000;
            tx.header.sender = Address(OBELIX_ADDRESS);
            tx.header.type = constants::xAxfer;
            tx.header.round = 6734108;
            tx.header.note = std::vector<uint8_t>();
            BaseConverter::decode("T2JlbGl4IGNvaW4gdGVzdCB0cmFuc2Zlcg==", BaseConverter::BASE64_RFC4648, *tx.header.note);
            tx.header.genesisId = TESTNET_GENESIS_ID;
            tx.header.genesisHash = TESTNET_GENESIS_HASH;
            tx.header.firstValid = 6734094;
            tx.header.lastValid = 6734193;
            tx.header.fromRewards = 0UL;
            tx.header.timestamp = 1589448692;

            tx.details = model::AssetTransferTxnFields();
            auto& details = boost::get<model::AssetTransferTxnFields>(tx.details);
            details.assetId = 342836;
            details.assetReceiver = Address(TEST_ACCOUNT_ADDRESS);
            details.assetAmount = 1000;
        }

        return tx;
    }

    static void assertSameAssetParams(const model::AssetParams & refAssetParams,
                               const model::AssetParams & resultAssetParams) {
        if (refAssetParams.metaDataHash.hasValue()) EXPECT_EQ(*refAssetParams.metaDataHash, *resultAssetParams.metaDataHash);
        if (refAssetParams.assetName.hasValue()) EXPECT_EQ(*refAssetParams.assetName, *resultAssetParams.assetName);
        if (refAssetParams.unitName.hasValue()) EXPECT_EQ(*refAssetParams.unitName, *resultAssetParams.unitName);
        if (refAssetParams.total.hasValue()) EXPECT_EQ(*refAssetParams.total, *resultAssetParams.total);
        if (refAssetParams.decimals.hasValue()) EXPECT_EQ(*refAssetParams.decimals, *resultAssetParams.decimals);
        if (refAssetParams.clawbackAddr.hasValue()) EXPECT_EQ(*refAssetParams.clawbackAddr, *resultAssetParams.clawbackAddr);
        if (refAssetParams.creatorAddr.hasValue()) EXPECT_EQ(*refAssetParams.creatorAddr, *resultAssetParams.creatorAddr);
        if (refAssetParams.freezeAddr.hasValue()) EXPECT_EQ(*refAssetParams.freezeAddr, *resultAssetParams.freezeAddr);
        if (refAssetParams.managerAddr.hasValue()) EXPECT_EQ(*refAssetParams.managerAddr, *resultAssetParams.managerAddr);
        if (refAssetParams.reserveAddr.hasValue()) EXPECT_EQ(*refAssetParams.reserveAddr, *resultAssetParams.reserveAddr);
        if (refAssetParams.defaultFrozen.hasValue()) EXPECT_EQ(*refAssetParams.defaultFrozen, *resultAssetParams.defaultFrozen);
        if (refAssetParams.url.hasValue()) EXPECT_EQ(*refAssetParams.url, *resultAssetParams.url);
    }

    static void assertSamePaymentDetails(const model::Transaction::Details & txRefDetails,
                                  const model::Transaction::Details & txResultDetails) {
        auto& txRefPaymentDetails = boost::get<model::PaymentTxnFields>(txRefDetails);
        auto& txResultPaymentDetails = boost::get<model::PaymentTxnFields>(txResultDetails);

        EXPECT_EQ(txRefPaymentDetails.amount, txResultPaymentDetails.amount);
        EXPECT_EQ(txRefPaymentDetails.receiverAddr, txResultPaymentDetails.receiverAddr);
        if (txRefPaymentDetails.closeAddr.hasValue()) EXPECT_EQ(*txRefPaymentDetails.closeAddr, *txResultPaymentDetails.closeAddr);
        if (txRefPaymentDetails.closeAmount.hasValue()) EXPECT_EQ(*txRefPaymentDetails.closeAmount, *txResultPaymentDetails.closeAmount);
        if (txRefPaymentDetails.closeRewards.hasValue()) EXPECT_EQ(*txRefPaymentDetails.closeRewards, *txResultPaymentDetails.closeRewards);
        if (txRefPaymentDetails.receiverRewards.hasValue()) EXPECT_EQ(*txRefPaymentDetails.receiverRewards, *txResultPaymentDetails.receiverRewards);
    }

    static void assertSameAssetConfigDetails(const model::Transaction::Details & txRefDetails,
                                      const model::Transaction::Details & txResultDetails) {
        auto& txRefAssetConfigDetails = boost::get<model::AssetConfigTxnFields>(txRefDetails);
        auto& txResultAssetConfigDetails = boost::get<model::AssetConfigTxnFields>(txResultDetails);

        if (txRefAssetConfigDetails.assetId.hasValue()) EXPECT_EQ(*txRefAssetConfigDetails.assetId, *txResultAssetConfigDetails.assetId);
        if (txRefAssetConfigDetails.assetParams.hasValue()) assertSameAssetParams(*txRefAssetConfigDetails.assetParams, *txResultAssetConfigDetails.assetParams);
    }

    static void assertSameAssetTransferDetails(const model::Transaction::Details & txRefDetails,
                                        const model::Transaction::Details & txResultDetails) {
        auto& txRefAssetTransferDetails = boost::get<model::AssetTransferTxnFields>(txRefDetails);
        auto& txResultAssetTransferDetails = boost::get<model::AssetTransferTxnFields>(txResultDetails);

        EXPECT_EQ(txRefAssetTransferDetails.assetId, txResultAssetTransferDetails.assetId);
        EXPECT_EQ(txRefAssetTransferDetails.assetReceiver, txResultAssetTransferDetails.assetReceiver);
        if (txRefAssetTransferDetails.assetAmount.hasValue()) EXPECT_EQ(*txRefAssetTransferDetails.assetAmount, *txResultAssetTransferDetails.assetAmount);
        if (txRefAssetTransferDetails.assetSender.hasValue()) EXPECT_EQ(*txRefAssetTransferDetails.assetSender, *txResultAssetTransferDetails.assetSender);
        if (txRefAssetTransferDetails.assetCloseTo.hasValue()) EXPECT_EQ(*txRefAssetTransferDetails.assetCloseTo, *txResultAssetTransferDetails.assetCloseTo);
    }

    // NOTE: Untested transaction types: Key Registration & Asset Freeze
    static void assertSameTransaction(const model::Transaction & txRef, const model::Transaction & txResult) {
        if (txRef.header.id.hasValue()) EXPECT_EQ(*txRef.header.id, *txResult.header.id);
        EXPECT_EQ(txRef.header.fee, txResult.header.fee);
        EXPECT_EQ(txRef.header.sender, txResult.header.sender);
        EXPECT_EQ(txRef.header.type, txResult.header.type);
        if (txRef.header.round.hasValue()) EXPECT_EQ(*txRef.header.round, *txResult.header.round);
        if (txRef.header.timestamp.hasValue()) EXPECT_EQ(*txRef.header.timestamp, *txResult.header.timestamp);
        if (txRef.header.note.hasValue()) EXPECT_EQ(*txRef.header.note, *txResult.header.note);
        EXPECT_EQ(txRef.header.genesisHash, txResult.header.genesisHash);
        EXPECT_EQ(txRef.header.firstValid, txResult.header.firstValid);
        EXPECT_EQ(txRef.header.lastValid, txResult.header.lastValid);
        if (txRef.header.fromRewards.hasValue()) EXPECT_EQ(*txRef.header.fromRewards, *txResult.header.fromRewards);

        if (txResult.header.type == constants::xPay) {
            assertSamePaymentDetails(txRef.details, txResult.details);
        } else if (txResult.header.type == constants::xAcfg) {
            assertSameAssetConfigDetails(txRef.details, txResult.details);
        } else if (txResult.header.type == constants::xAxfer) {
            assertSameAssetTransferDetails(txRef.details, txResult.details);
        }
    }

}
}
}

#endif
