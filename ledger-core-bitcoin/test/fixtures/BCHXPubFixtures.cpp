#include "BCHXPubFixtures.hpp"

namespace ledger {
	namespace testing {
		namespace bch_xpub {
			core::api::ExtendedKeyAccountCreationInfo XPUB_INFO(
			        0, {"abc"}, {"44'/145'/0'"}, {"xpub6BvNdfGcyMB9Usq88ibXUt3KhbaEJVLFMbhTSNNfTm8Qf1sX9inTv3xL6pA6KofW4WF9GpdxwGDoYRwRDjHEir3Av23m2wHb7AqhxJ9ohE8"}
			);

			std::shared_ptr<core::BitcoinLikeAccount> inflate(const std::shared_ptr<core::Services>& services, const std::shared_ptr<core::AbstractWallet>& wallet) {
				auto account = std::dynamic_pointer_cast<core::BitcoinLikeAccount>(wait(wallet->newAccountWithExtendedKeyInfo(XPUB_INFO)));
				soci::session sql(services->getDatabaseSessionPool()->getPool());
				sql.begin();				
				account->putTransaction(sql, *core::JSONUtils::parse<core::BitcoinLikeTransactionParser>(TX_1));
				sql.commit();
				
				return account;
			}

			const std::string TX_1 = "{\"inputs\": [{\"value\": 5000000, \"output_index\": 1, \"output_hash\": \"954aa2ad3a5a825c1de9266bd3e92ff337bf1a65b01f6a29b02d4412a084bf4a\", \"input_index\": 0, \"address\": \"14RYdhaFU9fMH25e6CgkRrBRjZBvEvKxne\", \"script_signature\": \"4730440220074dff2693aa6e2569c4aa607caac3a60b232df2d6e693c98efb71f942167583022079711633ed6462f81120d5942ecd5a22b5b6000961f2a3fbc121f1159e582a004121025cc0670063855536ef72e06a02ec6e0aecb4f2000fa23ddedf334b14c41410e5\"}], \"lock_time\": 0, \"hash\": \"d5e76be45d1abb2ff6292b93f3ebb35245a3d89291e1e5cd5b7af30cd241534e\", \"outputs\": [{\"script_hex\": \"76a914389d8c1c9d871eb2d075e71f1d32bbbf1e67aeea88ac\", \"output_index\": 0, \"value\": 1000000, \"address\": \"16AMaKewP778obhBUAWWV5sVU6Qg6rvuBt\"}, {\"script_hex\": \"76a91411b4072c0a7d5efb9d158fecea028733e729df2288ac\", \"output_index\": 1, \"value\": 3997740, \"address\": \"12cc9pxqLh3DudnxdpzBzv4NrBoX8EpdVn\"}], \"amount\": 4997740, \"confirmations\": 16, \"fees\": 2260, \"received_at\": \"2018-05-31T15:24:41Z\", \"block\": {\"time\": \"2018-05-31T15:38:32Z\", \"hash\": \"000000000000000000e306db78ec67239c84c3ddd74514e32d0a63dfac33b52d\", \"height\": 532629}}";
		}
	}
}
