/*
 *
 * coin_selection_P2PKH.cpp
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 14/06/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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
#include "transaction_test_helper.h"
#include "../../fixtures/coin_selection_xpub_fixtures.h"
#include <api/KeychainEngines.hpp>
#include <api/EstimatedSize.hpp>

struct CoinSelectionP2PKH : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"49'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin_testnet";
        testData.inflate_btc = ledger::testing::coin_selection_xpub::inflate;
    }
};

TEST_F(CoinSelectionP2PKH, PickOneUTXOWithoutChange) {

        auto builder = tx_builder();
        //1997970
        builder->sendToAddress(api::Amount::fromLong(currency, 20000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 0));
        auto f = builder->build();
        auto tx = uv::wait(f);

        EXPECT_EQ(tx->getInputs().size(), 1);
        EXPECT_EQ(tx->getInputs().at(0)->getValue()->toLong(), 20000000);

        EXPECT_EQ(tx->getOutputs().size(), 1);
        EXPECT_EQ(tx->getOutputs().at(0)->getValue()->toLong(), 20000000);
}

TEST_F(CoinSelectionP2PKH, DISABLED_PickOneUTXOWithChange) {

        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 20000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
        int64_t fees = 10;
        builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
        auto f = builder->build();
        auto tx = uv::wait(f);

        auto firstInput = tx->getInputs().at(0)->getValue()->toLong();
        EXPECT_EQ(tx->getInputs().size(), 1);
        EXPECT_EQ(firstInput, 30000000);

        EXPECT_EQ(tx->getOutputs().size(), 2);
        auto firstOutput = tx->getOutputs().at(0)->getValue()->toLong();
        EXPECT_EQ(firstOutput, 20000000);
//        cout<<" >> Fees are : "<< firstInput - firstOutput - tx->getOutputs().at(1)->getValue()->toLong()<<endl;
//        EXPECT_EQ(tx->getOutputs().at(1)->getValue()->toLong(), firstInput - firstOutput - tx->getEstimatedSize().Max * fees);

}

TEST_F(CoinSelectionP2PKH, PickMultipleUTXO) {

        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 70000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
        auto f = builder->build();
        auto tx = uv::wait(f);

        EXPECT_LE(tx->getInputs().size(), 3);
        int64_t total = 0;
        for (auto& input : tx->getInputs()) {
                total += input->getValue()->toLong();
        }

        EXPECT_EQ(total, 80000000);

        EXPECT_EQ(tx->getOutputs().size(), 2);
        EXPECT_EQ(tx->getOutputs().at(0)->getValue()->toLong(), 70000000);
}

TEST_F(CoinSelectionP2PKH, PickAllUTXO) {

        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 145000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
        auto f = builder->build();
        auto tx = uv::wait(f);

        EXPECT_EQ(tx->getInputs().size(), 5);
        int64_t total = 0;
        for (auto& input : tx->getInputs()) {
                total += input->getValue()->toLong();
        }
        EXPECT_EQ(total, 150000000);

        EXPECT_EQ(tx->getOutputs().size(), 2);
        EXPECT_EQ(tx->getOutputs().at(0)->getValue()->toLong(), 145000000);
}

TEST_F(CoinSelectionP2PKH, PickUTXOWithMergeOutputs) {

        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(currency, 80000000), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(api::BitcoinLikePickingStrategy::MERGE_OUTPUTS, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(currency, 10));
        auto f = builder->build();
        auto tx = uv::wait(f);

        EXPECT_EQ(tx->getInputs().size(), 4);
        int64_t total = 0;
        for (auto& input : tx->getInputs()) {
                total += input->getValue()->toLong();
        }
        EXPECT_EQ(total, 100000000);

        EXPECT_EQ(tx->getOutputs().size(), 2);
        EXPECT_EQ(tx->getOutputs().at(0)->getValue()->toLong(), 80000000);
}

TEST_F(CoinSelectionP2PKH, CompareUTXOPickingStrategies) {

    auto buildTx = [=](const api::BitcoinLikePickingStrategy & strategy, int64_t amount) -> std::shared_ptr<api::BitcoinLikeTransaction> {
        auto builder = tx_builder();
        builder->sendToAddress(api::Amount::fromLong(wallet->getCurrency(), amount), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        builder->pickInputs(strategy, 0xFFFFFFFF);
        builder->setFeesPerByte(api::Amount::fromLong(wallet->getCurrency(), 41));
        auto f = builder->build();
        auto tx = uv::wait(f);
        return tx;
    };

    auto balance = uv::wait(account->getBalance());
    auto currentAmount = balance->toLong();
    auto iterations = 20;
    for (auto index = 0; index < iterations; index++) {
        currentAmount -= balance->toLong() / iterations;
        auto merge = buildTx(api::BitcoinLikePickingStrategy::MERGE_OUTPUTS, currentAmount);
        auto deep = buildTx(api::BitcoinLikePickingStrategy::DEEP_OUTPUTS_FIRST, currentAmount);
        EXPECT_LE(deep->getFees()->toLong(), merge->getFees()->toLong());
        EXPECT_LE(deep->getOutputs().size(), merge->getOutputs().size());
        auto optimize = buildTx(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, currentAmount);
        EXPECT_LE(optimize->getFees()->toLong(), deep->getFees()->toLong());
        EXPECT_LE(optimize->getOutputs().size(), deep->getOutputs().size());
    }
}




