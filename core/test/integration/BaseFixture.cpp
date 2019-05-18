/*
 *
 * BaseFixture.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/09/2017.
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

#include <utils/FilesystemUtils.h>
#include "BaseFixture.h"
#include "IntegrationEnvironment.h"
#include <utils/hex.h>

api::ExtendedKeyAccountCreationInfo P2PKH_MEDIUM_XPUB_INFO(
        0, {"main"}, {"44'/0'/0'"}, {"xpub6D4waFVPfPCpRvPkQd9A6n65z3hTp6TvkjnBHG5j2MCKytMuadKgfTUHqwRH77GQqCKTTsUXSZzGYxMGpWpJBdYAYVH75x7yMnwJvra1BUJ"}
);

api::ExtendedKeyAccountCreationInfo P2WPKH_MEDIUM_XPUB_INFO(
        0, {"main"}, {"84'/0'/0'"}, {"xpub6CMeLkY9TzXyLYXPWMXB5LWtprVABb6HwPEPXnEgESMNrSUBsvhXNsA7zKS1ZRKhUyQG4HjZysEP8v7gDNU4J6PvN5yLx4meEm3mpEapLMN"}
);

api::AccountCreationInfo P2PKH_MEDIUM_KEYS_INFO(
        0, {"main", "main"}, {"44'/0'/0'", "44'/0'"},
        {hex::toByteArray("0437bc83a377ea025e53eafcd18f299268d1cecae89b4f15401926a0f8b006c0f7ee1b995047b3e15959c5d10dd1563e22a2e6e4be9572aa7078e32f317677a901"), hex::toByteArray("04fb60043afe80ee1aeb0160e2aafc94690fb4427343e8d4bf410105b1121f7a44a311668fa80a7a341554a4ef5262bc6ebd8cc981b8b600dafd40f7682edb5b3b")},
        {hex::toByteArray("d1bb833ecd3beed6ec5f6aa79d3a424d53f5b99147b21dbc00456b05bc978a71"), hex::toByteArray("88c2281acd51737c912af74cc1d1a8ba564eb7925e0d58a5500b004ba76099cb")}
);

api::ExtendedKeyAccountCreationInfo P2PKH_BIG_XPUB_INFO(
        0, {"main"}, {"44'/0'/0'"}, {"xpub6CThYZbX4PTeA7KRYZ8YXP3F6HwT2eVKPQap3Avieds3p1eos35UzSsJtTbJ3vQ8d3fjRwk4bCEz4m4H6mkFW49q29ZZ6gS8tvahs4WCZ9X"}
);

api::ExtendedKeyAccountCreationInfo P2SH_XPUB_INFO(
        0, {"main"}, {"49'/0'/0'"}, {"tpubDCcvqEHx7prGddpWTfEviiew5YLMrrKy4oJbt14teJZenSi6AYMAs2SNXwYXFzkrNYwECSmobwxESxMCrpfqw4gsUt88bcr8iMrJmbb8P2q"}
);

api::ExtendedKeyAccountCreationInfo ETH_XPUB_INFO(
        0, {"main"}, {"44'/60'/0'"}, {"xpub6E3C4qRJgptBznysvYD9JW7BNXQXvjhNBHYqASZKeF6konKuAH5bAm9Q9444SBasXSAB56BjjYnGCg9z1RYZzcEJeMxe6g4Ppgw8Q7Fo8fh"}
);

api::ExtendedKeyAccountCreationInfo ETH_MAIN_XPUB_INFO(
        0, {"main"}, {"44'/60'/0'"}, {"xpub6EswUg1PkDcQqx6xKrwxdVnnf5Wmi7nX6A9KjpufdCrzxMzwqNnnwmGFWnsWzsz3Es8nCqCjGJAGHdP3b8jMSfWmzPWskqrfzHuYbrSFYif"}
);

api::AccountCreationInfo ETH_KEYS_INFO(
        0, {"main"}, {"44'/1'/0'"},
        {hex::toByteArray("04d1dc4a3180fe2d56a1f02a68b053e59022ce5e107eae879ebef66a46d4ffe04dc3994facd376abcbab49c421599824a2600ee30e8520878e65581f598e2c497a")},
        {hex::toByteArray("2d560fcaaedb929eea27d316dec7961eee884259e6483fdf192704db7582ca14")}
);

api::AccountCreationInfo ETH_KEYS_INFO_VAULT(
        0, {"main"}, {"44'/1'/0'"},
        {hex::toByteArray("045650BE990F3CD39DF6CBAEBB8C06646727B1629509F993883681AE815EE1F3F76CC4628A600F15806D8A25AE164C061BF5EAB3A01BD8A7E8DB3BAAC07629DC67")},
        {hex::toByteArray("81F18B05DF5F54E5602A968D39AED1ED4EDC146F5971C4E84AA8273376B05D49")}
);

api::AccountCreationInfo ETH_KEYS_INFO_LIVE(
        0, {"main"}, {"44'/60'/0'/0/0"},
        {hex::toByteArray("046596fcbe77efedf05755dc7ee2f58748d57254defb098191e5867fcd1fb5e05624bbac4307fa0ee7f297fe0b1f27d46cd59242d12588101692767f9d2de3b6b7")},
        {hex::toByteArray("2a224ce46d853d381a68c6b819dabc7d00b14aaa538b6d472963820a48092cff")}
);

api::AccountCreationInfo ETC_KEYS_INFO_LIVE(
        0, {"main"}, {"44'/60'/0'/0/0"},
        {hex::toByteArray("0408b2ddef4cb4af62412ea70cce188ba1318651bb9c0ad599b2714d245109212fb2c79871ec5d35f479ee502c3d7a927908301823f6152823d37da9bd4cb31de7")},
        {hex::toByteArray("2a224ce46d853d381a68c6b819dabc7d00b14aaa538b6d472963820a48092cff")}
);

api::AccountCreationInfo XRP_KEYS_INFO(
        0, {"main"}, {"44'/144'/0'"},
        {hex::toByteArray("024819f9d4bd29318226e3c807cdd2da84161abaf5619c5d2bbfe5be63c74cc9ed")},
        {hex::toByteArray("b4f8427e7e19f284dfe7b99f107c55d00b3eae56df9569f0c4d56722742a5d71")}
);

api::AccountCreationInfo XTZ_KEYS_INFO(
        0, {"main"}, {"44'/1729'/0'/0'"},
        {hex::toByteArray("02af5696511e23b9e3dc5a527abc6929fae708defb5299f96cfa7dd9f936fe747d")},
        {hex::toByteArray("abcc4933bec06eeca6628b9e44f8e71d5e3cf510c0450dd1e29d9aa0f1717da9")}
);


const std::string TX_1 = "{\"hash\":\"666613fd82459f94c74211974e74ffcb4a4b96b62980a6ecaee16af7702bbbe5\",\"received_at\":\"2015-06-22T13:31:27Z\",\"lock_time\":0,\"block\":{\"hash\":\"00000000000000000b2df329293632a46c6b6a6c066fe5a617b264ff6edfa6a4\",\"height\":362035,\"time\":\"2015-06-22T13:31:27Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"e4486c80c17f41ff16b607044a1bf8a4d4aa3985156396b040290812575e71e1\",\"output_index\":0,\"value\":890000,\"address\":\"1KMbwcH1sGpHetLwwQVNMt4cEZB5u8Uk4b\",\"script_signature\":\"473044022009b9163abea9783d2ed1f9d4f9d0c16d624f5bba013728a1551877b9f934e5ad02201b5803ab33d7f19fa8d71106242467b21be592b24f1397569b970ceacf62e8870121020a90429f7e8964be1595789f7cedcb80b850405993e59c7ff244d83a7ec4ac4a\"},{\"input_index\":1,\"output_hash\":\"1565ad6bccb0c9d6c4771d8df49a2b6c5022b6f9dcd307582c7e2fe37e8b4f48\",\"output_index\":0,\"value\":13970000,\"address\":\"15zxN5EuryNJRpgo6rBRvgiYiDYix6EiHK\",\"script_signature\":\"483045022100ec78b471f6760c429044c76d34a084417967e090fada2cc797ef03bf0144a034022072b9e31f69b4a2a37e9d7c02a2e08d8a8195d6fd6da17ef8e2392c8c786015a30121021775300e7f3ba1c538349eb35f24a47314b53e6cc57e7ed154a073fabb00f3d7\"},{\"input_index\":2,\"output_hash\":\"e4486c80c17f41ff16b607044a1bf8a4d4aa3985156396b040290812575e71e1\",\"output_index\":1,\"value\":100000,\"address\":\"12BSjgZhjNi5JVh418h88nVkEBph9tn9JH\",\"script_signature\":\"483045022100ce0fe58a1297c6c96a3059ec11be82351046bbe977f4f5780a6b9710e871069f0220677bbd8364dbf8ef4b9817b404668c26fc6ae98141fbfa5b5c2e60b814e5e6da012102647367b0928f40d80ccec71104692e34f9e2abd4a0e6b7cad6faa1c9e51f3292\"},{\"input_index\":3,\"output_hash\":\"2f7e07679ea5babfe105c1a7c202efa8791819ef59e57ead25911f147dd90eaf\",\"output_index\":1,\"value\":23990000,\"address\":\"1PMiVc1UXestrbbaMqSwDuUrRRhTfy1UZc\",\"script_signature\":\"47304402201047d9e4eb4ce52377ec54a2e75fa0e6ecae6dc349f8a252f5eda4ea2bc8603c022002c016500d49a896aa79a8fb0235b0de27dd32a52feb7ab71c6a67bd44ddfa3501210249511220e84a3b3172895a000a012429c67a78fb33207a2650ec7e69fee21709\"},{\"input_index\":4,\"output_hash\":\"b9325a52d3373f725e8d39d04ff67531490af2221fd94bd3973ffc1b8208053b\",\"output_index\":1,\"value\":37019500,\"address\":\"13XU2CS1YgkDu7N1Xv7zoyypX1B7t4XHWS\",\"script_signature\":\"483045022100c37b6d5f22d6ba176ee0d06be46bcc86a7bd7ef6301bf7983e5f3fcac95ad414022029537ecc338f9a50316d287411bb69362539122b78ff121ecaa5c5ae79796dd70121025df76c1fc25d3d839a27a9b1f1b7051f612b929de2d19b48cbeade18d6439730\"},{\"input_index\":5,\"output_hash\":\"2f7e07679ea5babfe105c1a7c202efa8791819ef59e57ead25911f147dd90eaf\",\"output_index\":0,\"value\":1000000,\"address\":\"1PA2cZx9wZDo3xnd1HpRiLwTAtrpUFgPMw\",\"script_signature\":\"483045022100b257fb66f2ab29e80a4c6cfd4a07fcbb3aafedd02cb0ad76fdd455c34cf94c4502202dbf2392b466b6fe6e9bd4f1fc85077a639a07f3ec956fc45deefc6a5bdf5e26012103d766a986dbeab84e8827d6997e1e7630b909c8f6d16455485df2070ae21ebfe6\"},{\"input_index\":6,\"output_hash\":\"f5a1c29cbce3f0b99a38f46989bda4365a1f32941c8dcf415d76e35cbd0b1cbb\",\"output_index\":0,\"value\":10000,\"address\":\"14nMrPfMVLtXK3csgphyGZPsBPaPzNqtnQ\",\"script_signature\":\"483045022100ff1efd876eaf5813c9d8df381cc46f5b0d283e9b560461988a26465da1fb9fff0220567adc5ab53667e8864cba76a4540b2b5a6e8600495e7117d11e1747f458cdc801210319e6c9ac4a6f0d99d7dc2311dc177506ae3ad251b92a7345beb4add203cc342d\"},{\"input_index\":7,\"output_hash\":\"f871f48ddcf9ae17e5f0ff00121b36bfe54d329a9cbc38afa77db7c4540af7a4\",\"output_index\":1,\"value\":60000,\"address\":\"1EYLmfsvCVGsBmmiUmgJzoQG3MCou2Mh7J\",\"script_signature\":\"47304402202765acf7ea410e5712d5c5adacad0bcb0d71ef1b22b30860a42e851f745338e8022001de1738785ca43291b50e51281d20ecc4f451df9797e5c95641136f267cd4fe01210297af3c6d7e5f0c96a3ac24d059f838d20792362fbf945a10aa3cddeeacdb3cc4\"},{\"input_index\":8,\"output_hash\":\"2adc12ff62c58b56bb9fd5bf6cb702d2d07f39e7fd06b44ed69181c202f1ac2b\",\"output_index\":0,\"value\":9630000,\"address\":\"1Bmpme646SNGa1jjjYAfuijdyBNJXLGEh\",\"script_signature\":\"47304402201a44458da76e5359a904e40c6318e3930d5951a6649df9a846807c5107652d2b0220112e5d86a87e5f54d2d58b234392cf39cd7e9e3313a2f6850f89a9c60fc95d250121037b9922b3bea53333486d9b068ffafba088e4730e4b4081d6a71063b9aefda8ca\"},{\"input_index\":9,\"output_hash\":\"2217f57871f2a973e1544828b3fafe79d7bc8168e875ae50ed2304d7abd5be3a\",\"output_index\":1,\"value\":5000000,\"address\":\"1PhMRy1tEDiPYLxLGGXqVUSYr2qQ5NL8fm\",\"script_signature\":\"47304402200bf3579d28b96dd59a466ad5ae89ca4aef3ca5a473683f27a63d71cb7e90203702206900566e2f84abf4839e87b1bd52969e81c9629e728be9249f2be9e8c4e60bed01210325d655fc73c30551f6cf5a80bd224ccf676d51431d58e3e39fb0ca8ddbf4d891\"},{\"input_index\":10,\"output_hash\":\"7fa17586ffb7775bd19be72f568cba521c6e97eb166c9702096daf4856a2a4d3\",\"output_index\":0,\"value\":2000000,\"address\":\"1JpVd5LgkDw9o43bFsT9kMSMtqEKuYY69m\",\"script_signature\":\"473044022072655d94fef5fcecaa6153ef89a8ed93361665392d222b275072578af623f01a0220232f49fe3ad36eeca863884ac70a5800964239a6851e25f12cba1ebd97c4cdc301210357885d4578594861cdc10e1fbe9456a1df2461038936a8b913f7c727d1becb1b\"},{\"input_index\":11,\"output_hash\":\"8ff2a1c170a728fbd0ceb2947b120f8f1d0d08a52567b05882c02b41b866e357\",\"output_index\":1,\"value\":540000,\"address\":\"12YvvHBzc4r9bS5EaEs27AgqYTjX1guUer\",\"script_signature\":\"4730440220485e6884a4490a318bc8e8d04d93f283819ece82dfed6bf0434e3aadf8f8855d02202c46fa38e627beae7accd47adcb0ac3d75eaa05f9e962967ecedd1ca5870405c012102fc03f7162b34700e03a2da4bcbf9799eb6eec61ae01548e79db50765fb943932\"},{\"input_index\":12,\"output_hash\":\"7b7bb35e059cfabe79ea3b72caaa866fbf624d645768e198c5d021622262be80\",\"output_index\":0,\"value\":20000000,\"address\":\"12LVnPYsDzZKN3F4gdFaBJ1WSkkSxje6AS\",\"script_signature\":\"483045022100e7539137c60e00b5320ff1f7f4a0e1539a538c87fb133731dba7c8bc22fcfc7402200c4a2db4b203c6a84e7d41446d4ebc3c66ac203cf0ce93285887d8c7e23bb7680121030f25559e42584669dc19661b398a60660258e0bd3edfc16f3f38d1171c59c1fc\"},{\"input_index\":13,\"output_hash\":\"904fb0ba6c126a5fc50507c0f953e8e48363eb34798e82945971fc3634e2b305\",\"output_index\":0,\"value\":68384000,\"address\":\"1PKsemsjKHX8r2AWNHWTXhDq4waRMQpFEA\",\"script_signature\":\"47304402202013318f36716b9405627529a42d76901256798f5c8ac3a248b4b17f97e3640e02205cf81ccf9f0827679d462b6b20d954e0556f50cd52861eb50494841e9240aea40121034bd0aa6737a5d7b1571a3ac24afafd9225109feb0b79f57c1cf2271cfb6d2f59\"},{\"input_index\":14,\"output_hash\":\"a7e29eb70660fb23d0f6ecededf91519d9d17fa2ed2624a44f80f39e7f1cc634\",\"output_index\":1,\"value\":10000,\"address\":\"16VLgkWhKRSgLhW8sCra88qRuWXjpaZRtM\",\"script_signature\":\"47304402200b78257a162efa7410dc74cd6b29a8a80677a38f8ff2a3632fb472e59296d321022038de9ebe64e2a96454c4ddadb75f32d9b7f7026e2c3fefb66ebe60106569a5c30121025748f109aa23987b80a4ac8a681881e0b5532387ac0db5226ec1931bad68ebc0\"}],\"outputs\":[{\"output_index\":0,\"value\":182593500,\"address\":\"1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7\",\"script_hex\":\"76a91485efaded203a798edc46cd458f0714136f6d0acb88ac\"}],\"fees\":10000,\"amount\":182593500,\"confirmations\":110220}";
const std::string TX_2 = "{\"hash\":\"a5fb8b23c1131850569874b8d8592800211b3d0392753b84d2d5f9f53b7e09fc\",\"received_at\":\"2015-06-22T15:58:30Z\",\"lock_time\":0,\"block\":{\"hash\":\"00000000000000000198e024d936c87807b4198f9de7105015036ce785fa2bdc\",\"height\":362055,\"time\":\"2015-06-22T15:58:30Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"666613fd82459f94c74211974e74ffcb4a4b96b62980a6ecaee16af7702bbbe5\",\"output_index\":0,\"value\":182593500,\"address\":\"1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7\",\"script_signature\":\"473044022018ac8a44412d66f489138c3e8f9196b60dba1c24fb715dd8c3d66921bcc13f4702201f222fd3e25fe8f3807347650ae7b41451c078c9a8fc2e5b7d82d6a928a8c363012103da611b1fcacc0056ceb5489ee951b523e52de7ff1902fd1c6f9c212a542da297\"}],\"outputs\":[{\"output_index\":0,\"value\":182483500,\"address\":\"18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1\",\"script_hex\":\"76a9144ed14e321713e1c97056643a233b968d34b2231188ac\"},{\"output_index\":1,\"value\":100000,\"address\":\"1NMfmPC9yHBe5US2CUwWARPRM6cDP6N86m\",\"script_hex\":\"76a914ea4351fd2a0a2cd62ab264d9f0b1997696a632f488ac\"}],\"fees\":10000,\"amount\":182583500,\"confirmations\":110200}";
const std::string TX_3 = "{\"hash\":\"a5fb8b23c1131850569874b8d8592800211b3d0392753b84d2d5f9f53b7e09fc\",\"received_at\":\"2015-06-22T15:58:30Z\",\"lock_time\":0,\"block\":{\"hash\":\"00000000000000000198e024d936c87807b4198f9de7105015036ce785fa2bdc\",\"height\":362055,\"time\":\"2015-06-22T15:58:30Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"666613fd82459f94c74211974e74ffcb4a4b96b62980a6ecaee16af7702bbbe5\",\"output_index\":0,\"value\":182593500,\"address\":\"1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7\",\"script_signature\":\"473044022018ac8a44412d66f489138c3e8f9196b60dba1c24fb715dd8c3d66921bcc13f4702201f222fd3e25fe8f3807347650ae7b41451c078c9a8fc2e5b7d82d6a928a8c363012103da611b1fcacc0056ceb5489ee951b523e52de7ff1902fd1c6f9c212a542da297\"}],\"outputs\":[{\"output_index\":0,\"value\":182483500,\"address\":\"18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1\",\"script_hex\":\"76a9144ed14e321713e1c97056643a233b968d34b2231188ac\"},{\"output_index\":1,\"value\":100000,\"address\":\"1NMfmPC9yHBe5US2CUwWARPRM6cDP6N86m\",\"script_hex\":\"76a914ea4351fd2a0a2cd62ab264d9f0b1997696a632f488ac\"}],\"fees\":10000,\"amount\":182583500,\"confirmations\":110200}";
const std::string TX_4 = "{\"hash\":\"4450e70656888bd7f5240a9b532eac54db7d72f3b48bfef09fb45a185bb9c570\",\"received_at\":\"2015-06-22T16:19:26Z\",\"lock_time\":0,\"block\":{\"hash\":\"0000000000000000037e0f13d498d13a0b08e7ecc069ed497d665b6927b4724d\",\"height\":362058,\"time\":\"2015-06-22T16:19:26Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"a5fb8b23c1131850569874b8d8592800211b3d0392753b84d2d5f9f53b7e09fc\",\"output_index\":0,\"value\":182483500,\"address\":\"18BkSm7P2wQJfQhV7B5st14t13mzHRJ2o1\",\"script_signature\":\"483045022100bb268c100d815e49db33b14426b894639e62f6ac6cabbe1ded0af045ed7b5ca502200e9828576212c5003058ff9e2a6ac507d956fbe9c2ee24687f12ed43902a77410121029a8a38f29bfadfd6b9fe2a41f0f079b8c0a39f7e9c0479c3ac49a091d2e9d550\"}],\"outputs\":[{\"output_index\":0,\"value\":182373500,\"address\":\"139dJmHhFuuhrgbNAPehpokjYHNEtvkxot\",\"script_hex\":\"76a9141791e2de9302ecc21f60ac0cd417538c49cb5c6b88ac\"},{\"output_index\":1,\"value\":100000,\"address\":\"16cqCPDSoBYkaKUkYw94k3CeoNcMDcTLYN\",\"script_hex\":\"76a9143d9f6b778193e20365b2e8a493f33c35886d630688ac\"}],\"fees\":10000,\"amount\":182473500,\"confirmations\":110197}";


void BaseFixture::SetUp() {
    ::testing::Test::SetUp();
    ledger::qt::FilesystemUtils::clearFs(IntegrationEnvironment::getInstance()->getApplicationDirPath());
    dispatcher = std::make_shared<QtThreadDispatcher>();
    resolver = std::make_shared<NativePathResolver>(IntegrationEnvironment::getInstance()->getApplicationDirPath());
    backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    http = std::make_shared<QtHttpClient>(dispatcher->getMainExecutionContext());
    ws = std::make_shared<FakeWebSocketClient>();
    rng = std::make_shared<OpenSSLRandomNumberGenerator>();
}

void BaseFixture::TearDown() {
    ::testing::Test::TearDown();
    qDebug() << "TEAR DOWN";
    resolver->clean();
}

std::shared_ptr<WalletPool> BaseFixture::newDefaultPool(const std::string &poolName, const std::string &password) {
    return WalletPool::newInstance(
            poolName,
            password,
            http,
            ws,
            resolver,
            printer,
            dispatcher,
            rng,
            backend,
            api::DynamicObject::newInstance()
    );
}

BitcoinLikeWalletDatabase
BaseFixture::newBitcoinAccount(const std::shared_ptr<WalletPool> &pool,
                               const std::string &walletName,
                               const std::string &currencyName,
                               const std::shared_ptr<api::DynamicObject> &configuration,
                               int32_t index,
                               const std::string &xpub) {
    BitcoinLikeWalletDatabase db(pool, walletName, currencyName);
    if (!db.accountExists(index)) {
        createWallet(pool, walletName, currencyName, configuration);
        createAccount(pool, walletName, index);
        db.createAccount(index, xpub);
    }
    return db;
}

void BaseFixture::createWallet(const std::shared_ptr<WalletPool> &pool,
                               const std::string &walletName,
                               const std::string &currencyName,
                               const std::shared_ptr<api::DynamicObject> &configuration) {
    soci::session sql(pool->getDatabaseSessionPool()
                              ->getPool());
    WalletDatabaseEntry entry;
    entry.configuration = std::static_pointer_cast<DynamicObject>(configuration);
    entry.name = walletName;
    entry.poolName = pool->getName();
    entry.currencyName = currencyName;
    entry.updateUid();
    PoolDatabaseHelper::putWallet(sql, entry);
}

void BaseFixture::createAccount(const std::shared_ptr<WalletPool> &pool, const std::string &walletName, int32_t index) {
    soci::session sql(pool->getDatabaseSessionPool()
                              ->getPool());
    auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), walletName);
    if (!AccountDatabaseHelper::accountExists(sql, walletUid, index))
        AccountDatabaseHelper::createAccount(sql, walletUid, index);
}

std::shared_ptr<BitcoinLikeAccount>
BaseFixture::createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<BitcoinLikeAccount>(::wait(wallet->newAccountWithInfo(info)));
}

std::shared_ptr<BitcoinLikeAccount>
BaseFixture::createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::ExtendedKeyAccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<BitcoinLikeAccount>(::wait(wallet->newAccountWithExtendedKeyInfo(i)));
}

std::shared_ptr<EthereumLikeAccount>
BaseFixture::createEthereumLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<EthereumLikeAccount>(::wait(wallet->newAccountWithInfo(info)));
}

std::shared_ptr<EthereumLikeAccount>
BaseFixture::createEthereumLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::ExtendedKeyAccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<EthereumLikeAccount>(::wait(wallet->newAccountWithExtendedKeyInfo(i)));
}

std::shared_ptr<RippleLikeAccount>
BaseFixture::createRippleLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                       const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<RippleLikeAccount>(::wait(wallet->newAccountWithInfo(info)));
}

std::shared_ptr<RippleLikeAccount>
BaseFixture::createRippleLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                       const api::ExtendedKeyAccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<RippleLikeAccount>(::wait(wallet->newAccountWithExtendedKeyInfo(i)));
}

std::shared_ptr<TezosLikeAccount>
BaseFixture::createTezosLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                    int32_t index,
                                    const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<TezosLikeAccount>(::wait(wallet->newAccountWithInfo(i)));
}
