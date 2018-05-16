/*
 *
 * bitcoin_transaction_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/03/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "../BaseFixture.h"
#include "../../fixtures/medium_xpub_fixtures.h"
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeWritableInputApi.h>
#include "transaction_test_helper.h"

struct BitcoinMakeP2PKHTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin";
        testData.inflate = ledger::testing::medium_xpub::inflate;
    }
};

TEST_F(BitcoinMakeP2PKHTransaction, CreateStandardP2PKHWithOneOutput) {
    auto builder = tx_builder();

    auto balance = wait(account->getBalance());

    builder->sendToAddress(api::Amount::fromLong(currency, 20000000), "36v1GRar68bBEyvGxi9RQvdP6Rgvdwn2C2");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    //builder->excludeUtxo("beabf89d72eccdcb895373096a402ae48930aa54d2b9e4d01a05e8f068e9ea49", 0);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 61));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize());
    auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
//    EXPECT_EQ(
//            "0100000001f6390f2600568e3dd28af5d53e821219751d6cb7a03ec9476f96f5695f2807a2000000001976a914bfe0a15bbed6211262d3a8d8a891e738bab36ffb88acffffffff0210270000000000001976a91423cc0488e5832d8f796b88948b8af1dd186057b488ac10580100000000001976a914d642b9c546d114dc634e65f72283e3458032a3d488ac41eb0700",
//            hex::toString(tx->serialize())
//    );
}


TEST_F(BitcoinMakeP2PKHTransaction, CreateStandardP2PKHWithOneOutputAndFakeSignature) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 10000), "14GH47aGFWSjvdrEiYTEfwjgsphNtbkWzP");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
    auto f = builder->build();
    auto tx = ::wait(f);
    tx->getInputs()[0]->pushToScriptSig({5, 'h', 'e', 'l', 'l', 'o'});
    std::cout << hex::toString(tx->serialize()) << std::endl;
//    EXPECT_EQ(
//            "0100000001f6390f2600568e3dd28af5d53e821219751d6cb7a03ec9476f96f5695f2807a200000000060568656c6c6fffffffff0210270000000000001976a91423cc0488e5832d8f796b88948b8af1dd186057b488ac10580100000000001976a914d642b9c546d114dc634e65f72283e3458032a3d488ac41eb0700",
//            hex::toString(tx->serialize())
//    );
}

TEST_F(BitcoinMakeP2PKHTransaction, CreateStandardP2PKHWithMultipleInputs) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 100000000), "14GH47aGFWSjvdrEiYTEfwjgsphNtbkWzP");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
    auto f = builder->build();
    auto tx = ::wait(f);
    std::cout << hex::toString(tx->serialize()) << std::endl;
//    EXPECT_EQ(
//            "0100000003f6390f2600568e3dd28af5d53e821219751d6cb7a03ec9476f96f5695f2807a2000000001976a914bfe0a15bbed6211262d3a8d8a891e738bab36ffb88acffffffff02e32c49a32937f38fc25d28b8bcd90baaea7b592649af465792cac7b6a9e484000000001976a9148229692a444f0c3f75faafcfd465540a7c2f954988acffffffff1de6319dc1176cc26ee2ed80578dfdbd85a1147dcf9e10eebb5d416f33919f51000000001976a91415a3065a3c32b2d4de4dcceafca0fbd674bdcf7988acffffffff0280969800000000001976a91423cc0488e5832d8f796b88948b8af1dd186057b488acd0b51000000000001976a914d642b9c546d114dc634e65f72283e3458032a3d488ac41eb0700",
//            hex::toString(tx->serialize())
//    );
}

TEST_F(BitcoinMakeP2PKHTransaction, Toto) {
    std::shared_ptr<AbstractWallet> w = wait(pool->createWallet("my_btc_wallet", "bitcoin_testnet", DynamicObject::newInstance()));
    api::ExtendedKeyAccountCreationInfo info = wait(w->getNextExtendedKeyAccountCreationInfo());
    info.extendedKeys.push_back("tpubDCJarhe7f951cUufTWeGKh1w6hDgdBcJfvQgyMczbxWvwvLdryxZuchuNK3KmTKXwBNH6Ze6tHGrUqvKGJd1VvSZUhTVx58DrLn9hR16DVr");
    std::shared_ptr<AbstractAccount> account = std::dynamic_pointer_cast<AbstractAccount>(wait(w->newAccountWithExtendedKeyInfo(info)));
    std::shared_ptr<BitcoinLikeAccount> bla = std::dynamic_pointer_cast<BitcoinLikeAccount>(account);
    Promise<Unit> p;
    auto s = bla->synchronize();
    s->subscribe(bla->getContext(), make_receiver([=](const std::shared_ptr<api::Event> &event) mutable {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
        p.success(unit);
    }));
    Unit u = wait(p.getFuture());

    auto builder = std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(bla->buildTransaction());
    builder->sendToAddress(api::Amount::fromLong(currency, 1000), "ms8C1x7qHa3WJM986NKyx267i2LFGaHRZn");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
    auto f = builder->build();
    auto tx = ::wait(f);
    std::cout << hex::toString(tx->serialize()) << std::endl;
    std::cout << tx->getOutputs()[0]->getAddress().value_or("NOP") << std::endl;
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize());
    auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    std::cout << hex::toString(parsedTx->serialize()) << std::endl;
    std::cout << parsedTx->getInputs().size() << std::endl;
    std::cout << hex::toString(rawPrevious) << std::endl;
    std::cout << tx->getFees()->toLong() << std::endl;
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}