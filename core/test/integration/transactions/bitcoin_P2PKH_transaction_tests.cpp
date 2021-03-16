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
#include "../../fixtures/bch_xpub_fixtures.h"
#include "../../fixtures/zec_xpub_fixtures.h"
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeWritableInputApi.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include "transaction_test_helper.h"
#include <crypto/HASH160.hpp>
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/bitcoin/networks.hpp>
#include <iostream>
using namespace std;

struct BitcoinMakeP2PKHTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin";
        testData.inflate_btc = ledger::testing::medium_xpub::inflate;
    }
};

struct BitcoinStardustTransaction : public BitcoinMakeBaseTransaction {
    // A Bitcoin clone with a very, very high DustAmount to test filtering
    api::Currency bitcoinStardust;

    void SetUpConfig() override {
        api::BitcoinLikeNetworkParameters params(
            "bitcoin_stardust",
            {0x00},
            {0x05},
            {0x04, 0x88, 0xB2, 0x1E},
            api::BitcoinLikeFeePolicy::PER_BYTE,
            std::numeric_limits<int64_t>::max(),
            "Bitcoin Stardust Signed Message:\n",
            false,
            0,
            {0x01},
            {}
        );

        bitcoinStardust = CurrencyBuilder("bitcoin_stardust")
            .forkOfBitcoin(params)
            .bip44(42)
            .unit("satoshiStardust", 0, "SSD");

        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_stardust";
        testData.inflate_btc = ledger::testing::medium_xpub::inflate;
    }

    void recreate() override {
        pool = newDefaultPool();
        pool->addCurrency(bitcoinStardust);
        wallet = wait(pool->createWallet(testData.walletName, testData.currencyName, testData.configuration));
        account = testData.inflate_btc(pool, wallet);
        currency = wallet->getCurrency();
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
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
    auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
//    EXPECT_EQ(
//            "0100000001f6390f2600568e3dd28af5d53e821219751d6cb7a03ec9476f96f5695f2807a2000000001976a914bfe0a15bbed6211262d3a8d8a891e738bab36ffb88acffffffff0210270000000000001976a91423cc0488e5832d8f796b88948b8af1dd186057b488ac10580100000000001976a914d642b9c546d114dc634e65f72283e3458032a3d488ac41eb0700",
//            hex::toString(tx->serialize())
//    );
}

TEST_F(BitcoinStardustTransaction, FilterDustUtxo) {
    ASSERT_EQ(
        currency.bitcoinLikeNetworkParameters->DustAmount,
        std::numeric_limits<int64_t>::max()
    ) << "The currency in this test should have a very high dust amount";

    auto builder = tx_builder();

    builder->sendToAddress(api::Amount::fromLong(currency, 20000000), "36v1GRar68bBEyvGxi9RQvdP6Rgvdwn2C2");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 61));
    auto f = builder->build();
    try {
        auto tx = ::wait(f);
        FAIL() << "Should throw a \"no UTXO found\" exception when trying to build the transaction";
    } catch (const Exception &err) {
        ASSERT_EQ(err.getErrorCode(), api::ErrorCode::NOT_ENOUGH_FUNDS);
        ASSERT_STREQ("There is no UTXO on this account.", err.what());
    }
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

TEST_F(BitcoinMakeP2PKHTransaction, OptimizeSize) {
    auto builder = tx_builder();
    const int64_t feesPerByte = 20;
    builder->sendToAddress(api::Amount::fromLong(currency, 10000), "14GH47aGFWSjvdrEiYTEfwjgsphNtbkWzP");
    builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, feesPerByte));
    auto f = builder->build();
    auto tx = ::wait(f);
    tx->getInputs()[0]->pushToScriptSig({ 5, 'h', 'e', 'l', 'l', 'o' });
    auto transactionSize = tx->serialize().size();
    auto fees = tx->getFees();
    EXPECT_TRUE(fees->toLong() >= transactionSize * feesPerByte);
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

    auto builder = std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(bla->buildTransaction(false));
    builder->sendToAddress(api::Amount::fromLong(currency, 1000), "ms8C1x7qHa3WJM986NKyx267i2LFGaHRZn");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 10));

    // Change as fresh address
    auto freshChangeAddress = bla->getKeychain()->getFreshAddress(BitcoinLikeKeychain::CHANGE);
    auto changePath = freshChangeAddress->getDerivationPath().value_or("");
    EXPECT_GT(changePath.size(), 0);
    auto tx = ::wait(builder->build());
    auto addressFoundInOutputs = [=] (const std::shared_ptr<api::BitcoinLikeTransaction> &inputTx,
                                      const std::string &inputAddress) {
        return std::find_if(inputTx->getOutputs().begin(),
                            inputTx->getOutputs().end(),
                            [&] (const std::shared_ptr<api::BitcoinLikeOutput> &out) {
                                if (!out) {
                                    return false;
                                }
                                return out->getAddress().value_or("") == inputAddress;
                            }
        ) != inputTx->getOutputs().end();
    };
    EXPECT_TRUE(addressFoundInOutputs(tx, freshChangeAddress->toString()));

    // Change as chosen address
    // Compare change address to the one set
    auto changeAddress = bla->getKeychain()->getAllObservableAddresses(BitcoinLikeKeychain::CHANGE, 5, 5);
    EXPECT_GT(changeAddress.size(), 0);
    changePath = changeAddress[0]->getDerivationPath().value_or("");
    EXPECT_GT(changePath.size(), 0);
    builder->addChangePath(changePath);
    tx = ::wait(builder->build());
    EXPECT_TRUE(addressFoundInOutputs(tx, changeAddress[0]->toString()));

    std::cout << hex::toString(tx->serialize()) << std::endl;
    std::cout << tx->getOutputs()[0]->getAddress().value_or("NOP") << std::endl;
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
    auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    std::cout << hex::toString(parsedTx->serialize()) << std::endl;
    std::cout << parsedTx->getInputs().size() << std::endl;
    std::cout << hex::toString(rawPrevious) << std::endl;
    std::cout << tx->getFees()->toLong() << std::endl;
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}

struct BCHMakeP2PKHTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_cash";
        testData.inflate_btc = ledger::testing::bch_xpub::inflate;
    }
};

TEST_F(BCHMakeP2PKHTransaction, CreateStandardP2SHWithOneOutput) {
    auto buildBCHTxWithAddress = [=](const std::string & toAddress) -> std::string {
        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 5000), toAddress);
        builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 41));
        auto f = builder->build();
        auto tx = ::wait(f);
        auto serializedTx = tx->serialize();
        auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), serializedTx, 0);
        EXPECT_EQ(serializedTx, parsedTx->serialize());
        return hex::toString(tx->serialize());
    };

    // https://explorer.bitcoin.com/bch/address/14RYdhaFU9fMH25e6CgkRrBRjZBvEvKxne
    auto legacySerializeTx = buildBCHTxWithAddress("14RYdhaFU9fMH25e6CgkRrBRjZBvEvKxne");
    auto cashAddressSerializeTx = buildBCHTxWithAddress("bitcoincash:qqjce3pqczzukajdqc27psjnd26lyz2d5yg2cfjtxe");
    EXPECT_EQ(legacySerializeTx, cashAddressSerializeTx);
}

struct ZCASHMakeP2PKHTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "zcash";
        testData.inflate_btc = ledger::testing::zec_xpub::inflate;
    }
};

TEST_F(ZCASHMakeP2PKHTransaction, CreateStandardP2PKHWithOneOutput) {

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
    dispatcher->waitUntilStopped();

    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 2000), "t1MepQJABxoWarqMvgBHGiFprtuvA47Hiv8");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 41));
    auto f = builder->build();
    auto tx = ::wait(f);
    cout<<hex::toString(tx->serialize())<<endl;
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 40000000);
    cout<<" parsedTx = "<<hex::toString(parsedTx->serialize())<<endl;
    cout<<" tx = "<<hex::toString(tx->serialize())<<endl;
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}

TEST_F(ZCASHMakeP2PKHTransaction, ParseSignedRawTransaction) {
    //Tx hash 4858a0a3d5f1de0c0f5729f25c3501bda946093aed07f842e53a90ac65d66f70
    auto hash = "4858a0a3d5f1de0c0f5729f25c3501bda946093aed07f842e53a90ac65d66f70";
    auto strTx = "0100000001f8355b0761296d28e29bd39833fe8c6558120037498dfdedabf41890e65c68dc010000006b483045022100a13ae06b36e3d4e90c7b9265bfff296b98d79c9970d7ef4964eb26d23ab44a5f022024155e86bde7a2322b1395d904c5fa007a925bc02f7d60623bde56a8b09bbb680121032d1d22333719a013313e538557971639f8c167fa5be8089dd2e996d704fb580cffffffff02a0860100000000001976a91407c4358a95e07e570d67857e12086fd6b1ee873688acf24f1600000000001976a9143c1a6afff1941911e0b524ffcd2a15de6e68b6d188ac00000000";
    auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(currency, hex::toByteArray(strTx), 0);
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);
    EXPECT_EQ(tx->getHash(), hash);
    EXPECT_GT(tx->getInputs().size(), 0);
    EXPECT_EQ(tx->getInputs()[0]->getAddress().value_or(""), "t1MepQJABxoWarqMvgBHGiFprtuvA47Hiv8");
    EXPECT_GT(tx->getOutputs().size(), 0);
    EXPECT_EQ(tx->getOutputs()[0]->getAddress().value_or(""), "t1JafnXdJDUUjLnTfbvuEBdCARKLvqmj5jb");
    EXPECT_EQ(tx->getOutputs()[1]->getAddress().value_or(""), "t1PMQCQ36ccBJeEdFXJh4nnU34rPyDimANE");
}

//TODO: activate when adding expiryHeight and extraData to serialized transaction (refer to BitcoinLikeTransactionApi::serializeEpilogue)
/*
TEST_F(ZCASHMakeP2SHTransaction, ParseSignedRawTransactionOverwinter) {
    //Tx hash 69d831ac9c59f7d7077c1ae6d85daa7b7094a6f70e57b77cc17ab325bba218ab
    auto strTx = "030000807082c40301c3843943e3fe3a339ea82df309e9e47800901bac663d7560cba596b6a8867e66000000006a4730440220601f97f6542da7759e5ef6b970a1a174f05729476fdd31fe4eea663ab59fb62502201d25092c5c7e78c4870436442ecdb97bbe82f7821f0eaffe8381e6299fb2ad24012103453572fa9f91e9ff996c07a86df358ede8c1048e70512bd4137c4c7d14820496ffffff0002ade11b00000000001976a9146f09a1a28ccb28ad4f43cb28dce0726eb67782d988ac3fdc1100000000001976a914f297f6f25efa75450bd859158ccc34cad254830c88ac000000000000000000";
    auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(currency, hex::toByteArray(strTx), networks::ZIP143_PARAMETERS.blockHeight + 1);
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);
}

TEST_F(ZCASHMakeP2SHTransaction, ParseSignedRawTransactionSapling) {
    //Tx hash 612554466c22ff0868642780772af1d39fae32f532c9aa8ecfd618aff80a061c
    auto strTx = "0400008085202f8903b78280210b4f47efd39494367fad0dd1b4a28ae29630eea51fe4a013f313d217000000006a4730440220353ee5947facaa24aed4c24bd26f83ba9328f6442c1a72a021d78ac207cf838d02201a577ca3e9beeeaf06fe282dd45ebbc93590e141c2c3bbca5b5a82ea976b86430121025a40f94ea1b1d2d1dbf3724479d3d2de6672d4fcbe1e01b27d7fb52b139003f7ffffff00b78280210b4f47efd39494367fad0dd1b4a28ae29630eea51fe4a013f313d217010000006a47304402201653f8b4aedf8a7946d1b5938375234f33d14ef36fbbc4193a287a3ccc662af602200c92b4412ee33c37f3d474342c91dfc8dcd5c33852c86e4fb293be5db70d0f98012103a9eb92dc2b28806d86575fbc61fd4211b0814eaa9b558fd40971c7688afd8111ffffff00c955fe5f0b02228d48ee2b57987b1e5f06ab098a42c00c6f8c457d8570521d03000000006b483045022100daaf0b55bef234679a92483c277ae24770453e54d2dcbce04ce3fa1ae4b18ae20220589f1cac3f6f9d7b2ffbe14766b0109db89de446c0839e2fc504f1ace8fb34ee012103720608c07403ba0fc52a7b6d7f06d4ba24ced514637534af10da60f8ecd04eaeffffff000240420f00000000001976a9147861d9accb79726d4462ea7bc8a1c6367914053a88ac10500e00000000001976a91481e0461de63e3e4c4f2f2691e61da5925b6f526688ac00000000000000000000000000000000000000";
    auto tx = BitcoinLikeTransactionApi::parseRawSignedTransaction(currency, hex::toByteArray(strTx), networks::ZIP_SAPLING_PARAMETERS.blockHeight + 1);
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);
}
*/