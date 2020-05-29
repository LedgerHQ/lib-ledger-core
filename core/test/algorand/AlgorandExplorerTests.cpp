/*
 * AlgorandExplorerTests
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
#include "AlgorandTestFixtures.hpp"
#include <wallet/algorand/AlgorandBlockchainExplorer.hpp>
#include <api/AlgorandConfigurationDefaults.hpp>

#include "../integration/BaseFixture.h"

using namespace ledger::testing::algorand;
using namespace ledger::core::algorand;

class AlgorandExplorerTest : public BaseFixture {
public:
    void SetUp() override {
        BaseFixture::SetUp();

        auto worker = dispatcher->getSerialExecutionContext("worker");
        auto client = std::make_shared<HttpClient>(api::AlgorandConfigurationDefaults::ALGORAND_API_ENDPOINT, http, worker);

        explorer = std::make_shared<BlockchainExplorer>(
                        worker,
                        client,
                        networks::getAlgorandNetworkParameters("algorand-testnet"),
                        std::make_shared<DynamicObject>());
    }

    void TearDown() override {
        BaseFixture::TearDown();
        explorer.reset();
    }
    
    std::shared_ptr<BlockchainExplorer> explorer;
};


TEST_F(AlgorandExplorerTest, GetBlock) {

    uint64_t blockHeight = 6000000; // Some arbitrary block
    std::chrono::system_clock::time_point blockTime(std::chrono::seconds(1586345796));

    api::Block block = ::wait(explorer->getBlock(blockHeight));

    EXPECT_EQ(block.height, blockHeight);
    EXPECT_EQ(block.blockHash, "SWM7OIULX7F7KMGZSGU54FCEVVVCV6XWT6CEG7EH7WI4LHWHNB7A");
    EXPECT_EQ(block.time, blockTime);
}

TEST_F(AlgorandExplorerTest, GetAccount) {

    auto address = ::algorand::Address(OBELIX_ADDRESS);
    model::Account account = ::wait(explorer->getAccount(address.toString()));

    EXPECT_EQ(account.address, address.toString());
    EXPECT_GT(account.round, 6000000);
    EXPECT_GT(account.amount, 0);
    EXPECT_GT(account.amountWithoutPendingRewards, 0);
    // FIXME Figure out rewards on testnet
    //EXPECT_GT(account->pendingRewards, 0); // May fail if rewards have been claimed very recently, but ultra unlikely
    //EXPECT_GT(account->rewards, 0);
  
    EXPECT_FALSE(account.assetsAmounts.empty());
    EXPECT_NE(account.assetsAmounts.find(342836), account.assetsAmounts.end());
    EXPECT_EQ(account.assetsAmounts.at(342836).creatorAddress, address);

    EXPECT_NE(account.createdAssets.find(342836), account.createdAssets.end());
    assertSameAssetParams(testAsset(), account.createdAssets.at(342836));
}

TEST_F(AlgorandExplorerTest, GetPaymentTransaction) {
    auto txRef = paymentTransaction();
    model::Transaction tx = ::wait(explorer->getTransactionById(*txRef.header.id));
    assertSameTransaction(txRef, tx);
}

TEST_F(AlgorandExplorerTest, GetAssetConfigTransaction) {
    auto txRef = assetConfigTransaction();
    model::Transaction tx = ::wait(explorer->getTransactionById(*txRef.header.id));
    assertSameTransaction(txRef, tx);
}

TEST_F(AlgorandExplorerTest, GetAssetTransferTransaction) {
    auto txRef = assetTransferTransaction();
    model::Transaction tx = ::wait(explorer->getTransactionById(*txRef.header.id));
    assertSameTransaction(txRef, tx);
}

TEST_F(AlgorandExplorerTest, GetAccountTransactions) {

    auto address = "RGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL56XZM"; // Obelix
    model::TransactionsBulk txs = ::wait(explorer->getTransactionsForAddress(address, 0));

    for (const auto& tx : txs.transactions) {
        if (*tx.header.id == PAYMENT_TX_ID) {
            assertSameTransaction(paymentTransaction(), tx);
        } else if (*tx.header.id == ASSET_CONFIG_TX_ID) {
            assertSameTransaction(assetConfigTransaction(), tx);
        } else if (*tx.header.id == ASSET_TRANSFER_TX_ID) {
            assertSameTransaction(assetTransferTransaction(), tx);
        }
    }

    if (txs.hasNext == true) EXPECT_EQ(txs.transactions.size(), 100);
}
