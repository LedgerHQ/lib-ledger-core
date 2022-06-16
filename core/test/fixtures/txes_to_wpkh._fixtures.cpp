#include "txes_to_wpkh_fixtures.h"

namespace ledger {
    namespace testing {
        namespace txes_to_wpkh {
            core::api::ExtendedKeyAccountCreationInfo XPUB_INFO(
                0,
                {"main"},
                {"84'/0'/0'"},
                {"xpub6C2qnauSiRRR8UG9PXraYrfBtXZHzFRNgwJgwG7ZDp7SbWAdMSiMFoYDukTw4ExE95H4VkgECnhDe2uyLawoqUkoSHEDnoDafC2R44dXs7p"});
            // xpub from https://github.com/LedgerHQ/xpub-scan/blob/master/.github/workflows/regression_tests/datasets.json
            const std::vector<std::string> TXes =
                {
                    "{\"hash\":\"4b3a5535d24d882233bb991a2e6fa111e64294aadcd140960cdceada2fae1bb8\",\"received_at\":\"2020-06-26T13:58:11Z\",\"lock_time\":0,\"block\":{\"hash\":\"00000000000000000003f12608c713d887fc216753ba70b264cc54379415928b\",\"height\":636436,\"time\":\"2020-06-26T14:29:17Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"70dcda8357d590f0cfb6cd504731d427db76fe03616f57c7af8366106af67092\",\"output_index\":1,\"value\":37471,\"address\":\"3MVXMz1zCEBScDM1pMsifuYavjzEjxCfM6\",\"script_signature\":\"160014682a7e09d3f88f56f6e9ed0a9511f064b79ab722\",\"sequence\":0,\"txinwitness\":[\"30450221009575c1fc1890db00b7cc33426954b19e516d19b03fd9fc924579e718f8437e4902203f0f5b8763c389548c910bbd7cd72a8d0bb1f35ba67f5b53f9ca1620bc86139801\",\"0378b2de4c77eaa8c1285ff73837a4b7abe9babf30e1cbc522329e0e15db2c03bc\"]}],\"outputs\":[{\"output_index\":0,\"value\":27278,\"address\":\"bc1qr500ysrg653aaplftaac753srtt2jwtfvcr5vt\",\"script_hex\":\"00141d1ef24068d523de87e95f7b8f52301ad6a93969\"},{\"output_index\":1,\"value\":9017,\"address\":\"3Nq7i7JRT7rzoKhhnxE3F8hbp1FpjN9jmV\",\"script_hex\":\"a914e7e2cc7a5d070144867183cd27b13db3b074883b87\"}],\"fees\":1176,\"amount\":36295,\"confirmations\":86334}",
                    "{\"hash\":\"673f7e1155dd2cf61c961cedd24608274c0f20cfaeaa1154c2b5ef94ec7b81d1\",\"received_at\":\"2021-09-08T12:45:21Z\",\"lock_time\":0,\"block\":{\"hash\":\"00000000000000000003fa8ebda5058dca475e65a8aaf692dc5a7449cf6e5a89\",\"height\":699622,\"time\":\"2021-09-08T12:58:43Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"4b3a5535d24d882233bb991a2e6fa111e64294aadcd140960cdceada2fae1bb8\",\"output_index\":0,\"value\":27278,\"address\":\"bc1qr500ysrg653aaplftaac753srtt2jwtfvcr5vt\",\"script_signature\":\"\",\"sequence\":0,\"txinwitness\":[\"304402205e18af8a590bdfd5c9e4f2e349b105ec5efb56b5fd3cf07e69786ef979d53ff4022069f8fd41fb6e9edee10d80f5bd8144a72ee1c609d93512487e362733103f78d201\",\"026f85f41f6a4ba5a1fc6819ffcfd9013e2ccd372b94e31fe867362035b38f1652\"]}],\"outputs\":[{\"output_index\":0,\"value\":1000,\"address\":\"bc1qrewjj96rjfzc9z2al0hvs2jtdc58nkgvrr6fgv\",\"script_hex\":\"00141e5d291743924582895dfbeec82a4b6e2879d90c\"},{\"output_index\":1,\"value\":25402,\"address\":\"bc1qz2z9dnhzwhveqemt9utryeucqnjuupenmfzsxv\",\"script_hex\":\"0014128456cee275d990676b2f1632679804e5ce0733\"}],\"fees\":876,\"amount\":26402,\"confirmations\":23148}",
                    "{\"hash\":\"5464631456754e6b410d5d9eb7cff4f82d1dc9aec0e2ec8fe759df6118e0112f\",\"received_at\":\"2021-11-20T12:48:08Z\",\"lock_time\":0,\"block\":{\"hash\":\"0000000000000000000b96dc8d6e75dc3eb4f995a10545f84ad1b8f1de4b9893\",\"height\":710566,\"time\":\"2021-11-20T12:52:04Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"4fadad2c103014ebaa3073ea63ff060b27d2d30b4e7f956727b97a44d1cf8ad7\",\"output_index\":0,\"value\":67218,\"address\":\"bc1qr500ysrg653aaplftaac753srtt2jwtfvcr5vt\",\"script_signature\":\"\",\"sequence\":4294967295,\"txinwitness\":[\"304402201668900493e07d2a4c2cf2b9c98a2b3ad1afc83da34cf727098077f34111209202202da599d4368331def39c95eab86480a9e9772fe4a393ad97d7174358becf422a01\",\"026f85f41f6a4ba5a1fc6819ffcfd9013e2ccd372b94e31fe867362035b38f1652\"]}],\"outputs\":[{\"output_index\":0,\"value\":60000,\"address\":\"1MD43R5k9qoAch5nUxk3BxNkVxpGbDS8iw\",\"script_hex\":\"76a914ddaa0bd7223bd2b5e61e8e7541fc22b9197440d288ac\"},{\"output_index\":1,\"value\":6780,\"address\":\"bc1q98csrhfkzrvkeee0k6jn70xdl2ghtewlkxva87\",\"script_hex\":\"001429f101dd3610d96ce72fb6a53f3ccdfa9175e5df\"}],\"fees\":438,\"amount\":66780,\"confirmations\":12204}",
                    "{\"hash\":\"4fadad2c103014ebaa3073ea63ff060b27d2d30b4e7f956727b97a44d1cf8ad7\",\"received_at\":\"2021-11-20T12:43:20Z\",\"lock_time\":0,\"block\":{\"hash\":\"0000000000000000000b96dc8d6e75dc3eb4f995a10545f84ad1b8f1de4b9893\",\"height\":710566,\"time\":\"2021-11-20T12:52:04Z\"},\"inputs\":[{\"input_index\":0,\"output_hash\":\"6b1c1bab2ce9a3429c4ddbbecf70d5ca2ac9a27634ebc8dd2dff347073eb1e39\",\"output_index\":0,\"value\":67554,\"address\":\"bc1qrd58306xwnpxegh76tp8gns4jxvjm7zxv3zzlc\",\"script_signature\":\"\",\"sequence\":4294967295,\"txinwitness\":[\"3045022100c27cd772986b8b82af189be7260a1e436e1a0573f8e908570b0fc55cd33541f4022059c575c3738acc502651ac0806816e91c9775cc253ee1f6798c2cf87901bb6c501\",\"0242dedafae7bdaf0c2b0ec74d2e1d83f874c11ad71e4a00031301e3aeb3322a6f\"]}],\"outputs\":[{\"output_index\":0,\"value\":67218,\"address\":\"bc1qr500ysrg653aaplftaac753srtt2jwtfvcr5vt\",\"script_hex\":\"00141d1ef24068d523de87e95f7b8f52301ad6a93969\"}],\"fees\":336,\"amount\":67218,\"confirmations\":12204}"};
            std::shared_ptr<core::BitcoinLikeAccount> inflate(const std::shared_ptr<core::WalletPool> &pool, const std::shared_ptr<core::AbstractWallet> &wallet) {
                auto account = std::dynamic_pointer_cast<ledger::core::BitcoinLikeAccount>(
                    uv::wait(wallet->newAccountWithExtendedKeyInfo(XPUB_INFO)));
                for (const std::string &tx : TXes) {
                    std::vector<ledger::core::Operation> operations;
                    const auto parsedTx = ledger::core::JSONUtils::parse<ledger::core::TransactionParser>(tx);
                    account->interpretTransaction(*parsedTx, operations, true);
                    account->bulkInsert(operations);
                }
                return account;
            }
        } // namespace txes_to_wpkh
    }     // namespace testing
} // namespace ledger
