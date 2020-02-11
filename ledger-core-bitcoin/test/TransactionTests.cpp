/*
 *
 * TransactionTests.cpp
 * ledger-core-bitcoin
 *
 * Created by Alexis Le Provost on 10/02/2020.
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

#include <bitcoin/BitcoinLikeAccount.hpp>
#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/BitcoinLikeWallet.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>
#include <bitcoin/transactions/BitcoinLikeTransactionBuilder.hpp>
#include <bitcoin/io/BitcoinLikeWritableInput.hpp>
#include <bitcoin/transactions/BitcoinLikeTransaction.hpp>
#include <bitcoin/bech32/Bech32Factory.hpp>

#include <integration/TransactionTestHelper.hpp>

#include "fixtures/MediumXPubFixtures.hpp"
#include "fixtures/BCHXPubFixtures.hpp"
#include "fixtures/ZECXPubFixtures.hpp"
#include "fixtures/TestnetXPubFixtures.hpp"
#include "fixtures/BTGXPubFixtures.hpp"
#include "fixtures/CoinSelectionXPubFixtures.hpp"

struct BitcoinTransactions : public MakeBaseTransaction<
        BitcoinLikeAccount,
        BitcoinLikeWalletFactory,
        BitcoinLikeWallet,
        BitcoinLikeTransactionBuilder
    >
{
    std::shared_ptr<BitcoinLikeTransactionBuilder> tx_builder() {
        return std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(account->buildTransaction(false));
    } 
};
struct BitcoinP2PKHTransactions : public BitcoinTransactions
{
    api::Currency getCurrency() const override {
         return currencies::bitcoin();
    }

    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin";
        testData.inflate_coin = ledger::testing::medium_xpub::inflate;
    }
};

struct BitcoinBCHP2PKHTransactions : public BitcoinTransactions
{
    api::Currency getCurrency() const override {
        return currencies::bitcoin_cash();
    }

    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_cash";
        testData.inflate_coin = ledger::testing::bch_xpub::inflate;
    }
};

struct BitcoinZCASHP2PKHTransactions : public BitcoinTransactions {
    api::Currency getCurrency() const override {
        return currencies::zcash();
    }

    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "zcash";
        testData.inflate_coin = ledger::testing::zec_xpub::inflate;
    }
};

struct BitcoinP2SHTransactions : public BitcoinTransactions {
    api::Currency getCurrency() const override {
        return currencies::bitcoin_testnet();
    }

    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_testnet";
        testData.inflate_coin = ledger::testing::testnet_xpub::inflate;
    }
};

struct BitcoinBTGP2SHTransactions : public BitcoinTransactions {
    api::Currency getCurrency() const override {
        return currencies::bitcoin_gold();
    }

    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_gold";
        testData.inflate_coin = ledger::testing::btg_xpub::inflate;
    }
};

struct BitcoinP2WPKHTransactions : public BitcoinTransactions {
    api::Currency getCurrency() const override {
        return currencies::bitcoin();
    }

    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP173_P2WPKH);
        //https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"84'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin";
        testData.inflate_coin = ledger::testing::medium_xpub::inflate;
    }
};

struct BitcoinP2WSHTransactions : public BitcoinTransactions {
    api::Currency getCurrency() const override {
        return currencies::bitcoin();
    }

    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP173_P2WSH);
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin";
        testData.inflate_coin = ledger::testing::medium_xpub::inflate;
    }
};

struct BitcoinCoinSelectionTransactions : public BitcoinTransactions {
    api::Currency getCurrency() const override {
        return currencies::bitcoin_testnet();
    }
    
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_testnet";
        testData.inflate_coin = ledger::testing::coin_selection_xpub::inflate;
    }
};

TEST_F(BitcoinP2PKHTransactions, CreateStandardP2PKHWithOneOutput) {
    auto builder = tx_builder();

    auto balance = wait(account->getBalance());

    builder->sendToAddress(api::Amount::fromLong(currency, 20000000), "36v1GRar68bBEyvGxi9RQvdP6Rgvdwn2C2");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 61));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
    auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInput>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}


TEST_F(BitcoinP2PKHTransactions, CreateStandardP2PKHWithOneOutputAndFakeSignature) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 10000), "14GH47aGFWSjvdrEiYTEfwjgsphNtbkWzP");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
    auto f = builder->build();
    auto tx = ::wait(f);
    tx->getInputs()[0]->pushToScriptSig({5, 'h', 'e', 'l', 'l', 'o'});
    std::cout << hex::toString(tx->serialize()) << std::endl;
}

TEST_F(BitcoinP2PKHTransactions, CreateStandardP2PKHWithMultipleInputs) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 100000000), "14GH47aGFWSjvdrEiYTEfwjgsphNtbkWzP");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
    auto f = builder->build();
    auto tx = ::wait(f);
    std::cout << hex::toString(tx->serialize()) << std::endl;
}

// TODO: address a ticket to the "Toto" creator to validate the usefulness of this test
// TEST_F(BitcoinP2PKHTransactions, Toto) {
//     std::shared_ptr<AbstractWallet> w = wait(services->createWallet("my_btc_wallet", "bitcoin_testnet", DynamicObject::newInstance()));
//     api::ExtendedKeyAccountCreationInfo info = wait(w->getNextExtendedKeyAccountCreationInfo());
//     info.extendedKeys.push_back("tpubDCJarhe7f951cUufTWeGKh1w6hDgdBcJfvQgyMczbxWvwvLdryxZuchuNK3KmTKXwBNH6Ze6tHGrUqvKGJd1VvSZUhTVx58DrLn9hR16DVr");
//     std::shared_ptr<AbstractAccount> account = std::dynamic_pointer_cast<AbstractAccount>(wait(w->newAccountWithExtendedKeyInfo(info)));
//     std::shared_ptr<BitcoinLikeAccount> bla = std::dynamic_pointer_cast<BitcoinLikeAccount>(account);
//     Promise<Unit> p;
//     auto s = bla->synchronize();
//     s->subscribe(bla->getContext(), make_receiver([=](const std::shared_ptr<api::Event> &event) mutable {
//         fmt::print("Received event {}\n", api::to_string(event->getCode()));
//         if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
//             return;
//         EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
//         EXPECT_EQ(event->getCode(),
//                   api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
//         p.success(unit);
//     }));
//     Unit u = wait(p.getFuture());

//     auto builder = std::dynamic_pointer_cast<BitcoinLikeTransactionBuilder>(bla->buildTransaction(false));
//     builder->sendToAddress(api::Amount::fromLong(currency, 1000), "ms8C1x7qHa3WJM986NKyx267i2LFGaHRZn");
//     builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
//     builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
//     auto f = builder->build();
//     auto tx = ::wait(f);
//     std::cout << hex::toString(tx->serialize()) << std::endl;
//     std::cout << tx->getOutputs()[0]->getAddress().value_or("NOP") << std::endl;
//     auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
//     auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInput>(tx->getInputs()[0])->getPreviousTransaction());
//     std::cout << hex::toString(parsedTx->serialize()) << std::endl;
//     std::cout << parsedTx->getInputs().size() << std::endl;
//     std::cout << hex::toString(rawPrevious) << std::endl;
//     std::cout << tx->getFees()->toLong() << std::endl;
//     EXPECT_EQ(tx->serialize(), parsedTx->serialize());
// }

TEST_F(BitcoinBCHP2PKHTransactions, CreateStandardP2SHWithOneOutput) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 5000), "14RYdhaFU9fMH25e6CgkRrBRjZBvEvKxne");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 41));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
    //auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}

TEST_F(BitcoinZCASHP2PKHTransactions, CreateStandardP2PKHWithOneOutput) {

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
    std::cout<<hex::toString(tx->serialize())<<std::endl;
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 40000000);
    std::cout<<" parsedTx = "<<hex::toString(parsedTx->serialize())<<std::endl;
    std::cout<<" tx = "<<hex::toString(tx->serialize())<<std::endl;
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}

TEST_F(BitcoinZCASHP2PKHTransactions, ParseSignedRawTransaction) {
    //Tx hash 4858a0a3d5f1de0c0f5729f25c3501bda946093aed07f842e53a90ac65d66f70
    auto hash = "4858a0a3d5f1de0c0f5729f25c3501bda946093aed07f842e53a90ac65d66f70";
    auto strTx = "0100000001f8355b0761296d28e29bd39833fe8c6558120037498dfdedabf41890e65c68dc010000006b483045022100a13ae06b36e3d4e90c7b9265bfff296b98d79c9970d7ef4964eb26d23ab44a5f022024155e86bde7a2322b1395d904c5fa007a925bc02f7d60623bde56a8b09bbb680121032d1d22333719a013313e538557971639f8c167fa5be8089dd2e996d704fb580cffffffff02a0860100000000001976a91407c4358a95e07e570d67857e12086fd6b1ee873688acf24f1600000000001976a9143c1a6afff1941911e0b524ffcd2a15de6e68b6d188ac00000000";
    auto tx = BitcoinLikeTransaction::parseRawSignedTransaction(currency, hex::toByteArray(strTx), 0);
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

TEST_F(BitcoinP2SHTransactions, CreateStandardP2SHWithOneOutput) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 200000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 71));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
    //auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}

TEST_F(BitcoinP2SHTransactions, CreateStandardP2SHWithWipeToAddress) {
    auto builder = tx_builder();
    builder->wipeToAddress("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 71));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto outputs = tx->getOutputs();
    auto fees = tx->getFees();
    auto balance = wait(account->getBalance());
    EXPECT_EQ(outputs.size(), 1);
    auto maxAmount = outputs[0]->getValue();
    EXPECT_EQ(balance->toLong(), maxAmount->toLong() + fees->toLong());
    auto txSerialized = tx->serialize();
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), txSerialized, 0);
    auto parsedTxSerialized = parsedTx->serialize();
    std::cout<<"tx->serialize(): "<<hex::toString(txSerialized)<<std::endl;
    std::cout<<"parsedTx->serialize(): "<<hex::toString(parsedTxSerialized)<<std::endl;
    EXPECT_EQ(txSerialized, parsedTxSerialized);
}

TEST_F(BitcoinP2SHTransactions, ParseSignedRawTransaction) {
    //Tx hash 93ae1990d10745e3ab4bf742d4b06bd513e7a26384617a17525851e4e3ed7038
    auto hash = "93ae1990d10745e3ab4bf742d4b06bd513e7a26384617a17525851e4e3ed7038";
    auto strTx = "0100000000010182815d16259062c4bc08c4fb3aa985444b7208197cf676212f1b3da93782e19f0100000017160014e4fae08faaa8469c5756fda7fbfde46922a4e7b2ffffffff0280f0fa020000000017a91428242bc4e7266060e084fab55fb70916b605d0b3870f4a9b040000000017a91401445204b7063c76c702501899334d6f7499806d870248304502210085a85a2dec818ece4748c0c9d71640a5703a5eec9112dd58183b048c6a9961cf02201dd0beabc1c3500f75849a046deab9e2d2ce388fff6cb92693508f5ea406471d012103d2f424cd1f60e96241a968b9da3c3f6b780f90538bdf306350b9607c279ad48600000000";
    auto tx = BitcoinLikeTransaction::parseRawSignedTransaction(currency, hex::toByteArray(strTx), 0);
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);
    EXPECT_EQ(tx->getHash(), hash);
    EXPECT_GT(tx->getInputs().size(), 0);
    EXPECT_EQ(tx->getInputs()[0]->getAddress().value_or(""), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
    EXPECT_GT(tx->getOutputs().size(), 0);
    EXPECT_EQ(tx->getOutputs()[0]->getAddress().value_or(""), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
    EXPECT_EQ(tx->getOutputs()[1]->getAddress().value_or(""), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
}


TEST_F(BitcoinBTGP2SHTransactions, CreateStandardP2SHWithOneOutput) {
    auto builder = tx_builder();
    builder->sendToAddress(api::Amount::fromLong(currency, 2000), "ATqSa4V9ZjxPxDBe877bbXeKMfZA644mBk");
    builder->pickInputs(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, 0xFFFFFFFF);
    builder->setFeesPerByte(api::Amount::fromLong(currency, 41));
    auto f = builder->build();
    auto tx = ::wait(f);
    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), tx->serialize(), 0);
    //auto rawPrevious = ::wait(std::dynamic_pointer_cast<BitcoinLikeWritableInputApi>(tx->getInputs()[0])->getPreviousTransaction());
    EXPECT_EQ(tx->serialize(), parsedTx->serialize());
}

TEST_F(BitcoinP2WPKHTransactions, CreateStandardP2WPKHWithOneOutput) {
    auto address = "bc1qshh6mmfq8fucahzxe4zc7pc5zdhk6zkt4uv8md";
    auto builder = tx_builder();
    auto freshAddress = wait(account->getFreshPublicAddresses())[0];
    auto hrp = Bech32Factory::newBech32Instance("btc").getValue()->getBech32Params().hrp;
    auto freshAddressStr = std::dynamic_pointer_cast<BitcoinLikeAddress>(freshAddress)->toBech32();
    auto derivationPath = freshAddress->getDerivationPath().value_or("");
    EXPECT_EQ(derivationPath, "0/0");
    auto bechAddress = freshAddress->toString();
    EXPECT_EQ(bechAddress, address);
    EXPECT_EQ(freshAddressStr.substr(0, hrp.size()), hrp);
    EXPECT_EQ(freshAddressStr, address);
    auto balance = wait(account->getBalance());
    // TODO: send BTC on address bc1qshh6mmfq8fucahzxe4zc7pc5zdhk6zkt4uv8md to implement the rest of tests ?
    EXPECT_EQ(balance->toLong(), 0);

    // TODO: In the meantime ...
    // Reference: https://github.com/bitcoin/bitcoin/blob/master/src/test/data/tx_valid.json
    auto mockTx = "0100000000010100010000000000000000000000000000000000000000000000000000000000000000000000ffffffff01e8030000000000001976a9144c9c3dfac4207d5d8cb89df5722cb3d712385e3f88ac02483045022100cfb07164b36ba64c1b1e8c7720a56ad64d96f6ef332d3d37f9cb3c96477dc44502200a464cd7a9cf94cd70f66ce4f4f0625ef650052c7afcfe29d7d7e01830ff91ed012103596d3451025c19dbbdeb932d6bf8bfb4ad499b95b6f88db8899efac102e5fc7100000000";
    auto parsedTx = BitcoinLikeTransaction::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(mockTx), 0);
    EXPECT_EQ(mockTx, hex::toString(parsedTx->serialize()));
}

TEST_F(BitcoinP2WPKHTransactions, ParseSignedTx) {
    auto hash = "c3dd55c86d02ad9d4b0e748c219fd15b79f21c6d5e38f5fe84a453a7f9e37494";
    auto sender = "bc1qh4kl0a0a3d7su8udc2rn62f8w939prqpl34z86";
    std::vector<std::string> receivers {"bc1qh4kl0a0a3d7su8udc2rn62f8w939prqpl34z86", "bc1qry3crfssh8w6guajms7upclgqsfac4fs4g7nwj"};

    auto signedTx = "0100000000010154302828c224cb00c797038d4cbc9e06b5a38d832e879c67de523bd714a8c37c0000000000ffffff00021027000000000000160014bd6df7f5fd8b7d0e1f8dc2873d29277162508c01e82f010000000000160014192381a610b9dda473b2dc3dc0e3e80413dc553002483045022100dc57387b377550476a04f3147d915e57e396ab5ce41f8629f0aebd0f9a472876022025920a6a9d80aa6b31aedd10dfbd16d0b2eb8e449a93b70059e0cec7ac2a40ca012102fbba978d75f5fc4e7987840b78033e0e4797c7776c070037422616e622f8e6dc00000000";
    auto parsedTx = BitcoinLikeTransaction::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(signedTx), 0);
    EXPECT_EQ(signedTx, hex::toString(parsedTx->serialize()));
    EXPECT_EQ(parsedTx->getHash(), hash);
    EXPECT_GT(parsedTx->getInputs().size(), 0);
    EXPECT_EQ(parsedTx->getInputs()[0]->getAddress().value_or(""), sender);
    EXPECT_GT(parsedTx->getOutputs().size(), 0);
    EXPECT_EQ(parsedTx->getOutputs()[0]->getAddress().value_or(""), receivers[0]);
    EXPECT_EQ(parsedTx->getOutputs()[1]->getAddress().value_or(""), receivers[1]);
}

TEST_F(BitcoinP2WSHTransactions, CreateStandardP2WSHWithOneOutput) {
    auto builder = tx_builder();
    auto freshAddress = wait(account->getFreshPublicAddresses())[0];
    auto hrp = Bech32Factory::newBech32Instance("btc").getValue()->getBech32Params().hrp;
    auto freshAddressStr = std::dynamic_pointer_cast<BitcoinLikeAddress>(freshAddress)->toBech32();
    EXPECT_EQ(freshAddressStr.substr(0, hrp.size()), hrp);
    auto balance = wait(account->getBalance());
    // TODO: send BTC on address bc1q7kggee5ry2xpr0nul42grqul6ygyll5afj34xxehaftcc0pty4sqvud0h2 to implement the rest of tests ?
    EXPECT_EQ(balance->toLong(), 0);

    // TODO: In the meantime ...
    // Reference: https://github.com/bitcoin/bitcoin/blob/master/src/test/data/tx_valid.json
    auto mockTx = "0100000000010100010000000000000000000000000000000000000000000000000000000000000000000000ffffffff01e8030000000000001976a9144c9c3dfac4207d5d8cb89df5722cb3d712385e3f88ac02483045022100aa5d8aa40a90f23ce2c3d11bc845ca4a12acd99cbea37de6b9f6d86edebba8cb022022dedc2aa0a255f74d04c0b76ece2d7c691f9dd11a64a8ac49f62a99c3a05f9d01232103596d3451025c19dbbdeb932d6bf8bfb4ad499b95b6f88db8899efac102e5fc71ac00000000";
    auto parsedTx = BitcoinLikeTransaction::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(mockTx), 0);
    EXPECT_EQ(mockTx, hex::toString(parsedTx->serialize()));
}

TEST_F(BitcoinP2WSHTransactions, ParseSignedTx) {
    auto hash = "94236be7808bc824ae3c531ee4cdf26559d6cf40cb6541f38153c54701fb0ea7";
    auto sender = "bc1qsqe7gwppjngklwjd2lp8kde0cpglerldadudcua3efr7a0tf3ucs995hxa";
    std::vector<std::string> receivers {"bc1qmxalhet27lzt07tq5uxhagg8z4538k095f4s5u2znh67p972v5mswsecmn", "bc1qhfrga6jkrvnrq8jv7606xj77fttqeq08puze7pzu4xnejvgqffxs96j3cx"};

    auto signedTx = "0100000000010180e68831516392fcd100d186b3c2c7b95c80b53c77e77c35ba03a66b429a2a1b0000000000ffffffff028096980000000000220020d9bbfbe56af7c4b7f960a70d7ea107156913d9e5a26b0a71429df5e097ca65378096980000000000220020ba468eea561b26301e4cf69fa34bde4ad60c81e70f059f045ca9a79931004a4d024730440220032521802a76ad7bf74d0e2c218b72cf0cbc867066e2e53db905ba37f130397e02207709e2188ed7f08f4c952d9d13986da504502b8c3be59617e043552f506c46ff83275163ab68210392972e2eb617b2388771abe27235fd5ac44af8e61693261550447a4c3e39da98ac00000000";
    auto parsedTx = BitcoinLikeTransaction::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(signedTx), 0);
    EXPECT_EQ(signedTx, hex::toString(parsedTx->serialize()));
    EXPECT_EQ(parsedTx->getHash(), hash);
    EXPECT_GT(parsedTx->getInputs().size(), 0);
    EXPECT_EQ(parsedTx->getInputs()[0]->getAddress().value_or(""), sender);
    EXPECT_GT(parsedTx->getOutputs().size(), 0);
    EXPECT_EQ(parsedTx->getOutputs()[0]->getAddress().value_or(""), receivers[0]);
    EXPECT_EQ(parsedTx->getOutputs()[1]->getAddress().value_or(""), receivers[1]);
}

TEST_F(BitcoinCoinSelectionTransactions, PickOneUTXOWithoutChange) {

        auto builder = tx_builder();
        //1997970
        builder->sendToAddress(api::Amount::fromLong(currency, 20000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 0));
        auto f = builder->build();
        auto tx = ::wait(f);

        EXPECT_EQ(tx->getInputs().size(), 1);
        EXPECT_EQ(tx->getInputs().at(0)->getValue()->toLong(), 20000000);

        EXPECT_EQ(tx->getOutputs().size(), 1);
        EXPECT_EQ(tx->getOutputs().at(0)->getValue()->toLong(), 20000000);
}

TEST_F(BitcoinCoinSelectionTransactions, PickOneUTXOWithChange) {
        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 20000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
        int64_t fees = 10;
        builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
        auto f = builder->build();
        auto tx = ::wait(f);

        auto firstInput = tx->getInputs().at(0)->getValue()->toLong();
        EXPECT_EQ(tx->getInputs().size(), 1);
        EXPECT_EQ(firstInput, 30000000);

        EXPECT_EQ(tx->getOutputs().size(), 2);
        auto firstOutput = tx->getOutputs().at(0)->getValue()->toLong();
        EXPECT_EQ(firstOutput, 20000000);
//        cout<<" >> Fees are : "<< firstInput - firstOutput - tx->getOutputs().at(1)->getValue()->toLong()<<endl;
//        EXPECT_EQ(tx->getOutputs().at(1)->getValue()->toLong(), firstInput - firstOutput - tx->getEstimatedSize().Max * fees);

}

TEST_F(BitcoinCoinSelectionTransactions, PickMultipleUTXO) {

        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 70000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
        auto f = builder->build();
        auto tx = ::wait(f);

        EXPECT_LE(tx->getInputs().size(), 3);
        int64_t total = 0;
        for (auto& input : tx->getInputs()) {
                total += input->getValue()->toLong();
        }

        EXPECT_EQ(total, 80000000);

        EXPECT_EQ(tx->getOutputs().size(), 2);
        EXPECT_EQ(tx->getOutputs().at(0)->getValue()->toLong(), 70000000);
}

TEST_F(BitcoinCoinSelectionTransactions, PickAllUTXO) {

        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 145000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
        auto f = builder->build();
        auto tx = ::wait(f);

        EXPECT_EQ(tx->getInputs().size(), 5);
        int64_t total = 0;
        for (auto& input : tx->getInputs()) {
                total += input->getValue()->toLong();
        }
        EXPECT_EQ(total, 150000000);

        EXPECT_EQ(tx->getOutputs().size(), 2);
        EXPECT_EQ(tx->getOutputs().at(0)->getValue()->toLong(), 145000000);
}

TEST_F(BitcoinCoinSelectionTransactions, PickUTXOWithMergeOutputs) {

        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 80000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::MERGE_OUTPUTS, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
        auto f = builder->build();
        auto tx = ::wait(f);

        EXPECT_EQ(tx->getInputs().size(), 4);
        int64_t total = 0;
        for (auto& input : tx->getInputs()) {
                total += input->getValue()->toLong();
        }
        EXPECT_EQ(total, 100000000);

        EXPECT_EQ(tx->getOutputs().size(), 2);
        EXPECT_EQ(tx->getOutputs().at(0)->getValue()->toLong(), 80000000);
}