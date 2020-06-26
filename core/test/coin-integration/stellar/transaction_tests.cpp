/*
 *
 * transaction_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/08/2019.
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
#include <wallet/stellar/transaction_builders/StellarLikeTransactionBuilder.hpp>
#include <wallet/currencies.hpp>
#include <api_impl/BigIntImpl.hpp>

TEST_F(StellarFixture, PaymentTransaction) {
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet", "stellar", api::DynamicObject::newInstance());
    auto info = ::wait(wallet->getNextAccountCreationInfo());
    auto account = newAccount(wallet, 0, defaultAccount());
    auto bus = account->synchronize();
    bus->subscribe(dispatcher->getMainExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       dispatcher->stop();
                   }));
    EXPECT_EQ(bus, account->synchronize());
    dispatcher->waitUntilStopped();
    auto address = *wait(account->getFreshPublicAddresses()).begin();
    auto signature = hex::toByteArray("3045022100B2B31575F8536B284410D01217F688BE3A9FAF4BA0BA3A9093F983E40D630EC7022022A7A25B01403CFF0D00B3B853D230F8E96FF832B15D4CCC75203CB65896A2D5");
    auto builder = std::dynamic_pointer_cast<StellarLikeTransactionBuilder>(account->buildTransaction());
    auto sequence = ::wait(account->getSequence());
    auto fees =  100;//Disabled until nodes can handle this request ==> ::wait(account->getFeeStats()).modeAcceptedFee;
    builder->setSequence(api::BigInt::fromLong(sequence.toInt64()));
    builder->addNativePayment("GA5IHE27VP64IR2JVVGQILN4JX43LFCC6MS2E6LAKGP3UULK3OFFBJXR", api::Amount::fromLong(wallet->getCurrency(), 20000000));
    builder->setBaseFee( api::Amount::fromLong(wallet->getCurrency(), fees));
    auto tx = ::wait(builder->build());
    tx->putSignature(signature, address);
    auto wrappedEnvelope = std::dynamic_pointer_cast<StellarLikeTransaction>(tx)->envelope();
    const auto& envelope = boost::get<stellar::xdr::TransactionV1Envelope>(wrappedEnvelope.content);
    EXPECT_EQ(envelope.signatures.size() , 1);
    EXPECT_EQ(envelope.tx.sourceAccount.type, stellar::xdr::CryptoKeyType::KEY_TYPE_ED25519);
    auto accountPubKey = account->getKeychain()->getAddress()->toPublicKey();
    auto sourceAccount = boost::get<stellar::xdr::uint256>(envelope.tx.sourceAccount.content);
    std::vector<uint8_t> envelopeSourcePubKey(sourceAccount.begin(), sourceAccount.end());
    EXPECT_EQ(accountPubKey, envelopeSourcePubKey);
    EXPECT_TRUE(envelope.tx.seqNum >= 98448948301135874L);
    EXPECT_TRUE(envelope.tx.fee == fees);
    EXPECT_TRUE(envelope.tx.operations.size() == 1);
    EXPECT_EQ(envelope.tx.memo.type, stellar::xdr::MemoType::MEMO_NONE);
    EXPECT_EQ(envelope.signatures.front().signature, signature);
    fmt::print("{}\n", hex::toString(tx->toRawTransaction()));
}

TEST_F(StellarFixture, ParseRawTransaction) {
    auto strTx = "00000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4f0"
                 "0000064015dc2cc000000030000000000000000000000010000000000000001000000003a"
                 "83935fabfdc44749ad4d042dbc4df9b59442f325a27960519fba516adb8a5000000000000"
                 "00000000000000000000000000000";

    auto tx = api::StellarLikeTransactionBuilder::parseRawTransaction(ledger::core::currencies::STELLAR, hex::toByteArray(strTx));

    EXPECT_EQ(tx->getSourceAccount()->toString(), "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(tx->getSourceAccountSequence()->compare(api::BigInt::fromLong(98448948301135875L)), 0);
    EXPECT_EQ(tx->getFee()->toLong(), 100L);

    EXPECT_EQ(hex::toString(tx->toRawTransaction()), strTx);
}

TEST_F(StellarFixture, ParseSignatureBase) {
    auto strTx = "7ac33997544e3175d266bd022439b22cdb16508c01163f26e5cb2a3e1045a9790000000200"
                 "000000a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4f0000"
                 "0064015dc2cc000000030000000000000000000000010000000000000001000000003a8393"
                 "5fabfdc44749ad4d042dbc4df9b59442f325a27960519fba516adb8a500000000000000000"
                 "0000000000000000";

    auto tx = api::StellarLikeTransactionBuilder::parseSignatureBase(ledger::core::currencies::STELLAR, hex::toByteArray(strTx));

    EXPECT_EQ(tx->getSourceAccount()->toString(), "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(tx->getSourceAccountSequence()->compare(api::BigInt::fromLong(98448948301135875L)), 0);
    EXPECT_EQ(tx->getFee()->toLong(), 100L);

    EXPECT_EQ(hex::toString(tx->toSignatureBase()), strTx);
}
