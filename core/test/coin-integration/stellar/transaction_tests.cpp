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
    auto builder = std::dynamic_pointer_cast<StellarLikeTransactionBuilder>(account->buildTransaction());
    auto sequence = ::wait(account->getSequence());
    auto fees = ::wait(account->getFeeStats()).modeAcceptedFee;
    builder->setSequence(api::BigInt::fromLong(sequence.toInt64()));
    builder->setBaseFee(api::Amount::fromLong(wallet->getCurrency(), sequence.toInt64()));
    builder->addNativePayment("GA5IHE27VP64IR2JVVGQILN4JX43LFCC6MS2E6LAKGP3UULK3OFFBJXR", api::Amount::fromLong(wallet->getCurrency(), 20000000));
    builder->setBaseFee( api::Amount::fromLong(wallet->getCurrency(), 100));
    auto tx = ::wait(builder->build());
    tx->putSignature(hex::toByteArray("3045022100B2B31575F8536B284410D01217F688BE3A9FAF4BA0BA3A9093F983E40D630EC7022022A7A25B01403CFF0D00B3B853D230F8E96FF832B15D4CCC75203CB65896A2D5"));
    auto envelope = std::dynamic_pointer_cast<StellarLikeTransaction>(tx)->envelope();
    EXPECT_EQ(envelope.signatures.size() , 1);
    EXPECT_EQ(envelope.tx.sourceAccount.type, stellar::xdr::PublicKeyType::PUBLIC_KEY_TYPE_ED25519);
    auto accountPubKey = account->getKeychain()->getAddress()->toPublicKey();
    std::vector<uint8_t> envelopeSourcePubKey(envelope.tx.sourceAccount.content.begin(), envelope.tx.sourceAccount.content.end());
    EXPECT_EQ(accountPubKey, envelopeSourcePubKey);
    EXPECT_TRUE(envelope.tx.seqNum >= 98448948301135874L);
    EXPECT_TRUE(envelope.tx.fee == fees);
    EXPECT_TRUE(envelope.tx.operations.size() == 1);
    EXPECT_EQ(envelope.tx.memo.type, stellar::xdr::MemoType::MEMO_NONE);
}