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

#include <gtest/gtest.h>
#include <async/QtThreadDispatcher.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <NativePathResolver.hpp>
#include <unordered_set>
#include <src/wallet/pool/WalletPool.hpp>
#include <CoutLogPrinter.hpp>
#include <src/api/DynamicObject.hpp>
#include <wallet/common/CurrencyBuilder.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>
#include <utils/JSONUtils.h>
#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>
#include <async/wait.h>
#include <BitcoinLikeStringXpubProvider.h>
#include <api/BitcoinLikeExtendedPublicKeyProvider.hpp>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>

using namespace ledger::core;
using namespace ledger::qt;

static const std::string XPUB_1 = "xpub6EedcbfDs3pkzgqvoRxTW6P8NcCSaVbMQsb6xwCdEBzqZBronwY3Nte1Vjunza8f6eSMrYvbM5CMihGo6SbzpHxn4R5pvcr2ZbZ6wkDmgpy";

static const std::string SAMPLE_TRANSACTION = "{\"hash\":\"1700a71b4f833cc8f7a774c605a974c4f1e3c144b67ee18ec1bafe4e5d38aa47\",\"received_at\":\"2017-03-07T19:31:11Z\",\"lock_time\":0,\"block\":{\"hash\":\"000000000000000001c6fa7ba025f37f7e58e028b6c10a9c0bc832907e533515\",\"height\":456232,\"time\":\"2017-03-08T00:56:40Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"c423803deff204448535f9fe031aec757c24a509535cc114a86cc8f072977721\",\"output_index\":0,\"value\":2194550,\"address\":\"1Jwxc4n2gVebkxypFaTBSW51r2uREUtuHF\",\"script_signature\":\"47304402201df6c9075747d01c5ac7818843d7d26916212bb2cb63e3d980de65cabd4a96cf02204d8235067ae53c7e57d2fdb4664fd94a584039e1bf387b276c666d65cce842b30121035b108b92a2818149085bee328da06f5ae2c26cfb45b5ccab2dc5addc8761c789\"}],\"outputs\":[{\"output_index\":0,\"value\":2174550,\"address\":\"1HuZgLc5VFnjE4T4hqFURLvTDCUdDHq12S\",\"script_hex\":\"76a914b972ebc512c30746b4d53c58c7ea1e6b91b32e1e88ac\"}],\"fees\":20000,\"amount\":2174550,\"confirmations\":12804}";
static const std::string SAMPLE_TRANSACTION_2 = "{\"hash\":\"9585deb93fd3f769ad9727f31eba00fbc8eebfc96fe7be28e45ab10919ac0c72\",\"received_at\":\"2017-06-05T08:19:03Z\",\"lock_time\":469845,\"block\":{\"hash\":\"00000000000000000185ad65d72e2cd7614b6877344a965db193d4a9e7fa050b\",\"height\":469846,\"time\":\"2017-06-05T08:28:04Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"8608a13077d59639219b8ce65a66a049a75290cf2adcb7e6b2b38c7639124af5\",\"output_index\":1,\"value\":7738800,\"address\":\"1eWDVgQNWv5HATC7FPLbfJMi5ynVrMqsk\",\"script_signature\":\"483045022100bfef6fc0315cc8b5ef3435babcd3d1605d3996beb5b7eddb324b504cbfe3e4800220246cdd6316a7c2ab46253007ca2ce343e1fcba98744cc390eda1f46ab9bd74f9012103576667b83e7ad8af4dd890f3ff646eac4dc1cced9fc9dbcb85f1f6912337e84d\"},{\"input_index\":1,\"output_hash\":\"fa375899d875ba30fc3cc3ca4b5f383c7971a732393a7b6872fd29627cdf89d8\",\"output_index\":1,\"value\":60000000,\"address\":\"1M7sGxht7RUpkvXVY9N8NLWpvTk4aXer5r\",\"script_signature\":\"47304402204913df69cf49efbc2b505bd6056c6cc9057475c2427f032c8763499893754efc022049d3c9746dba096327f90b80932ef61bf90297e7890c526f99cb6e03f67033b9012102484018b108f9c9290fcaa5186eac0b3c24fd3a053aa1886ce8c4382d36617137\"},{\"input_index\":2,\"output_hash\":\"01609f4dec0f530ac40d81d966ff3257e4a504a8be0a3c3ddb8276a79e1da346\",\"output_index\":1,\"value\":316055,\"address\":\"1MWpTHpBUmafHiNnnxkkBgcspA7WMfgDCT\",\"script_signature\":\"473044022027c2843fdba15783fbfad292e9a43457b7032dc3f29b64453b330c2219eff036022075e54b376732e65d1284a0bd0e7c308818bda5198a2c885b736aa4bfb814d43301210384751e50350fb30f9ee91515b9dea0f1d952af13b09c5015186735b40574cc74\"},{\"input_index\":3,\"output_hash\":\"a7c6767fb954113bd48a7dd6b3464edeb44f08bf900c7415e27816a771d05268\",\"output_index\":0,\"value\":10000000,\"address\":\"1EuSDsLFQPjQuBgXUMCVAivBvrWoxPYuRz\",\"script_signature\":\"473044022003c0127b6870089b2a82aae0e30d700042d27b785cee757734ecc3e03b2ccf75022027468e7d2a9cf5968c312f4f2ef2f90bd3649aef372ec9aa5bb2b6c7af92bd1001210265142cc4212bc7cb918a91c3ff09fa6af890742efeee2a2d6b2ba116e6801425\"},{\"input_index\":4,\"output_hash\":\"67ddfc767b80fdac367360015bec16d583b594726d737f68a494bf88a8863193\",\"output_index\":0,\"value\":7920000,\"address\":\"1LrdWPT9ACz4bKApmucLxVtEdGmBjzsw3w\",\"script_signature\":\"483045022100aa7176bb40946fc812392ce87afc0a09682f75baf463652f6037aedb41500fbc02204a769e00d6f347748dafe389d4045970d850d5abeb97a546e3bbe1e1092c222c012103c2319da06f8223ba81044b0b517023b683997b5d6e820e87cd1662733d5011ba\"},{\"input_index\":5,\"output_hash\":\"67cb4ee1d131f83d2a1eaf784f5d63874702a1821b97886829badf9ee3b0101e\",\"output_index\":1,\"value\":10800000,\"address\":\"1N3k6LsSeZnvxJ4hYSnfcgBc6UqaGKgaR9\",\"script_signature\":\"483045022100e8a378d2ab82fbfc440b8400688718de1233752a3eefa833d41c9c352e8545d90220355f670cba768c2448d8dfebed263451aafe704df3908baed742e21f062d61840121030771f16140ea3c4319389a17914a8c929d1a532098fc8e23fe9ff1cba4edf02e\"},{\"input_index\":6,\"output_hash\":\"1a3f259f1db0823495890589b7f9e08d0211136bbeb893d4334ed5ef20a61818\",\"output_index\":1,\"value\":4883698,\"address\":\"163LXLgW3hkFfz8vDViUyPmXBFxT4P36TP\",\"script_signature\":\"4730440220466758c19b7be521889644c52a7201bf0c59fec7dceb5009f4c82b65199f284e02200506cd2d57fbdf61327f75df68c3e5714e7bdb618127987a17c6f2245fffaaea0121020bf202afffd525a11f7961c5f912c10b9cb041f71d0fff9c10a5de302b2c15e6\"},{\"input_index\":7,\"output_hash\":\"bf8cb8f6c1679764a63e943bdf965df0a922338dfb8c3408da957d662a3e7988\",\"output_index\":0,\"value\":5000000,\"address\":\"1FrhU5oWL6yiEu1CEbkXv1CEy6H7bEH6Sd\",\"script_signature\":\"48304502210086f4a79d6a97f23d572a8fafd3cd6ba7ee9331def6cd8623f2fd7b7f7ab4665c0220612a387178fe5d03217a8dff4b42d8b43e88bd4a3e43e10488485347f772e4f80121021106e00e05a7dc9b2e2c19f9b7653e80801d9735e1d6151c7286cf9b58c66b73\"},{\"input_index\":8,\"output_hash\":\"9bd650b03fa02c6a10e9a1f4a6a6efadf72134c7d786f84a8fc81a64573533e2\",\"output_index\":1,\"value\":4000000,\"address\":\"1KHgg8cCL37BATCM2JxjTFEvNsNUUE4kbg\",\"script_signature\":\"483045022100a4876199592533e77552299b94cae313ed099d1d07ccb0e8fc373b0a0e98a93c022071691da7daf8f739b81fb15e5c61690597b3de33c177ffd4d0027aaf698dbece0121029b87e782ff47a0d920edf87269635f5d4abce4e4e8959c40d98ef4d862cdae00\"},{\"input_index\":9,\"output_hash\":\"910feba8b2ccfb3ae6b7ab3cbd011681aca19595ee52844324b9f9c9929d754e\",\"output_index\":0,\"value\":5504231,\"address\":\"1FkZFkezsvtr5r72UCe9znoxxd8p625Eju\",\"script_signature\":\"483045022100b927e5922fe9bc5ba55de2ab9d0c56040e8cb6a6a67ba3fbfb81683eec304bfb02203e191f670dcf335a45269577441f7493a747c70977ccfeda195161d0dcd9cc70012103cda22d07be70defb2cc7bcbfd9af1b295cc0f00cd63aa8c18f6606454e1c11aa\"},{\"input_index\":10,\"output_hash\":\"84ea916b70b68433ed42da753977dd698814afdb9fbdf4105640df01cdd08cd2\",\"output_index\":1,\"value\":7900000,\"address\":\"1DRE89VftrcveVefszd4CSKmz6a9u5FS9Y\",\"script_signature\":\"483045022100eed5679e0e7b964504e37dabb61db1591740a01b686e6fd9ae53e24abd2c34dd022016fce52f8323a4bb09165b7babd32fa0f9fa710047ab869d4cba27b2fec809400121022108792d48c078563ba64078c0215a456051a9757df56b7273ce4a49fe8ea069\"},{\"input_index\":11,\"output_hash\":\"16965e0b01a159ee1cccbea463f59bcf213b2573f12a04bec88c2f260ee0659a\",\"output_index\":0,\"value\":20000000,\"address\":\"1AQQAAMgCatVW61rkswrutxvtJFxfs8MXx\",\"script_signature\":\"47304402201378efafaa58448f04b4710b4e48b078d461c5a1e60733371dab48a0f77e0a70022068d867f5461b0950c384eb6f9852c0a519a1e0210c2c70ea234154915cc18f4a012102db14b3a536a7d3286a6227aa9109a2cd08fbfa091597762562efbfe02e67649c\"},{\"input_index\":12,\"output_hash\":\"0a4aafbca924a640543b6fbbaa4a2297f66d3186860c0451c9293f42f34c2ee7\",\"output_index\":0,\"value\":3000000,\"address\":\"1EknqXyqNK7edoZR5BNHCsKUvn3ENKsqXj\",\"script_signature\":\"47304402202f2c62f9cafb09bbe39471bbdbad0fd5b4422c5a526fd1b6e5363b8d728a60d702206b409673df98e02c1a0acfcd49b0000a17da7ad22c6cf17f0e488308ccfe66cd012102c1be8cc0f32c0681fac38f391fd320b682dc91ad9fb6f8e0fed0d78627a427bf\"},{\"input_index\":13,\"output_hash\":\"6742cca46980523fd2f60b403f9b69922bd7fd57194d9db8b427440b28314382\",\"output_index\":0,\"value\":6201679,\"address\":\"1KgC9idNAPhsMyA2BsLENuQLz26eriuzBK\",\"script_signature\":\"47304402202073bd7a019eead4eb99f17bdc046dc58ed4d34df57aef7e8eb6d70a003c2ce0022032daf0c6a9b1b1b9df37eee0db9484b1e002f1d61cd20ff5363fcd5e00f25e9901210274596701e1218aa71139e6cbde442bc4f6e90d334106d4695e64a4c3ae0dc4ac\"}],\"outputs\":[{\"output_index\":0,\"value\":50000,\"address\":\"114T52xRqSbbAjLE9NcgkpqwakoJx5129\",\"script_hex\":\"76a9140002e1256226bc6154f6f94881b8dfc27362d61288ac\"},{\"output_index\":1,\"value\":95000,\"address\":\"173z8Kpt5ZDqgs1hVt1FGixsDcoFZ4uM1g\",\"script_hex\":\"76a914426145f3979e19d98dc3aafc94764745a809a15188ac\"},{\"output_index\":2,\"value\":9526042,\"address\":\"17NXj3YUK51PqT6TLH18N4MU5MqYuHTWGM\",\"script_hex\":\"76a91445e323439460696c2b864c16ef05484ff166e6da88ac\"},{\"output_index\":3,\"value\":1000016,\"address\":\"1N4YcMQxTj6ZnZnQ8wGXm4ew26kUdTJ5o8\",\"script_hex\":\"76a914e70646905a5bd233991eea9f558281f335e8c8fe88ac\"},{\"output_index\":4,\"value\":162774,\"address\":\"18H7d3CcgLB6BysJ14JxcmvpU7MUhjdjXt\",\"script_hex\":\"76a9144fd51026f5558ffc76825905f774724f0e2cfe3188ac\"},{\"output_index\":5,\"value\":52740000,\"address\":\"18kWw3MruU5kobKmD7mo42jkZNKgRsK9ZK\",\"script_hex\":\"76a9145503c0aa4795facc3aff6f3ff2b84e228a5225c888ac\"},{\"output_index\":6,\"value\":59770000,\"address\":\"1BUtgvRWVRUjqyMe5LhX1YA5baGszZbWaf\",\"script_hex\":\"76a91472f7a2d24cc7bc5fb12e06cf5a8fdf7828e56cfc88ac\"},{\"output_index\":7,\"value\":92000,\"address\":\"1CYMMpZJYjuXf9PWKifSNhgMGXsr9hZcuc\",\"script_hex\":\"76a9147e9745f35f6b3d1694a8d35b8a71dba1c0da825388ac\"},{\"output_index\":8,\"value\":600000,\"address\":\"1DzxtJXPZW23XXMmBBgP9MCieWftJPazDZ\",\"script_hex\":\"76a9148e97de88ce86d081b0afcf86e05bd965d8d4fff888ac\"},{\"output_index\":9,\"value\":112014,\"address\":\"1Gqfdh8JmgCXr6FwwDxybT9rmrzpuzHB2Q\",\"script_hex\":\"76a914adbe1a1012f28f871389b53546989c6b3dabc9ff88ac\"},{\"output_index\":10,\"value\":24789000,\"address\":\"1HhqAgtTMniZHKYVBV7oFru8migSfQPMm\",\"script_hex\":\"76a9140328bd2b83be40a25740b039c6cfd58462813dac88ac\"},{\"output_index\":11,\"value\":141680,\"address\":\"1M7k7mFpyE63P7UcMrrfcZXfEGDdCuFARG\",\"script_hex\":\"76a914dca900dd2a47f3fcda51f6a45a9147f131f823cd88ac\"},{\"output_index\":12,\"value\":500000,\"address\":\"1NHMCMzriXLVctAkhnHM88ZToAUtBQCeCg\",\"script_hex\":\"76a914e97227d10fe29e613f3923d4cb6f8b21915c892188ac\"},{\"output_index\":13,\"value\":63592,\"address\":\"36TLShVi5eBFSetDF5Yj9mgdzEF9NHPzYS\",\"script_hex\":\"a914344243ddc994ab6cfea02caa94ccbcfe0037fcd287\"}],\"fees\":3622345,\"amount\":149642118,\"confirmations\":4}";
static const std::string SAMPLE_TRANSACTION_3 = "{\"hash\":\"5d0fcab290dac66ee9da149a948d1e30d16d2f8f852eccaf4394021deeaa7b61\",\"received_at\":\"2017-06-07T16:05:33Z\",\"lock_time\":0,\"block\":null,\"inputs\":[{\"input_index\":0,\"output_hash\":\"bd8eabb80b020c5b05b0d2a69b64a81380049c6102477698ea7b73d13776458c\",\"output_index\":0,\"value\":5449257,\"address\":\"1DiKs1fV7HjcDdZNJTG7GFVyXbVe834uax\",\"script_signature\":\"4730440221009b37fa67b7320f597e0f4b2c1aaab705927e888e4c1b0fff45a3f7e394041b1c021f600cbea1955e6d57b38c737d52ca68a58eeb69d870f06c8cbbcaf2c0eefcc5012103fcc5efc0c3a0dd30b9f64d99ee372a43d9985791c49ed92b66e74a3315767c28\"}],\"outputs\":[{\"output_index\":0,\"value\":20000,\"address\":\"1TipsnxGEhPwNxhAwKouhHgTUnmmuYg9P\",\"script_hex\":\"76a914050dbaa82baeaa15ab5e31385fd880a8f25ef42288ac\"},{\"output_index\":1,\"value\":5350185,\"address\":\"1NkDgmWnuMYXrqXyFgQcAfaxJt93Sm5fHd\",\"script_hex\":\"76a914ee871e04c6f17f2e4bc73d73233c761544f3eefb88ac\"}],\"fees\":79072,\"amount\":5370185,\"confirmations\":0}";



static std::vector<std::pair<std::string, std::string>> XPUBS = {
    {"main", "xpub6D4waFVPfPCpRvPkQd9A6n65z3hTp6TvkjnBHG5j2MCKytMuadKgfTUHqwRH77GQqCKTTsUXSZzGYxMGpWpJBdYAYVH75x7yMnwJvra1BUJ"}
};
static auto XPUB_PROVIDER = api::BitcoinLikeExtendedPublicKeyProvider::fromBitcoinLikeBase58ExtendedPublicKeyProvider(
        std::make_shared<BitcoinLikeStringXpubProvider>(XPUBS)
);

static const std::string TX_1 = "{\"hash\":\"666613fd82459f94c74211974e74ffcb4a4b96b62980a6ecaee16af7702bbbe5\",\"received_at\":\"2015-06-22T13:31:27Z\",\"lock_time\":0,\"block\":{\"hash\":\"00000000000000000b2df329293632a46c6b6a6c066fe5a617b264ff6edfa6a4\",\"height\":362035,\"time\":\"2015-06-22T13:31:27Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"e4486c80c17f41ff16b607044a1bf8a4d4aa3985156396b040290812575e71e1\",\"output_index\":0,\"value\":890000,\"address\":\"1KMbwcH1sGpHetLwwQVNMt4cEZB5u8Uk4b\",\"script_signature\":\"473044022009b9163abea9783d2ed1f9d4f9d0c16d624f5bba013728a1551877b9f934e5ad02201b5803ab33d7f19fa8d71106242467b21be592b24f1397569b970ceacf62e8870121020a90429f7e8964be1595789f7cedcb80b850405993e59c7ff244d83a7ec4ac4a\"},{\"input_index\":1,\"output_hash\":\"1565ad6bccb0c9d6c4771d8df49a2b6c5022b6f9dcd307582c7e2fe37e8b4f48\",\"output_index\":0,\"value\":13970000,\"address\":\"15zxN5EuryNJRpgo6rBRvgiYiDYix6EiHK\",\"script_signature\":\"483045022100ec78b471f6760c429044c76d34a084417967e090fada2cc797ef03bf0144a034022072b9e31f69b4a2a37e9d7c02a2e08d8a8195d6fd6da17ef8e2392c8c786015a30121021775300e7f3ba1c538349eb35f24a47314b53e6cc57e7ed154a073fabb00f3d7\"},{\"input_index\":2,\"output_hash\":\"e4486c80c17f41ff16b607044a1bf8a4d4aa3985156396b040290812575e71e1\",\"output_index\":1,\"value\":100000,\"address\":\"12BSjgZhjNi5JVh418h88nVkEBph9tn9JH\",\"script_signature\":\"483045022100ce0fe58a1297c6c96a3059ec11be82351046bbe977f4f5780a6b9710e871069f0220677bbd8364dbf8ef4b9817b404668c26fc6ae98141fbfa5b5c2e60b814e5e6da012102647367b0928f40d80ccec71104692e34f9e2abd4a0e6b7cad6faa1c9e51f3292\"},{\"input_index\":3,\"output_hash\":\"2f7e07679ea5babfe105c1a7c202efa8791819ef59e57ead25911f147dd90eaf\",\"output_index\":1,\"value\":23990000,\"address\":\"1PMiVc1UXestrbbaMqSwDuUrRRhTfy1UZc\",\"script_signature\":\"47304402201047d9e4eb4ce52377ec54a2e75fa0e6ecae6dc349f8a252f5eda4ea2bc8603c022002c016500d49a896aa79a8fb0235b0de27dd32a52feb7ab71c6a67bd44ddfa3501210249511220e84a3b3172895a000a012429c67a78fb33207a2650ec7e69fee21709\"},{\"input_index\":4,\"output_hash\":\"b9325a52d3373f725e8d39d04ff67531490af2221fd94bd3973ffc1b8208053b\",\"output_index\":1,\"value\":37019500,\"address\":\"13XU2CS1YgkDu7N1Xv7zoyypX1B7t4XHWS\",\"script_signature\":\"483045022100c37b6d5f22d6ba176ee0d06be46bcc86a7bd7ef6301bf7983e5f3fcac95ad414022029537ecc338f9a50316d287411bb69362539122b78ff121ecaa5c5ae79796dd70121025df76c1fc25d3d839a27a9b1f1b7051f612b929de2d19b48cbeade18d6439730\"},{\"input_index\":5,\"output_hash\":\"2f7e07679ea5babfe105c1a7c202efa8791819ef59e57ead25911f147dd90eaf\",\"output_index\":0,\"value\":1000000,\"address\":\"1PA2cZx9wZDo3xnd1HpRiLwTAtrpUFgPMw\",\"script_signature\":\"483045022100b257fb66f2ab29e80a4c6cfd4a07fcbb3aafedd02cb0ad76fdd455c34cf94c4502202dbf2392b466b6fe6e9bd4f1fc85077a639a07f3ec956fc45deefc6a5bdf5e26012103d766a986dbeab84e8827d6997e1e7630b909c8f6d16455485df2070ae21ebfe6\"},{\"input_index\":6,\"output_hash\":\"f5a1c29cbce3f0b99a38f46989bda4365a1f32941c8dcf415d76e35cbd0b1cbb\",\"output_index\":0,\"value\":10000,\"address\":\"14nMrPfMVLtXK3csgphyGZPsBPaPzNqtnQ\",\"script_signature\":\"483045022100ff1efd876eaf5813c9d8df381cc46f5b0d283e9b560461988a26465da1fb9fff0220567adc5ab53667e8864cba76a4540b2b5a6e8600495e7117d11e1747f458cdc801210319e6c9ac4a6f0d99d7dc2311dc177506ae3ad251b92a7345beb4add203cc342d\"},{\"input_index\":7,\"output_hash\":\"f871f48ddcf9ae17e5f0ff00121b36bfe54d329a9cbc38afa77db7c4540af7a4\",\"output_index\":1,\"value\":60000,\"address\":\"1EYLmfsvCVGsBmmiUmgJzoQG3MCou2Mh7J\",\"script_signature\":\"47304402202765acf7ea410e5712d5c5adacad0bcb0d71ef1b22b30860a42e851f745338e8022001de1738785ca43291b50e51281d20ecc4f451df9797e5c95641136f267cd4fe01210297af3c6d7e5f0c96a3ac24d059f838d20792362fbf945a10aa3cddeeacdb3cc4\"},{\"input_index\":8,\"output_hash\":\"2adc12ff62c58b56bb9fd5bf6cb702d2d07f39e7fd06b44ed69181c202f1ac2b\",\"output_index\":0,\"value\":9630000,\"address\":\"1Bmpme646SNGa1jjjYAfuijdyBNJXLGEh\",\"script_signature\":\"47304402201a44458da76e5359a904e40c6318e3930d5951a6649df9a846807c5107652d2b0220112e5d86a87e5f54d2d58b234392cf39cd7e9e3313a2f6850f89a9c60fc95d250121037b9922b3bea53333486d9b068ffafba088e4730e4b4081d6a71063b9aefda8ca\"},{\"input_index\":9,\"output_hash\":\"2217f57871f2a973e1544828b3fafe79d7bc8168e875ae50ed2304d7abd5be3a\",\"output_index\":1,\"value\":5000000,\"address\":\"1PhMRy1tEDiPYLxLGGXqVUSYr2qQ5NL8fm\",\"script_signature\":\"47304402200bf3579d28b96dd59a466ad5ae89ca4aef3ca5a473683f27a63d71cb7e90203702206900566e2f84abf4839e87b1bd52969e81c9629e728be9249f2be9e8c4e60bed01210325d655fc73c30551f6cf5a80bd224ccf676d51431d58e3e39fb0ca8ddbf4d891\"},{\"input_index\":10,\"output_hash\":\"7fa17586ffb7775bd19be72f568cba521c6e97eb166c9702096daf4856a2a4d3\",\"output_index\":0,\"value\":2000000,\"address\":\"1JpVd5LgkDw9o43bFsT9kMSMtqEKuYY69m\",\"script_signature\":\"473044022072655d94fef5fcecaa6153ef89a8ed93361665392d222b275072578af623f01a0220232f49fe3ad36eeca863884ac70a5800964239a6851e25f12cba1ebd97c4cdc301210357885d4578594861cdc10e1fbe9456a1df2461038936a8b913f7c727d1becb1b\"},{\"input_index\":11,\"output_hash\":\"8ff2a1c170a728fbd0ceb2947b120f8f1d0d08a52567b05882c02b41b866e357\",\"output_index\":1,\"value\":540000,\"address\":\"12YvvHBzc4r9bS5EaEs27AgqYTjX1guUer\",\"script_signature\":\"4730440220485e6884a4490a318bc8e8d04d93f283819ece82dfed6bf0434e3aadf8f8855d02202c46fa38e627beae7accd47adcb0ac3d75eaa05f9e962967ecedd1ca5870405c012102fc03f7162b34700e03a2da4bcbf9799eb6eec61ae01548e79db50765fb943932\"},{\"input_index\":12,\"output_hash\":\"7b7bb35e059cfabe79ea3b72caaa866fbf624d645768e198c5d021622262be80\",\"output_index\":0,\"value\":20000000,\"address\":\"12LVnPYsDzZKN3F4gdFaBJ1WSkkSxje6AS\",\"script_signature\":\"483045022100e7539137c60e00b5320ff1f7f4a0e1539a538c87fb133731dba7c8bc22fcfc7402200c4a2db4b203c6a84e7d41446d4ebc3c66ac203cf0ce93285887d8c7e23bb7680121030f25559e42584669dc19661b398a60660258e0bd3edfc16f3f38d1171c59c1fc\"},{\"input_index\":13,\"output_hash\":\"904fb0ba6c126a5fc50507c0f953e8e48363eb34798e82945971fc3634e2b305\",\"output_index\":0,\"value\":68384000,\"address\":\"1PKsemsjKHX8r2AWNHWTXhDq4waRMQpFEA\",\"script_signature\":\"47304402202013318f36716b9405627529a42d76901256798f5c8ac3a248b4b17f97e3640e02205cf81ccf9f0827679d462b6b20d954e0556f50cd52861eb50494841e9240aea40121034bd0aa6737a5d7b1571a3ac24afafd9225109feb0b79f57c1cf2271cfb6d2f59\"},{\"input_index\":14,\"output_hash\":\"a7e29eb70660fb23d0f6ecededf91519d9d17fa2ed2624a44f80f39e7f1cc634\",\"output_index\":1,\"value\":10000,\"address\":\"16VLgkWhKRSgLhW8sCra88qRuWXjpaZRtM\",\"script_signature\":\"47304402200b78257a162efa7410dc74cd6b29a8a80677a38f8ff2a3632fb472e59296d321022038de9ebe64e2a96454c4ddadb75f32d9b7f7026e2c3fefb66ebe60106569a5c30121025748f109aa23987b80a4ac8a681881e0b5532387ac0db5226ec1931bad68ebc0\"}],\"outputs\":[{\"output_index\":0,\"value\":182593500,\"address\":\"1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7\",\"script_hex\":\"76a91485efaded203a798edc46cd458f0714136f6d0acb88ac\"}],\"fees\":10000,\"amount\":182593500,\"confirmations\":109504}";
static const std::string TX_2 = "{\"hash\":\"a5fb8b23c1131850569874b8d8592800211b3d0392753b84d2d5f9f53b7e09fc\",\"received_at\":\"2015-06-22T15:58:30Z\",\"lock_time\":0,\"block\":{\"hash\":\"00000000000000000198e024d936c87807b4198f9de7105015036ce785fa2bdc\",\"height\":362055,\"time\":\"2015-06-22T15:58:30Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"666613fd82459f94c74211974e74ffcb4a4b96b62980a6ecaee16af7702bbbe5\",\"output_index\":0,\"value\":182593500,\"address\":\"1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7\",\"script_signature\":\"473044022018ac8a44412d66f489138c3e8f9196b60dba1c24fb715dd8c3d66921bcc13f4702201f222fd3e25fe8f3807347650ae7b41451c078c9a8fc2e5b7d82d6a928a8c363012103da611b1fcacc0056ceb5489ee951b523e52de7ff1902fd1c6f9c212a542da297\"}],\"outputs\":[{\"output_index\":0,\"value\":182483500,\"address\":\"18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1\",\"script_hex\":\"76a9144ed14e321713e1c97056643a233b968d34b2231188ac\"},{\"output_index\":1,\"value\":100000,\"address\":\"1NMfmPC9yHBe5US2CUwWARPRM6cDP6N86m\",\"script_hex\":\"76a914ea4351fd2a0a2cd62ab264d9f0b1997696a632f488ac\"}],\"fees\":10000,\"amount\":182583500,\"confirmations\":109484}";
static const std::string TX_3 = "{\"hash\":\"a207285f69f5966f47c93ea0b76c1d751912823ed5f58ad23d8e5600260f39f6\",\"received_at\":\"2016-10-05T11:13:03Z\",\"lock_time\":0,\"block\":{\"hash\":\"0000000000000000007dd92a1fa59e272bdade5ec99cc1d9d75d19e030e34e02\",\"height\":432965,\"time\":\"2016-10-05T11:13:03Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"6dcce0034dce58bb10e29ab05c98e24797e3a010628d94be9da204de97d873cf\",\"output_index\":1,\"value\":50000,\"address\":\"1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7\",\"script_signature\":\"473044022060a8a45ae565ff0fa5621f605f6767950ebe5b0ea853a693cc42be0d3b36e659022011f335167d1f818eb3eb765276e9251886beccbce5165be5d1cc83c1ebd1c215012103da611b1fcacc0056ceb5489ee951b523e52de7ff1902fd1c6f9c212a542da297\"},{\"input_index\":1,\"output_hash\":\"88b71e589120a0019e5f45b4242bc872600deb09b4197dd340ae2af5448c18fd\",\"output_index\":0,\"value\":64516615,\"address\":\"1NdzNgDmaeeTifcGABR7Yn5MGsnpjxobMa\",\"script_signature\":\"483045022100ade3d393b8ccf63d0becdceff2d9076f391d4b84c89b9da402d0db007a89bb7302206d87b3d10218b6666c8cee0b851467878c94a09cf9155f43ff84eead1e8b47a00121039a94d1d8a952b3e751e760f05d948523364323f8b08ba5678c7817063173435a\"}],\"outputs\":[{\"output_index\":0,\"value\":100000,\"address\":\"1JVZAEAJQikwgToSbv9eFgy56jufBs4RKT\",\"script_hex\":\"76a914bfe0a15bbed6211262d3a8d8a891e738bab36ffb88ac\"},{\"output_index\":1,\"value\":64442175,\"address\":\"1QDgRFmoDqaotX3ShdjRQN89qxRTSVQjUp\",\"script_hex\":\"76a914feb0ca09c61adc57bd1c0098f33adebf347bd44e88ac\"}],\"fees\":24440,\"amount\":64542175,\"confirmations\":38574}";
static const std::string TX_4 = "{\"hash\":\"6dcce0034dce58bb10e29ab05c98e24797e3a010628d94be9da204de97d873cf\",\"received_at\":\"2016-10-05T11:13:03Z\",\"lock_time\":0,\"block\":{\"hash\":\"0000000000000000007dd92a1fa59e272bdade5ec99cc1d9d75d19e030e34e02\",\"height\":432965,\"time\":\"2016-10-05T11:13:03Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"4afb3118cadba885e4ff0b238aeb87e9dec16fe8262aadedf3576017042152d5\",\"output_index\":0,\"value\":205245,\"address\":\"1GbovgXDQFyKAbrrHZnvANTLi9fnnuqv6T\",\"script_signature\":\"47304402204c468acbb314477428f927ac396355522f4d8552d673d2ad0ee649ac74d01e6c0220064702479f9272bce803b4b2c95ae06d90ae321ef53d62f63dd6e948757491db012102c83d849494b5a418eefee64a92e3081d0b265f8a432183da99da2fdc74610241\"}],\"outputs\":[{\"output_index\":0,\"value\":142173,\"address\":\"1Kvmzupx8DNNa6mfwk1yuQAdBaCFWYTikR\",\"script_hex\":\"76a914cf9ddf76dbeb046e2b1ddd005f98a6167dedbdd488ac\"},{\"output_index\":1,\"value\":50000,\"address\":\"1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7\",\"script_hex\":\"76a91485efaded203a798edc46cd458f0714136f6d0acb88ac\"}],\"fees\":13072,\"amount\":192173,\"confirmations\":38574}";

static void createWallet(const std::shared_ptr<WalletPool>& pool, const std::string& walletName) {
    soci::session sql(pool->getDatabaseSessionPool()
                          ->getPool());
    WalletDatabaseEntry entry;
    entry.configuration = std::static_pointer_cast<DynamicObject>(DynamicObject::newInstance());
    entry.name = "my_wallet";
    entry.poolName = pool->getName();
    entry.currencyName = "bitcoin";
    entry.updateUid();
    PoolDatabaseHelper::putWallet(sql, entry);
}

static void createAccount(const std::shared_ptr<WalletPool>& pool, const std::string& walletName, int32_t index) {
    soci::session sql(pool->getDatabaseSessionPool()
                          ->getPool());
    auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), walletName, "bitcoin");
    if (!AccountDatabaseHelper::accountExists(sql, walletUid, index))
        AccountDatabaseHelper::createAccount(sql, walletUid, index);
}

static BitcoinLikeWalletDatabase newAccount(const std::shared_ptr<WalletPool>& pool,
                                            const std::string& walletName,
                                            int32_t index,
                                            const std::string& xpub) {
    BitcoinLikeWalletDatabase db(pool, walletName, "bitcoin");
    if (!db.accountExists(index)) {
        createWallet(pool, walletName);
        createAccount(pool, walletName, index);
        db.createAccount(index, xpub);
    }
    return db;
}

TEST(BitcoinWalletDatabase, EmptyWallet) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();
        BitcoinLikeWalletDatabase db(pool, "my_wallet", "bitcoin");

        EXPECT_EQ(db.getAccountsCount(), 0);
        EXPECT_FALSE(db.accountExists(255));
    }
    resolver->clean();
}

TEST(BitcoinWalletDatabase, CreateWalletWithOneAccount) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();

        BitcoinLikeWalletDatabase db(pool, "my_wallet", "bitcoin");

        EXPECT_EQ(db.getAccountsCount(), 0);
        EXPECT_FALSE(db.accountExists(0));

        // We need to create the abstract entry first to satisfy the foreign key constraint
        createWallet(pool, "my_wallet");
        createAccount(pool, "my_wallet", 0);

        db.createAccount(0, XPUB_1);

        EXPECT_EQ(db.getAccountsCount(), 1);
        EXPECT_TRUE(db.accountExists(0));
    }
    resolver->clean();
}

TEST(BitcoinWalletDatabase, CreateWalletWithMultipleAccountAndDelete) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();

        BitcoinLikeWalletDatabase db = newAccount(pool, "my_wallet", 0, XPUB_1);

        EXPECT_EQ(db.getAccountsCount(), 1);
        EXPECT_EQ(db.getNextAccountIndex(), 1);
        for (auto i = 1; i < 100; i++) {
            newAccount(pool, "my_wallet", i, XPUB_1);
        }
        EXPECT_EQ(db.getAccountsCount(), 100);

        soci::session sql(pool->getDatabaseSessionPool()->getPool());

        auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), "my_wallet", "bitcoin");
        EXPECT_EQ(AccountDatabaseHelper::getAccountsCount(sql, walletUid), 100);
        AccountDatabaseHelper::removeAccount(sql, walletUid, 0);
        EXPECT_EQ(AccountDatabaseHelper::getAccountsCount(sql, walletUid), 99);
        EXPECT_EQ(db.getAccountsCount(), 99);
        EXPECT_EQ(db.getNextAccountIndex(), 0);
    }
    resolver->clean();
}

TEST(BitcoinWalletDatabase, PutTransaction) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();

        BitcoinLikeWalletDatabase db = newAccount(pool, "my_wallet", 0, XPUB_1);
        auto transaction = JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION);
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        BitcoinLikeAccountDatabase acc(db.getWalletUid(), 0);
        BitcoinLikeTransactionDatabaseHelper::putTransaction(sql, *transaction);

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
    resolver->clean();
}

TEST(BitcoinWalletDatabase, PutTransactionWithMultipleOutputs) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();

        BitcoinLikeWalletDatabase db = newAccount(pool, "my_wallet", 0, XPUB_1);
        std::vector<BitcoinLikeBlockchainExplorer::Transaction> transactions = {
                *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION),
                *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION_2),
                *JSONUtils::parse<TransactionParser>(SAMPLE_TRANSACTION_3)
        };
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        sql.begin();
        BitcoinLikeAccountDatabase acc(db.getWalletUid(), 0);
        for (auto& transaction : transactions) {
            BitcoinLikeTransactionDatabaseHelper::putTransaction(sql, transaction);
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
    resolver->clean();
}

TEST(BitcoinWalletDatabase, PutOperations) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
                "my_pool",
                Option<std::string>::NONE,
                nullptr,
                nullptr,
                resolver,
                printer,
                dispatcher,
                nullptr,
                backend,
                api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();
        auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
        auto nextIndex = wait(wallet->getNextAccountIndex());
        EXPECT_EQ(nextIndex, 0);
        auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(std::dynamic_pointer_cast<BitcoinLikeWallet>(wallet->asBitcoinLikeWallet())->createNewAccount(nextIndex, XPUB_PROVIDER)));

        std::vector<BitcoinLikeBlockchainExplorer::Transaction> transactions = {
                *JSONUtils::parse<TransactionParser>(TX_1),
                *JSONUtils::parse<TransactionParser>(TX_2),
                *JSONUtils::parse<TransactionParser>(TX_3),
                *JSONUtils::parse<TransactionParser>(TX_4)
        };
        soci::session sql(pool->getDatabaseSessionPool()->getPool());
        for (auto& tx : transactions) {
            account->putTransaction(sql, tx);
        }
        EXPECT_EQ(wallet->getName(), "my_wallet");
    }
    resolver->clean();
}