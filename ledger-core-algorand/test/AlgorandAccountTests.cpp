/*
 * AlgorandAccountTests
 *
 * Created by Rémi Barjon on 02/06/2020.
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
#include <algorand/AlgorandAccount.hpp>
#include <algorand/AlgorandLikeCurrencies.hpp>
#include <algorand/AlgorandWallet.hpp>
#include <algorand/AlgorandWalletFactory.hpp>
#include <algorand/database/AlgorandAccountDatabaseHelper.hpp>
#include <algorand/model/transactions/AlgorandTransaction.hpp>
#include <algorand/utils/B64String.hpp>

#include <algorand/api/AlgorandAssetAmount.hpp>

#include <core/api/AccountCreationInfo.hpp>

#include <integration/WalletFixture.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

using namespace ledger::core;

const auto OBELIX = std::string("RGX5XA7DWZOZ5SLG4WQSNIFKIG4CNX4VOH23YCEX56523DQEAL3QL56XZM");
const auto ADDR1 = std::string("6ENXFMQRRIF6KD7HXE47HUHCJXEUKGGRGR6LXSX7RRZBTMVI5NUDOQDTNE");
const auto ADDR2 = std::string("6ENXFMQRRIF6KD7HXE47HUHCJXEUKGGRGR6LXSX7RRZBTMVI5NUDOQDTNF");

std::vector<uint64_t> month2020 {
        1577836800, // 01/01/2020
        1580515200, // 01/02/2020
        1583020800, // 01/03/2020
        1585699200, // 01/04/2020
        1588291200, // 01/05/2020
        1590969600, // 01/06/2020
        1593561600, // 01/07/2020
};

namespace {

    auto makeHeader(
            const std::string& sender,
            uint64_t fee,
            const std::string& type,
            uint64_t timestamp,
            const std::string& id)
    {
        return [&]() {
            auto header = algorand::model::Transaction::Header(
                    fee,
                    0,
                    std::string("testnet-v1.0"),
                    algorand::B64String("SGO1GKSzyE7IEPItTxCByw9x8FmnrCDexi9/cOUJOiI="),
                    {},
                    0,
                    {},
                    {},
                    algorand::Address(sender),
                    type
            );
            header.timestamp = timestamp;
            header.id = id;
            return header;
        }();
    }

    auto makePayment(
            const std::string& sender,
            uint64_t fee,
            uint64_t timestamp,
            const std::string& id,
            uint64_t amount,
            const Option<std::string>& close,
            const std::string& recipient,
            uint64_t closeAmount,
            uint64_t fromRewards,
            uint64_t recipientRewards,
            uint64_t closeRewards)
    {
        const auto header = [&]() {
            auto header = makeHeader(sender, fee, algorand::model::constants::pay, timestamp, id);
            header.fromRewards = fromRewards;
            return header;
        }();

        const auto details = [&]() {
            auto details = algorand::model::PaymentTxnFields(
                    amount,
                    close.map<algorand::Address>([](const std::string& s) { return algorand::Address(s); }),
                    algorand::Address(recipient)
            );
            details.closeRewards = closeRewards;
            details.receiverRewards = recipientRewards;
            details.closeAmount = closeAmount;
            return details;
        }();

        return algorand::model::Transaction(header, details);
    }

    auto makeAssetTransfer(
            const std::string& sender,
            uint64_t fee,
            uint64_t timestamp,
            const std::string& id,
            uint64_t amount,
            const Option<std::string>& close,
            const std::string& recipient,
            uint64_t asset)

    {
        const auto header =
            makeHeader(sender, fee, algorand::model::constants::axfer, timestamp, id);

        const auto details = algorand::model::AssetTransferTxnFields::transfer(
                amount,
                close.map<algorand::Address>([](const std::string& s) { return algorand::Address(s); }),
                algorand::Address(recipient),
                asset
        );

        return algorand::model::Transaction(header, details);
    }

    auto makeAssetFreeze(
            const std::string& sender,
            uint64_t fee,
            uint64_t timestamp,
            const std::string& id,
            bool frozen,
            const std::string& frozenAddress,
            uint64_t asset)
    {
        const auto header =
            makeHeader(sender, fee, algorand::model::constants::afreeze, timestamp, id);

        const auto details = algorand::model::AssetFreezeTxnFields(
                frozen,
                algorand::Address(frozenAddress),
                asset
        );

        return algorand::model::Transaction(header, details);
    }

    /// +------------+---------------------+-------------------+
    /// |    Date    |   Balance(malgos)   |   Balance(ASA)    |
    /// +------------+---------------------+-------------------+
    /// | 01/01/2020 |       10 000        |         0         |
    /// | 01/02/2020 |        4 004        |         0         |
    /// | 01/03/2020 |       34 024        |         0         |
    /// | 01/04/2020 |       34 024        |        30         |
    /// | 01/05/2020 |       28 024        |        10         |
    /// | 01/06/2020 |            0        |        10         |
    /// | 01/07/2020 |       26 026        |        10         |
    /// +------------+---------------------+-------------------+
    auto makeTransactions()
    {
        auto transactions = std::vector<algorand::model::Transaction>();
        transactions.push_back(makePayment(ADDR1, 1000, month2020[0], "id1", 10000, {}, OBELIX, 0, 0, 0, 0)); // +10000 malgos, +0 ASA
        transactions.push_back(makePayment(OBELIX, 2000, month2020[1], "id2", 4000, {}, ADDR2, 0, 4, 0, 0)); // -5996 malgos, +0 ASA
        transactions.push_back(makeAssetFreeze(ADDR1, 1000, month2020[2], "id3", true, OBELIX, 2)); // +0 malgos, +0 ASA
        transactions.push_back(makePayment(ADDR2, 1000, month2020[2], "id4", 0, OBELIX, OBELIX, 30000, 0, 0, 20)); // +30020 malgos, +0 ASA
        transactions.push_back(makeAssetTransfer(ADDR1, 1000, month2020[3], "id5",  30, {}, OBELIX, 2)); // +0 malgos, +30 ASA
        transactions.push_back(makeAssetTransfer(OBELIX, 1000, month2020[4], "id6", 20, {}, ADDR2, 2)); // -1000 malgos, -20 ASA
        transactions.push_back(makeAssetFreeze(OBELIX, 5000, month2020[4], "id7", false, OBELIX, 2)); // -5000 malgos, +0 ASA
        transactions.push_back(makePayment(OBELIX, 1000, month2020[5], "id8", 0, ADDR1, ADDR1, 27024, 0, 0, 0)); // -28024 malgos, +0 ASA
        transactions.push_back(makePayment(ADDR1, 1000, month2020[6], "id9", 26024, OBELIX, ADDR2, 26024, 0, 0, 2)); // +26026 malgos, +0 ASA

        return transactions;
    }

} // namespace

class AlgorandAccountTest : public WalletFixture<WalletFactory>
{
public:
    void SetUp() override {
        WalletFixture::SetUp();

        const auto currency = currencies::algorand();
        registerCurrency(currency);

        accountInfo = api::AccountCreationInfo(1, {}, {}, { { std::begin(OBELIX), std::end(OBELIX) } }, {}); // TODO: FIXEME

        wallet = std::dynamic_pointer_cast<algorand::Wallet>(wait(walletStore->createWallet("algorand", currency.name, api::DynamicObject::newInstance())));
        account = createAccount<algorand::Account>(wallet, accountInfo.index, accountInfo);

        accountUid = algorand::AccountDatabaseHelper::createAccountUid(wallet->getWalletUid(), accountInfo.index);

        soci::session sql(account->getWallet()->getDatabase()->getPool());
        for (const auto& txn : makeTransactions()) {
            account->putTransaction(sql, txn);
        }
    }

    void TearDown() override {
        WalletFixture::TearDown();

        wallet.reset();
        account.reset();
    }

    std::shared_ptr<algorand::Wallet> wallet;
    std::shared_ptr<algorand::Account> account;

    api::AccountCreationInfo accountInfo;
    std::string accountUid;
};

TEST_F(AlgorandAccountTest, algosBalanceHistory)
{
    const auto start = "2019-12-29T00:00:00Z";
    const auto end = "2020-08-01T00:00:00Z";
    const auto period = api::TimePeriod::MONTH;

    const auto expected = std::vector<uint64_t>
    {
        10000,
        4004,
        34024,
        34024,
        28024,
        0,
        26026,
        26026
    };

    const auto balances = ::wait(account->getBalanceHistory(start, end, period));
    for (auto i = 0; i < balances.size(); ++i) {
        EXPECT_EQ(std::stoull(balances[i]->toString()), expected[i]);
    }
}

TEST_F(AlgorandAccountTest, assetBalanceHistory)
{
    const auto id = std::string("2");
    const auto start = "2019-12-29T00:00:00Z";
    const auto end = "2020-08-01T00:00:00Z";
    const auto period = api::TimePeriod::MONTH;

    const auto expected = std::vector<uint64_t>
    {
        0,
        0,
        0,
        30,
        10,
        10,
        10,
        10
    };

    const auto balances = ::wait(account->getAssetBalanceHistory(id, start, end, period));
    for (auto i = 0; i < balances.size(); ++i) {
        EXPECT_EQ(std::stoull(balances[i].amount), expected[i]);
    }
}

