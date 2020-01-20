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
#include <wallet/stellar/StellarLikeAddress.hpp>
#include <algorithm>

using namespace ledger::core::stellar::xdr;

TEST_F(StellarFixture, XDRCreateAccount) {
    auto hexTx = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63"
                 "de77a4f00000064015dc2cc0000000300000000000000000000000100000000000"
                 "00000000000003a83935fabfdc44749ad4d042dbc4df9b59442f325a27960519fb"
                 "a516adb8a500058d15e176280000000000000000000";
    auto source = std::dynamic_pointer_cast<StellarLikeAddress>(StellarLikeAddress::parse("GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3", getCurrency()))->toPublicKey();
    auto dest = std::dynamic_pointer_cast<StellarLikeAddress>(StellarLikeAddress::parse("GA5IHE27VP64IR2JVVGQILN4JX43LFCC6MS2E6LAKGP3UULK3OFFBJXR", getCurrency()))->toPublicKey();
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

TEST_F(StellarFixture, XDRPayment) {
    auto hexTx = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4"
                 "f00000064015dc2cc0000000300000000000000000000000100000000000000010000000"
                 "03a83935fabfdc44749ad4d042dbc4df9b59442f325a27960519fba516adb8a500000000"
                 "0000000000ee6b2800000000000000000";
    auto source = std::dynamic_pointer_cast<StellarLikeAddress>(StellarLikeAddress::parse("GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3", getCurrency()))->toPublicKey();
    auto dest = std::dynamic_pointer_cast<StellarLikeAddress>(StellarLikeAddress::parse("GA5IHE27VP64IR2JVVGQILN4JX43LFCC6MS2E6LAKGP3UULK3OFFBJXR", getCurrency()))->toPublicKey();
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

TEST_F(StellarFixture, XDRStrings) {
    std::vector<std::string> tested {
        "123", "1", "1234567890", "12345678"
    };

    std::vector<std::string> expected {
        "0000000331323300",
        "0000000131000000",
        "0000000a313233343536373839300000",
        "000000083132333435363738"
    };

    for (auto i = 0; i < tested.size(); i += 1) {
        Encoder encoder;
        encoder << tested[i];
        EXPECT_EQ(hex::toString(encoder.toByteArray()), expected[i]);
    }

}