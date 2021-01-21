// This file was GENERATED by command:
//     generate_fixtures.py
// DO NOT EDIT BY HAND!!!
#include "zec_xpub_fixtures.h"

namespace ledger {
	namespace testing {
		namespace zec_xpub {
			core::api::ExtendedKeyAccountCreationInfo XPUB_INFO(
			        0, {"zec"}, {"49'/1'/0'"}, {"xpub6DWu8baXZKRb3FbLebkpXq2qm1hH4N9F8hzTBoZAWrPNBAXgCSK8qqfsc38gaCEFZWUS9rJHMgE3DS4rh7Qqn47PHKHYkMzWXfo39cYdwVJ"}
			);
			std::shared_ptr<core::BitcoinLikeAccount> inflate(const std::shared_ptr<core::WalletPool>& pool, const std::shared_ptr<core::AbstractWallet>& wallet) {
				auto account = std::dynamic_pointer_cast<core::BitcoinLikeAccount>(uv::wait(wallet->newAccountWithExtendedKeyInfo(XPUB_INFO)));
                std::vector<core::Operation> operations;
                account->interpretTransaction(*core::JSONUtils::parse<core::TransactionParser>(TX_1), operations, true);
                account->interpretTransaction(*core::JSONUtils::parse<core::TransactionParser>(TX_2), operations, true);
                account->bulkInsert(operations);
				return account;
			}
			const std::string TX_1 = "{\"inputs\": [{\"value\": 2562258, \"output_index\": 1, \"output_hash\": \"dc685ce69018f4abedfd8d4937001258658cfe3398d39be2286d2961075b35f8\", \"input_index\": 0, \"address\": \"t1MepQJABxoWarqMvgBHGiFprtuvA47Hiv8\", \"script_signature\": \"483045022100a13ae06b36e3d4e90c7b9265bfff296b98d79c9970d7ef4964eb26d23ab44a5f022024155e86bde7a2322b1395d904c5fa007a925bc02f7d60623bde56a8b09bbb680121032d1d22333719a013313e538557971639f8c167fa5be8089dd2e996d704fb580c\"}], \"lock_time\": 0, \"hash\": \"4858a0a3d5f1de0c0f5729f25c3501bda946093aed07f842e53a90ac65d66f70\", \"outputs\": [{\"script_hex\": \"76a91407c4358a95e07e570d67857e12086fd6b1ee873688ac\", \"output_index\": 0, \"value\": 100000, \"address\": \"t1JafnXdJDUUjLnTfbvuEBdCARKLvqmj5jb\"}, {\"script_hex\": \"76a9143c1a6afff1941911e0b524ffcd2a15de6e68b6d188ac\", \"output_index\": 1, \"value\": 1462258, \"address\": \"t1PMQCQ36ccBJeEdFXJh4nnU34rPyDimANE\"}], \"amount\": 1562258, \"confirmations\": 345617, \"fees\": 1000000, \"received_at\": \"2016-11-18T13:01:28Z\", \"block\": {\"time\": \"2016-11-18T13:01:28Z\", \"hash\": \"00000000de85d65e6f33c5c0f7e4f1e4aa648c14063d6a6f3002b5f566df3ebd\", \"height\": 12508}}";
			const std::string TX_2 = "{\"inputs\": [{\"value\": 1473822, \"output_index\": 0, \"output_hash\": \"1ba8fe99e65197a4f9504e91a14d6cad1619ea2090ed760f00db0fbdcce2d175\", \"input_index\": 0, \"address\": \"t1cAassY4ucBYLkNmn3zAPKWsHFPtVdrqaw\", \"script_signature\": \"4730440220185809b62484070cfa2eb15964abbd5705ea2bcaac4b30a36b85144cc31f671e022072b6efa26aa9cedc0d747d04cceef7bcd0b7fc18ff2933682bc7260e98026b550121033893693821812d70c09ba0145a5bd0ed0f0b38e544964f03e7fe0ac9963a1e67\"}, {\"value\": 2094552, \"output_index\": 0, \"output_hash\": \"1dc7e0e41cbffb09665f9d998b1c1e12615f3ffa1ceac2cd6d87261b0bb17c17\", \"input_index\": 1, \"address\": \"t1fgtey9STrFTtVW68yePEeLFU5UBBDMX2k\", \"script_signature\": \"47304402204caa84f0c643f8c9916e3279bf700d0030b96ed338e1fca0580be3b7b864d4950220350d4faf09f35af9af305560955d1f4f65e6d6c705255a08c22ae27cfa87a6250121022ff18d74d453bd1873b2f90f188eb7202fe59ebb0c1e3ccc2941b9c6fd49788a\"}], \"lock_time\": 10297, \"hash\": \"dc685ce69018f4abedfd8d4937001258658cfe3398d39be2286d2961075b35f8\", \"outputs\": [{\"script_hex\": \"76a9142047741595e17c2df246adaa644bedfde1bbc25c88ac\", \"output_index\": 0, \"value\": 1004196, \"address\": \"t1LpHE9VRg5Gz2CdkKGNRmZPrdLDEysGyZY\"}, {\"script_hex\": \"76a9142975717505881264910b9c42c4ac2d7035c00a1f88ac\", \"output_index\": 1, \"value\": 2562258, \"address\": \"t1MepQJABxoWarqMvgBHGiFprtuvA47Hiv8\"}], \"amount\": 3566454, \"confirmations\": 347816, \"fees\": 1920, \"received_at\": \"2016-11-14T17:19:20Z\", \"block\": {\"time\": \"2016-11-14T17:19:20Z\", \"hash\": \"000000015da4eb1d29b9b704346561ea931649af2c38c9bc039b748b236d02fc\", \"height\": 10309}}";
		}
	}
}
