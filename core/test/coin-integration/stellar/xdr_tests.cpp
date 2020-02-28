/*
 *
 * xdr_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/07/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include "StellarFixture.hpp"
#include <wallet/stellar/xdr/models.hpp>
#include <wallet/stellar/xdr/XDREncoder.hpp>
#include <wallet/stellar/xdr/XDRDecoder.hpp>
#include <wallet/stellar/StellarLikeAddress.hpp>
#include <algorithm>

using namespace ledger::core::stellar::xdr;

#define SOURCE_ADDR "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3"
#define DEST_ADDR "GA5IHE27VP64IR2JVVGQILN4JX43LFCC6MS2E6LAKGP3UULK3OFFBJXR"

TEST_F(StellarFixture, XDRCreateAccountEncode) {
    auto hexTx = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63"
                 "de77a4f00000064015dc2cc0000000300000000000000000000000100000000000"
                 "00000000000003a83935fabfdc44749ad4d042dbc4df9b59442f325a27960519fb"
                 "a516adb8a500058d15e176280000000000000000000";
    auto source = std::dynamic_pointer_cast<StellarLikeAddress>(StellarLikeAddress::parse(SOURCE_ADDR, getCurrency()))->toPublicKey();
    auto dest = std::dynamic_pointer_cast<StellarLikeAddress>(StellarLikeAddress::parse(DEST_ADDR, getCurrency()))->toPublicKey();
    Encoder encoder;
    TransactionEnvelope envelope;
    envelope.tx.sourceAccount.type = PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
    std::copy(source.begin(), source.end(), envelope.tx.sourceAccount.content.begin());
    envelope.tx.seqNum = 98448948301135875UL;
    envelope.tx.fee = 100;

    CreateAccountOp op;
    op.destination.type = PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
    std::copy(dest.begin(), dest.end(), op.destination.content.begin());
    op.startingBalance = 25000000000000000UL;

    stellar::xdr::Operation operation;
    operation.type = stellar::OperationType::CREATE_ACCOUNT;
    operation.content = op;

    envelope.tx.operations.emplace_back(operation);
    encoder << envelope;
    auto out = hex::toString(encoder.toByteArray());
    EXPECT_EQ(out, hexTx);
}

TEST_F(StellarFixture, XDRCreateAccountDecode) {
    auto hexTx = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63"
                 "de77a4f00000064015dc2cc0000000300000000000000000000000100000000000"
                 "00000000000003a83935fabfdc44749ad4d042dbc4df9b59442f325a27960519fb"
                 "a516adb8a500058d15e176280000000000000000000";

    Decoder decoder(hex::toByteArray(hexTx));
    TransactionEnvelope envelope;
    decoder >> envelope;

    auto sourceAddr = StellarLikeAddress::convertPubkeyToAddress(std::vector<uint8_t>(envelope.tx.sourceAccount.content.begin(), envelope.tx.sourceAccount.content.end()), getCurrency().stellarLikeNetworkParameters.value());
    EXPECT_EQ(sourceAddr, SOURCE_ADDR);
    EXPECT_EQ(envelope.tx.sourceAccount.type, stellar::xdr::PublicKeyType::PUBLIC_KEY_TYPE_ED25519);
    EXPECT_EQ(envelope.tx.fee, 100);
    EXPECT_EQ(envelope.tx.seqNum, 98448948301135875UL);

    EXPECT_EQ(envelope.tx.operations.size(), 1);
    stellar::xdr::Operation operation = envelope.tx.operations.front();
    EXPECT_EQ(operation.type, stellar::OperationType::CREATE_ACCOUNT);

    CreateAccountOp op = boost::get<CreateAccountOp>(operation.content);
    auto destAddr = StellarLikeAddress::convertPubkeyToAddress(std::vector<uint8_t>(op.destination.content.begin(), op.destination.content.end()), getCurrency().stellarLikeNetworkParameters.value());
    EXPECT_EQ(destAddr, DEST_ADDR);
    EXPECT_EQ(op.destination.type, PublicKeyType::PUBLIC_KEY_TYPE_ED25519);
    EXPECT_EQ(op.startingBalance, 25000000000000000UL);

}


TEST_F(StellarFixture, XDRPaymentEncode) {
    auto hexTx = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b6"
                 "3de77a4f00000064015dc2cc0000000300000000000000000000000100000000"
                 "00000001000000003a83935fabfdc44749ad4d042dbc4df9b59442f325a27960"
                 "519fba516adb8a5000000000000000000ee6b2800000000000000000";
    auto source = std::dynamic_pointer_cast<StellarLikeAddress>(StellarLikeAddress::parse(SOURCE_ADDR, getCurrency()))->toPublicKey();
    auto dest = std::dynamic_pointer_cast<StellarLikeAddress>(StellarLikeAddress::parse(DEST_ADDR, getCurrency()))->toPublicKey();
    Encoder encoder;
    TransactionEnvelope envelope;
    envelope.tx.sourceAccount.type = PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
    std::copy(source.begin(), source.end(), envelope.tx.sourceAccount.content.begin());
    envelope.tx.seqNum = 98448948301135875UL;
    envelope.tx.fee = 100;

    PaymentOp op;
    op.destination.type = PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
    std::copy(dest.begin(), dest.end(), op.destination.content.begin());
    op.amount = 250000000UL;
    op.asset.type = AssetType::ASSET_TYPE_NATIVE;

    stellar::xdr::Operation operation;
    operation.type = stellar::OperationType::PAYMENT;
    operation.content = op;

    envelope.tx.operations.emplace_back(operation);
    encoder << envelope;
    auto out = hex::toString(encoder.toByteArray());
    EXPECT_EQ(out, hexTx);
}

TEST_F(StellarFixture, XDRPaymentDecode) {
    auto hexTx = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b6"
                 "3de77a4f00000064015dc2cc0000000300000000000000000000000100000000"
                 "00000001000000003a83935fabfdc44749ad4d042dbc4df9b59442f325a27960"
                 "519fba516adb8a5000000000000000000ee6b2800000000000000000";

    Decoder decoder(hex::toByteArray(hexTx));
    TransactionEnvelope envelope;
    decoder >> envelope;

    auto sourceAddr = StellarLikeAddress::convertPubkeyToAddress(std::vector<uint8_t>(envelope.tx.sourceAccount.content.begin(), envelope.tx.sourceAccount.content.end()), getCurrency().stellarLikeNetworkParameters.value());
    EXPECT_EQ(sourceAddr, SOURCE_ADDR);
    EXPECT_EQ(envelope.tx.sourceAccount.type, PublicKeyType::PUBLIC_KEY_TYPE_ED25519);
    EXPECT_EQ(envelope.tx.fee, 100);
    EXPECT_EQ(envelope.tx.seqNum, 98448948301135875UL);

    EXPECT_EQ(envelope.tx.operations.size(), 1);
    stellar::xdr::Operation operation = envelope.tx.operations.front();
    EXPECT_EQ(operation.type, stellar::OperationType::PAYMENT);

    PaymentOp op = boost::get<PaymentOp>(operation.content);
    auto destAddr = StellarLikeAddress::convertPubkeyToAddress(std::vector<uint8_t>(op.destination.content.begin(), op.destination.content.end()), getCurrency().stellarLikeNetworkParameters.value());
    EXPECT_EQ(destAddr, DEST_ADDR);
    EXPECT_EQ(op.destination.type, PublicKeyType::PUBLIC_KEY_TYPE_ED25519);
    EXPECT_EQ(op.amount, 250000000UL);
    EXPECT_EQ(op.asset.type, AssetType::ASSET_TYPE_NATIVE);
}

TEST_F(StellarFixture, XDRStringsEncodeDecode) {
    std::vector<std::string> decodedRef {
        "123", "1", "1234567890", "12345678"
    };

    std::vector<std::string> encodedRef {
        "0000000331323300",
        "0000000131000000",
        "0000000a313233343536373839300000",
        "000000083132333435363738"
    };

    // Encode/decode std::strings individually
    for (auto i = 0; i < decodedRef.size(); i += 1) {
        Encoder encoder;
        encoder << decodedRef[i];
        auto encoded = encoder.toByteArray();
        EXPECT_EQ(encodedRef[i], hex::toString(encoded));

        Decoder decoder(encoded);
        std::string decoded;
        decoder >> decoded;
        EXPECT_EQ(decodedRef[i], decoded);
    }

    // Encode/decode std::strings encapsulated in an object
    {
        Encoder encoder;
        encoder << decodedRef;

        Decoder decoder(encoder.toByteArray());
        std::vector<std::string> decoded(decodedRef.size());
        decoder >> decoded;

        for (auto i = 0; i < decodedRef.size(); i += 1) {
            EXPECT_EQ(decodedRef[i], decoded[i]);
        }
    }

}

TEST_F(StellarFixture, XDRDecodeEnvelopeWithCustomAsset4) {
    auto strEnvelope = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4f00002710015dc2cc0000000"
                       "a0000000000000001000000124575726f207472616e73666572206261636b0000000000010000000000000001000000"
                       "007d3631c2cc8a5ade977d7c504e66837cd557fc46973087cb8fcc4b2e6dd20d9f0000000145555254000000001fd59"
                       "26eafb0827b58033be94c606be2377f25184d269d30460c65498ec80ee7000000000000138800000000000000013de7"
                       "7a4f0000004031ac4f37907cf12f793865dff5e3455656c0622c07e38f2e1a4f18851ca3fd96dfdcc03795e9954b20c"
                       "29f2596837999ad18f24365dc0025f6d0d0042ed37d0f";
    auto rawEnvelope = hex::toByteArray(strEnvelope);
    Decoder decoder(rawEnvelope);
    TransactionEnvelope envelope;
    try {
        decoder >> envelope;
    } catch (...) {

    }
    EXPECT_EQ(envelope.tx.fee, 10000);
    EXPECT_EQ(envelope.tx.memo.type, MemoType::MEMO_TEXT);
    EXPECT_EQ(boost::get<string28>(envelope.tx.memo.content), "Euro transfer back");
    EXPECT_EQ(envelope.tx.operations.size(), 1);
    EXPECT_EQ(envelope.tx.seqNum, 98448948301135882UL);
    EXPECT_EQ(envelope.tx.timeBounds.isEmpty(), true);
    auto sourceAddr = StellarLikeAddress::convertPubkeyToAddress(std::vector<uint8_t>(envelope.tx.sourceAccount.content.begin(), envelope.tx.sourceAccount.content.end()), getCurrency().stellarLikeNetworkParameters.value());
    EXPECT_EQ(sourceAddr, "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(envelope.tx.operations.begin()->type, OperationType::PAYMENT);
    auto operation = boost::get<PaymentOp>(envelope.tx.operations.begin()->content);
    auto destAddr = StellarLikeAddress::convertPubkeyToAddress(std::vector<uint8_t>(operation.destination.content.begin(), operation.destination.content.end()), getCurrency().stellarLikeNetworkParameters.value());
    EXPECT_EQ(destAddr, "GB6TMMOCZSFFVXUXPV6FATTGQN6NKV74I2LTBB6LR7GEWLTN2IGZ6L6X");
    EXPECT_EQ(operation.asset.type, AssetType::ASSET_TYPE_CREDIT_ALPHANUM4);
    auto asset = boost::get<AssetCode4>(operation.asset.assetCode);
    EXPECT_EQ(operation.asset.issuer.nonEmpty(), true);
    EXPECT_EQ(std::string(asset.begin(), asset.end()), "EURT");
    auto issuer = StellarLikeAddress::convertPubkeyToAddress(std::vector<uint8_t>(operation.asset.issuer.getValue().content.begin(), operation.asset.issuer.getValue().content.end()), getCurrency().stellarLikeNetworkParameters.value());
    EXPECT_EQ(issuer, "GAP5LETOV6YIE62YAM56STDANPRDO7ZFDBGSNHJQIYGGKSMOZAHOOS2S");
}