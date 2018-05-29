/*
 *
 * wallet_database_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/05/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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

#include "BaseFixture.h"

static const std::string XPUB_1 = "xpub6EedcbfDs3pkzgqvoRxTW6P8NcCSaVbMQsb6xwCdEBzqZBronwY3Nte1Vjunza8f6eSMrYvbM5CMihGo6SbzpHxn4R5pvcr2ZbZ6wkDmgpy";

static const std::string SAMPLE_TRANSACTION = "{\"hash\":\"1700a71b4f833cc8f7a774c605a974c4f1e3c144b67ee18ec1bafe4e5d38aa47\",\"received_at\":\"2017-03-07T19:31:11Z\",\"lock_time\":0,\"block\":{\"hash\":\"000000000000000001c6fa7ba025f37f7e58e028b6c10a9c0bc832907e533515\",\"height\":456232,\"time\":\"2017-03-08T00:56:40Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"c423803deff204448535f9fe031aec757c24a509535cc114a86cc8f072977721\",\"output_index\":0,\"value\":2194550,\"address\":\"1Jwxc4n2gVebkxypFaTBSW51r2uREUtuHF\",\"script_signature\":\"47304402201df6c9075747d01c5ac7818843d7d26916212bb2cb63e3d980de65cabd4a96cf02204d8235067ae53c7e57d2fdb4664fd94a584039e1bf387b276c666d65cce842b30121035b108b92a2818149085bee328da06f5ae2c26cfb45b5ccab2dc5addc8761c789\"}],\"outputs\":[{\"output_index\":0,\"value\":2174550,\"address\":\"1HuZgLc5VFnjE4T4hqFURLvTDCUdDHq12S\",\"script_hex\":\"76a914b972ebc512c30746b4d53c58c7ea1e6b91b32e1e88ac\"}],\"fees\":20000,\"amount\":2174550,\"confirmations\":12804}";
static const std::string SAMPLE_TRANSACTION_2 = "{\"hash\":\"9585deb93fd3f769ad9727f31eba00fbc8eebfc96fe7be28e45ab10919ac0c72\",\"received_at\":\"2017-06-05T08:19:03Z\",\"lock_time\":469845,\"block\":{\"hash\":\"00000000000000000185ad65d72e2cd7614b6877344a965db193d4a9e7fa050b\",\"height\":469846,\"time\":\"2017-06-05T08:28:04Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"8608a13077d59639219b8ce65a66a049a75290cf2adcb7e6b2b38c7639124af5\",\"output_index\":1,\"value\":7738800,\"address\":\"1eWDVgQNWv5HATC7FPLbfJMi5ynVrMqsk\",\"script_signature\":\"483045022100bfef6fc0315cc8b5ef3435babcd3d1605d3996beb5b7eddb324b504cbfe3e4800220246cdd6316a7c2ab46253007ca2ce343e1fcba98744cc390eda1f46ab9bd74f9012103576667b83e7ad8af4dd890f3ff646eac4dc1cced9fc9dbcb85f1f6912337e84d\"},{\"input_index\":1,\"output_hash\":\"fa375899d875ba30fc3cc3ca4b5f383c7971a732393a7b6872fd29627cdf89d8\",\"output_index\":1,\"value\":60000000,\"address\":\"1M7sGxht7RUpkvXVY9N8NLWpvTk4aXer5r\",\"script_signature\":\"47304402204913df69cf49efbc2b505bd6056c6cc9057475c2427f032c8763499893754efc022049d3c9746dba096327f90b80932ef61bf90297e7890c526f99cb6e03f67033b9012102484018b108f9c9290fcaa5186eac0b3c24fd3a053aa1886ce8c4382d36617137\"},{\"input_index\":2,\"output_hash\":\"01609f4dec0f530ac40d81d966ff3257e4a504a8be0a3c3ddb8276a79e1da346\",\"output_index\":1,\"value\":316055,\"address\":\"1MWpTHpBUmafHiNnnxkkBgcspA7WMfgDCT\",\"script_signature\":\"473044022027c2843fdba15783fbfad292e9a43457b7032dc3f29b64453b330c2219eff036022075e54b376732e65d1284a0bd0e7c308818bda5198a2c885b736aa4bfb814d43301210384751e50350fb30f9ee91515b9dea0f1d952af13b09c5015186735b40574cc74\"},{\"input_index\":3,\"output_hash\":\"a7c6767fb954113bd48a7dd6b3464edeb44f08bf900c7415e27816a771d05268\",\"output_index\":0,\"value\":10000000,\"address\":\"1EuSDsLFQPjQuBgXUMCVAivBvrWoxPYuRz\",\"script_signature\":\"473044022003c0127b6870089b2a82aae0e30d700042d27b785cee757734ecc3e03b2ccf75022027468e7d2a9cf5968c312f4f2ef2f90bd3649aef372ec9aa5bb2b6c7af92bd1001210265142cc4212bc7cb918a91c3ff09fa6af890742efeee2a2d6b2ba116e6801425\"},{\"input_index\":4,\"output_hash\":\"67ddfc767b80fdac367360015bec16d583b594726d737f68a494bf88a8863193\",\"output_index\":0,\"value\":7920000,\"address\":\"1LrdWPT9ACz4bKApmucLxVtEdGmBjzsw3w\",\"script_signature\":\"483045022100aa7176bb40946fc812392ce87afc0a09682f75baf463652f6037aedb41500fbc02204a769e00d6f347748dafe389d4045970d850d5abeb97a546e3bbe1e1092c222c012103c2319da06f8223ba81044b0b517023b683997b5d6e820e87cd1662733d5011ba\"},{\"input_index\":5,\"output_hash\":\"67cb4ee1d131f83d2a1eaf784f5d63874702a1821b97886829badf9ee3b0101e\",\"output_index\":1,\"value\":10800000,\"address\":\"1N3k6LsSeZnvxJ4hYSnfcgBc6UqaGKgaR9\",\"script_signature\":\"483045022100e8a378d2ab82fbfc440b8400688718de1233752a3eefa833d41c9c352e8545d90220355f670cba768c2448d8dfebed263451aafe704df3908baed742e21f062d61840121030771f16140ea3c4319389a17914a8c929d1a532098fc8e23fe9ff1cba4edf02e\"},{\"input_index\":6,\"output_hash\":\"1a3f259f1db0823495890589b7f9e08d0211136bbeb893d4334ed5ef20a61818\",\"output_index\":1,\"value\":4883698,\"address\":\"163LXLgW3hkFfz8vDViUyPmXBFxT4P36TP\",\"script_signature\":\"4730440220466758c19b7be521889644c52a7201bf0c59fec7dceb5009f4c82b65199f284e02200506cd2d57fbdf61327f75df68c3e5714e7bdb618127987a17c6f2245fffaaea0121020bf202afffd525a11f7961c5f912c10b9cb041f71d0fff9c10a5de302b2c15e6\"},{\"input_index\":7,\"output_hash\":\"bf8cb8f6c1679764a63e943bdf965df0a922338dfb8c3408da957d662a3e7988\",\"output_index\":0,\"value\":5000000,\"address\":\"1FrhU5oWL6yiEu1CEbkXv1CEy6H7bEH6Sd\",\"script_signature\":\"48304502210086f4a79d6a97f23d572a8fafd3cd6ba7ee9331def6cd8623f2fd7b7f7ab4665c0220612a387178fe5d03217a8dff4b42d8b43e88bd4a3e43e10488485347f772e4f80121021106e00e05a7dc9b2e2c19f9b7653e80801d9735e1d6151c7286cf9b58c66b73\"},{\"input_index\":8,\"output_hash\":\"9bd650b03fa02c6a10e9a1f4a6a6efadf72134c7d786f84a8fc81a64573533e2\",\"output_index\":1,\"value\":4000000,\"address\":\"1KHgg8cCL37BATCM2JxjTFEvNsNUUE4kbg\",\"script_signature\":\"483045022100a4876199592533e77552299b94cae313ed099d1d07ccb0e8fc373b0a0e98a93c022071691da7daf8f739b81fb15e5c61690597b3de33c177ffd4d0027aaf698dbece0121029b87e782ff47a0d920edf87269635f5d4abce4e4e8959c40d98ef4d862cdae00\"},{\"input_index\":9,\"output_hash\":\"910feba8b2ccfb3ae6b7ab3cbd011681aca19595ee52844324b9f9c9929d754e\",\"output_index\":0,\"value\":5504231,\"address\":\"1FkZFkezsvtr5r72UCe9znoxxd8p625Eju\",\"script_signature\":\"483045022100b927e5922fe9bc5ba55de2ab9d0c56040e8cb6a6a67ba3fbfb81683eec304bfb02203e191f670dcf335a45269577441f7493a747c70977ccfeda195161d0dcd9cc70012103cda22d07be70defb2cc7bcbfd9af1b295cc0f00cd63aa8c18f6606454e1c11aa\"},{\"input_index\":10,\"output_hash\":\"84ea916b70b68433ed42da753977dd698814afdb9fbdf4105640df01cdd08cd2\",\"output_index\":1,\"value\":7900000,\"address\":\"1DRE89VftrcveVefszd4CSKmz6a9u5FS9Y\",\"script_signature\":\"483045022100eed5679e0e7b964504e37dabb61db1591740a01b686e6fd9ae53e24abd2c34dd022016fce52f8323a4bb09165b7babd32fa0f9fa710047ab869d4cba27b2fec809400121022108792d48c078563ba64078c0215a456051a9757df56b7273ce4a49fe8ea069\"},{\"input_index\":11,\"output_hash\":\"16965e0b01a159ee1cccbea463f59bcf213b2573f12a04bec88c2f260ee0659a\",\"output_index\":0,\"value\":20000000,\"address\":\"1AQQAAMgCatVW61rkswrutxvtJFxfs8MXx\",\"script_signature\":\"47304402201378efafaa58448f04b4710b4e48b078d461c5a1e60733371dab48a0f77e0a70022068d867f5461b0950c384eb6f9852c0a519a1e0210c2c70ea234154915cc18f4a012102db14b3a536a7d3286a6227aa9109a2cd08fbfa091597762562efbfe02e67649c\"},{\"input_index\":12,\"output_hash\":\"0a4aafbca924a640543b6fbbaa4a2297f66d3186860c0451c9293f42f34c2ee7\",\"output_index\":0,\"value\":3000000,\"address\":\"1EknqXyqNK7edoZR5BNHCsKUvn3ENKsqXj\",\"script_signature\":\"47304402202f2c62f9cafb09bbe39471bbdbad0fd5b4422c5a526fd1b6e5363b8d728a60d702206b409673df98e02c1a0acfcd49b0000a17da7ad22c6cf17f0e488308ccfe66cd012102c1be8cc0f32c0681fac38f391fd320b682dc91ad9fb6f8e0fed0d78627a427bf\"},{\"input_index\":13,\"output_hash\":\"6742cca46980523fd2f60b403f9b69922bd7fd57194d9db8b427440b28314382\",\"output_index\":0,\"value\":6201679,\"address\":\"1KgC9idNAPhsMyA2BsLENuQLz26eriuzBK\",\"script_signature\":\"47304402202073bd7a019eead4eb99f17bdc046dc58ed4d34df57aef7e8eb6d70a003c2ce0022032daf0c6a9b1b1b9df37eee0db9484b1e002f1d61cd20ff5363fcd5e00f25e9901210274596701e1218aa71139e6cbde442bc4f6e90d334106d4695e64a4c3ae0dc4ac\"}],\"outputs\":[{\"output_index\":0,\"value\":50000,\"address\":\"114T52xRqSbbAjLE9NcgkpqwakoJx5129\",\"script_hex\":\"76a9140002e1256226bc6154f6f94881b8dfc27362d61288ac\"},{\"output_index\":1,\"value\":95000,\"address\":\"173z8Kpt5ZDqgs1hVt1FGixsDcoFZ4uM1g\",\"script_hex\":\"76a914426145f3979e19d98dc3aafc94764745a809a15188ac\"},{\"output_index\":2,\"value\":9526042,\"address\":\"17NXj3YUK51PqT6TLH18N4MU5MqYuHTWGM\",\"script_hex\":\"76a91445e323439460696c2b864c16ef05484ff166e6da88ac\"},{\"output_index\":3,\"value\":1000016,\"address\":\"1N4YcMQxTj6ZnZnQ8wGXm4ew26kUdTJ5o8\",\"script_hex\":\"76a914e70646905a5bd233991eea9f558281f335e8c8fe88ac\"},{\"output_index\":4,\"value\":162774,\"address\":\"18H7d3CcgLB6BysJ14JxcmvpU7MUhjdjXt\",\"script_hex\":\"76a9144fd51026f5558ffc76825905f774724f0e2cfe3188ac\"},{\"output_index\":5,\"value\":52740000,\"address\":\"18kWw3MruU5kobKmD7mo42jkZNKgRsK9ZK\",\"script_hex\":\"76a9145503c0aa4795facc3aff6f3ff2b84e228a5225c888ac\"},{\"output_index\":6,\"value\":59770000,\"address\":\"1BUtgvRWVRUjqyMe5LhX1YA5baGszZbWaf\",\"script_hex\":\"76a91472f7a2d24cc7bc5fb12e06cf5a8fdf7828e56cfc88ac\"},{\"output_index\":7,\"value\":92000,\"address\":\"1CYMMpZJYjuXf9PWKifSNhgMGXsr9hZcuc\",\"script_hex\":\"76a9147e9745f35f6b3d1694a8d35b8a71dba1c0da825388ac\"},{\"output_index\":8,\"value\":600000,\"address\":\"1DzxtJXPZW23XXMmBBgP9MCieWftJPazDZ\",\"script_hex\":\"76a9148e97de88ce86d081b0afcf86e05bd965d8d4fff888ac\"},{\"output_index\":9,\"value\":112014,\"address\":\"1Gqfdh8JmgCXr6FwwDxybT9rmrzpuzHB2Q\",\"script_hex\":\"76a914adbe1a1012f28f871389b53546989c6b3dabc9ff88ac\"},{\"output_index\":10,\"value\":24789000,\"address\":\"1HhqAgtTMniZHKYVBV7oFru8migSfQPMm\",\"script_hex\":\"76a9140328bd2b83be40a25740b039c6cfd58462813dac88ac\"},{\"output_index\":11,\"value\":141680,\"address\":\"1M7k7mFpyE63P7UcMrrfcZXfEGDdCuFARG\",\"script_hex\":\"76a914dca900dd2a47f3fcda51f6a45a9147f131f823cd88ac\"},{\"output_index\":12,\"value\":500000,\"address\":\"1NHMCMzriXLVctAkhnHM88ZToAUtBQCeCg\",\"script_hex\":\"76a914e97227d10fe29e613f3923d4cb6f8b21915c892188ac\"},{\"output_index\":13,\"value\":63592,\"address\":\"36TLShVi5eBFSetDF5Yj9mgdzEF9NHPzYS\",\"script_hex\":\"a914344243ddc994ab6cfea02caa94ccbcfe0037fcd287\"}],\"fees\":3622345,\"amount\":149642118,\"confirmations\":4}";
static const std::string SAMPLE_TRANSACTION_3 = "{\"hash\":\"5d0fcab290dac66ee9da149a948d1e30d16d2f8f852eccaf4394021deeaa7b61\",\"received_at\":\"2017-06-07T16:05:33Z\",\"lock_time\":0,\"block\":null,\"inputs\":[{\"input_index\":0,\"output_hash\":\"bd8eabb80b020c5b05b0d2a69b64a81380049c6102477698ea7b73d13776458c\",\"output_index\":0,\"value\":5449257,\"address\":\"1DiKs1fV7HjcDdZNJTG7GFVyXbVe834uax\",\"script_signature\":\"4730440221009b37fa67b7320f597e0f4b2c1aaab705927e888e4c1b0fff45a3f7e394041b1c021f600cbea1955e6d57b38c737d52ca68a58eeb69d870f06c8cbbcaf2c0eefcc5012103fcc5efc0c3a0dd30b9f64d99ee372a43d9985791c49ed92b66e74a3315767c28\"}],\"outputs\":[{\"output_index\":0,\"value\":20000,\"address\":\"1TipsnxGEhPwNxhAwKouhHgTUnmmuYg9P\",\"script_hex\":\"76a914050dbaa82baeaa15ab5e31385fd880a8f25ef42288ac\"},{\"output_index\":1,\"value\":5350185,\"address\":\"1NkDgmWnuMYXrqXyFgQcAfaxJt93Sm5fHd\",\"script_hex\":\"76a914ee871e04c6f17f2e4bc73d73233c761544f3eefb88ac\"}],\"fees\":79072,\"amount\":5370185,\"confirmations\":0}";


class BitcoinWalletDatabaseTests : public BaseFixture {

};

TEST_F(BitcoinWalletDatabaseTests, EmptyWallet) {
    auto pool = newDefaultPool();
    BitcoinLikeWalletDatabase db(pool, "my_wallet", "bitcoin");

    EXPECT_EQ(db.getAccountsCount(), 0);
    EXPECT_FALSE(db.accountExists(255));
}

TEST_F(BitcoinWalletDatabaseTests, CreateWalletWithOneAccount) {
    auto pool = newDefaultPool();

    BitcoinLikeWalletDatabase db(pool, "my_wallet", "bitcoin");

    EXPECT_EQ(db.getAccountsCount(), 0);
    EXPECT_FALSE(db.accountExists(0));

    // We need to create the abstract entry first to satisfy the foreign key constraint
    createWallet(pool, "my_wallet", "bitcoin", DynamicObject::newInstance());
    createAccount(pool, "my_wallet", 0);

    db.createAccount(0, XPUB_1);

    EXPECT_EQ(db.getAccountsCount(), 1);
    EXPECT_TRUE(db.accountExists(0));
}

TEST_F(BitcoinWalletDatabaseTests, CreateWalletWithMultipleAccountAndDelete) {
    auto pool = newDefaultPool();

    auto currencyName = "bitcoin";
    auto configuration = DynamicObject::newInstance();
    BitcoinLikeWalletDatabase db = newBitcoinAccount(pool, "my_wallet", currencyName, configuration, 0, XPUB_1);

    EXPECT_EQ(db.getAccountsCount(), 1);
    EXPECT_EQ(db.getNextAccountIndex(), 1);
    for (auto i = 1; i < 100; i++) {
        newBitcoinAccount(pool, "my_wallet", currencyName, configuration, i, XPUB_1);
    }
    EXPECT_EQ(db.getAccountsCount(), 100);

    auto database = pool->getDatabaseSessionPool();
    soci::session sql(database->getPool());

    auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), "my_wallet");
    EXPECT_EQ(AccountDatabaseHelper::getAccountsCount(sql, walletUid), 100);
    AccountDatabaseHelper::removeAccount(sql, walletUid, 0);
    EXPECT_EQ(AccountDatabaseHelper::getAccountsCount(sql, walletUid), 99);
    EXPECT_EQ(db.getAccountsCount(), 99);
    EXPECT_EQ(db.getNextAccountIndex(), 0);
}

TEST_F(BitcoinWalletDatabaseTests, PutTransaction) {
    auto pool = newDefaultPool();

    auto currencyName = "bitcoin";
    auto configuration = DynamicObject::newInstance();

    BitcoinLikeWalletDatabase db = newBitcoinAccount(pool, "my_wallet", currencyName, configuration, 0, XPUB_1);
    auto transaction = JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION);
    soci::session sql(pool->getDatabaseSessionPool()->getPool());
    BitcoinLikeAccountDatabase acc(db.getWalletUid(), 0);
    BitcoinLikeTransactionDatabaseHelper::putTransaction(sql, "fake_account", *transaction);

    BitcoinLikeBlockchainExplorer::Transaction dbTransaction;
    if (BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, transaction->hash, dbTransaction)) {
        EXPECT_EQ(transaction->hash, dbTransaction.hash);
        EXPECT_EQ(transaction->lockTime, dbTransaction.lockTime);
        EXPECT_EQ(transaction->receivedAt, dbTransaction.receivedAt);
        EXPECT_EQ(transaction->inputs[0].value.getValue().toUint64(), 2194550UL);
        EXPECT_EQ(transaction->inputs[0].address.getValue(), dbTransaction.inputs[0].address.getValue());
        EXPECT_EQ(transaction->outputs[0].value.toUint64(), dbTransaction.outputs[0].value.toUint64());
        EXPECT_EQ(transaction->outputs[0].address.getValue(), dbTransaction.outputs[0].address.getValue());
    } else {
        FAIL();
    }
}

TEST_F(BitcoinWalletDatabaseTests, PutTransactionWithMultipleOutputs) {
    auto pool = newDefaultPool();

    auto currencyName = "bitcoin";
    auto configuration = DynamicObject::newInstance();

    BitcoinLikeWalletDatabase db = newBitcoinAccount(pool, "my_wallet", currencyName, configuration, 0, XPUB_1);
    std::vector<BitcoinLikeBlockchainExplorer::Transaction> transactions = {
            *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION),
            *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION_2),
            *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION_3)
    };
    soci::session sql(pool->getDatabaseSessionPool()->getPool());
    sql.begin();
    BitcoinLikeAccountDatabase acc(db.getWalletUid(), 0);
    for (auto& transaction : transactions) {
        BitcoinLikeTransactionDatabaseHelper::putTransaction(sql, "fake_account", transaction);
    }
    sql.commit();

    for (auto& transaction : transactions) {
        BitcoinLikeBlockchainExplorer::Transaction dbTx;
        if (BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, transaction.hash, dbTx)) {
            EXPECT_EQ(transaction.hash, dbTx.hash);
            EXPECT_EQ(transaction.lockTime, dbTx.lockTime);
            EXPECT_EQ(transaction.receivedAt.time_since_epoch().count(), dbTx.receivedAt.time_since_epoch().count());
            EXPECT_EQ(transaction.block.isEmpty(), dbTx.block.isEmpty());
            if (transaction.block.nonEmpty()) {
                auto& block = transaction.block.getValue();
                auto& dbBlock = dbTx.block.getValue();
                EXPECT_EQ(block.hash, dbBlock.hash);
                EXPECT_EQ(block.height, dbBlock.height);
                EXPECT_EQ(block.time.time_since_epoch().count(), dbBlock.time.time_since_epoch().count());
            }
            EXPECT_EQ(transaction.inputs.size(), dbTx.inputs.size());
            for (auto i = 0; i < transaction.inputs.size(); i++) {
                auto& input = transaction.inputs[i];
                auto& dbInput = dbTx.inputs[i];
                EXPECT_EQ(input.address.getValueOr(""), dbInput.address.getValueOr(""));
                EXPECT_EQ(input.coinbase.getValueOr(""), dbInput.coinbase.getValueOr(""));
                EXPECT_EQ(input.index, dbInput.index);
                EXPECT_EQ(input.previousTxHash.getValueOr(""), dbInput.previousTxHash.getValueOr(""));
                EXPECT_EQ(input.previousTxOutputIndex.getValueOr(0), dbInput.previousTxOutputIndex.getValueOr(0));
                EXPECT_EQ(input.sequence, dbInput.sequence);
                EXPECT_EQ(input.value.getValue().toUint64(), dbInput.value.getValue().toUint64());
            }
            EXPECT_EQ(transaction.outputs.size(), dbTx.outputs.size());
            for (auto i = 0; i < transaction.outputs.size(); i++) {
                auto& output = transaction.outputs[i];
                auto& dbOutput = dbTx.outputs[i];
                EXPECT_EQ(output.address.getValueOr(""), dbOutput.address.getValueOr(""));
                EXPECT_EQ(output.value.toUint64(), dbOutput.value.toUint64());
                EXPECT_EQ(output.index, dbOutput.index);
                EXPECT_EQ(output.script, dbOutput.script);
            }
        } else {
            FAIL();
        }
    }
}

TEST_F(BitcoinWalletDatabaseTests, PutOperations) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    auto nextIndex = wait(wallet->getNextAccountIndex());
    EXPECT_EQ(nextIndex, 0);
    auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->newAccountWithExtendedKeyInfo(P2PKH_MEDIUM_XPUB_INFO)));

    std::vector<BitcoinLikeBlockchainExplorer::Transaction> transactions = {
            *JSONUtils::parse<TransactionParser>(TX_1),
            *JSONUtils::parse<TransactionParser>(TX_2),
            *JSONUtils::parse<TransactionParser>(TX_3),
            *JSONUtils::parse<TransactionParser>(TX_4)
    };
    soci::session sql(pool->getDatabaseSessionPool()->getPool());
    sql.begin();
    for (auto& tx : transactions) {
        account->putTransaction(sql, tx);
    }
    sql.commit();

    auto query = account->queryOperations()->complete();
    auto queryWithOrders = query->addOrder(api::OperationOrderKey::DATE, false)->addOrder(api::OperationOrderKey::TYPE, false);

    auto operations = wait(std::static_pointer_cast<OperationQuery>(queryWithOrders)->execute());
    EXPECT_EQ(operations.size(), 5);

    auto expectation_0 = std::make_tuple("666613fd82459f94c74211974e74ffcb4a4b96b62980a6ecaee16af7702bbbe5", 15, 1,
                                         890000, "1KMbwcH1sGpHetLwwQVNMt4cEZB5u8Uk4b", 182593500, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7",
                                         182593500, api::OperationType::RECEIVE, 362035);
    auto expectation_1 = std::make_tuple("a5fb8b23c1131850569874b8d8592800211b3d0392753b84d2d5f9f53b7e09fc", 1, 2,
                                         182593500, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7", 182483500, "18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1",
                                         100000, api::OperationType::RECEIVE, 362055);

    auto expectation_2 = std::make_tuple("a5fb8b23c1131850569874b8d8592800211b3d0392753b84d2d5f9f53b7e09fc", 1, 2,
                                         182593500, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7", 182483500, "18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1",
                                         100000, api::OperationType::SEND, 362055);

    auto expectation_3 = std::make_tuple("4450e70656888bd7f5240a9b532eac54db7d72f3b48bfef09fb45a185bb9c570", 1, 2,
                                         182483500, "18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1", 182373500, "139dJmHhFuuhrgbNAPehpokjYHNEtvkxot",
                                         100000, api::OperationType::RECEIVE, 362058);

    auto expectation_4 = std::make_tuple("4450e70656888bd7f5240a9b532eac54db7d72f3b48bfef09fb45a185bb9c570", 1, 2,
                                         182483500, "18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1", 182373500, "139dJmHhFuuhrgbNAPehpokjYHNEtvkxot",
                                         100000, api::OperationType::SEND, 362058);

    #define ASSERT_EXPECTATION(it) \
    { \
        auto& op = operations[it]; \
        auto tx = op->asBitcoinLikeOperation()->getTransaction(); \
        auto& expect = expectation_##it; \
        EXPECT_EQ(tx->getHash(), std::get<0>(expect)); \
        EXPECT_EQ(tx->getInputs().size(), std::get<1>(expect)); \
        EXPECT_EQ(tx->getOutputs().size(), std::get<2>(expect)); \
        EXPECT_EQ(tx->getInputs()[0]->getValue()->toBigInt()->intValue(), std::get<3>(expect)); \
        EXPECT_EQ(tx->getInputs()[0]->getAddress().value(), std::get<4>(expect)); \
        EXPECT_EQ(tx->getOutputs()[0]->getValue()->toBigInt()->intValue(), std::get<5>(expect)); \
        EXPECT_EQ(tx->getOutputs()[0]->getAddress().value(), std::get<6>(expect)); \
        EXPECT_EQ(op->getAmount()->toBigInt()->intValue(), std::get<7>(expect)); \
        EXPECT_EQ(op->getOperationType(), std::get<8>(expect)); \
        EXPECT_EQ(op->getBlockHeight().value(), std::get<9>(expect)); \
        EXPECT_EQ(op->getFees()->toBigInt()->intValue(), 10000); \
    }


    ASSERT_EXPECTATION(0);
    ASSERT_EXPECTATION(1);
    ASSERT_EXPECTATION(2);
    ASSERT_EXPECTATION(3);
    ASSERT_EXPECTATION(4);
}