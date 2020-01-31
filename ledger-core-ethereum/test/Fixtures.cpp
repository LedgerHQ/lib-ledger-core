#include <core/utils/Hex.hpp>

#include "Fixtures.hpp"

namespace ledger {
	namespace testing {
		namespace eth_xpub {
			const std::string TX_1 = "{\"nonce\": \"0x0\", \"index\": 0, \"gas_price\": 10000000, \"from\": \"0xfed6476b45bf49ec711c0366f647a003ed6eee56\", \"gas\": 21000, \"gas_used\": 21000, \"value\": 2464550012766714, \"to\": \"0x8f7a0afaaee372eefd020056fc552bd87dd75d73\", \"confirmations\": 698876, \"input\": \"0x\", \"received_at\": \"2018-08-08T04:50:09Z\", \"hash\": \"0xdca99dccc9e25b93249cbe2b793d93f2c9457020238bab27996e7a314c60b65a\", \"cumulative_gas_used\": 7885982, \"block\": {\"time\": \"2018-08-08T04:50:09Z\", \"hash\": \"0xe17428e5182575ad27b55556e33c121cebc42477e362d2bb9c619a257f2b2396\", \"height\": 6108540}}";
			const std::string TX_2 = "{\"nonce\": \"0x8\", \"index\": 0, \"gas_price\": 4600000000, \"from\": \"0x59f436d401ded6c1930375d4b27e7adfb490ff55\", \"gas\": 21000, \"gas_used\": 21000, \"value\": 3000000000000000, \"to\": \"0x8f7a0afaaee372eefd020056fc552bd87dd75d73\", \"confirmations\": 775082, \"input\": \"0x\", \"received_at\": \"2018-07-26T08:07:28Z\", \"hash\": \"0xf8faeafb1789a185aff3bd19fb8814b3604f36da607d184813db9a070ae714d0\", \"cumulative_gas_used\": 4526655, \"block\": {\"time\": \"2018-07-26T08:07:14Z\", \"hash\": \"0x9f4718e7ef8d22f6600f32dc825cf139eff54103d4ae3a71af64b4923d65e9a2\", \"height\": 6032334}}";
			const std::string TX_3 = "{\"nonce\": \"0x0\", \"index\": 184, \"gas_price\": 7500000000, \"from\": \"0x524abff9d799c5f5219e9354b54551fb1ed41ceb\", \"gas\": 21000, \"gas_used\": 21000, \"value\": 2320000000000000, \"to\": \"0x8f7a0afaaee372eefd020056fc552bd87dd75d73\", \"confirmations\": 933301, \"input\": \"0x\", \"received_at\": \"2018-06-29T09:27:39Z\", \"hash\": \"0x6bcc9aaa5249201f742fcdc99671a2b2e1f6f90fffebc5decbf4106b7a5c1d1d\", \"cumulative_gas_used\": 6124085, \"block\": {\"time\": \"2018-06-29T09:27:39Z\", \"hash\": \"0x39fdd85ecdbbcb8464eaa1ba272d9288d4edc087ff477a08adb4c61347af72f8\", \"height\": 5874115}}";
			const std::string TX_4 = "{\"nonce\": \"0x1\", \"index\": 103, \"gas_price\": 3500000000, \"from\": \"0x8f7a0afaaee372eefd020056fc552bd87dd75d73\", \"gas\": 21000, \"gas_used\": 21000, \"value\": 2479858101795297, \"to\": \"0x524abff9d799c5f5219e9354b54551fb1ed41ceb\", \"confirmations\": 943264, \"input\": \"0x\", \"received_at\": \"2018-06-27T16:12:58Z\", \"hash\": \"0x9f8c005aac3f8da70beb1fd3c1c703fa9846e5fe1f543fd13eb9b32e5b6798df\", \"cumulative_gas_used\": 6246931, \"block\": {\"time\": \"2018-06-27T16:12:49Z\", \"hash\": \"0x3fbc90567325b9d277243c593840876b1553f32840b2c2c2f080ba6fd58ab55f\", \"height\": 5864152}}";
			const std::string TX_5 = "{\"nonce\": \"0x0\", \"index\": 113, \"gas_price\": 10000000000, \"from\": \"0x8f7a0afaaee372eefd020056fc552bd87dd75d73\", \"gas\": 21000, \"gas_used\": 21000, \"value\": 2479858101795297, \"to\": \"0xfed6476b45bf49ec711c0366f647a003ed6eee56\", \"confirmations\": 944841, \"input\": \"0x\", \"received_at\": \"2018-06-27T09:45:51Z\", \"hash\": \"0x6323309e38dd099bf342c2b94d00297fa6a94acf3ba0e55b105b52edab2199a5\", \"cumulative_gas_used\": 3195121, \"block\": {\"time\": \"2018-06-27T09:45:41Z\", \"hash\": \"0x365a9912d81097a6e8c7712572599ca9751c28b4c1325c69db27ff26db41428b\", \"height\": 5862575}}";
			const std::string TX_6 = "{\"nonce\": \"0xb\", \"index\": 57, \"gas_price\": 20000000000, \"from\": \"0x8a98fc3dcef83289593c466bab24912ec55654df\", \"gas\": 21000, \"gas_used\": 21000, \"value\": 12409354304799612, \"to\": \"0x8f7a0afaaee372eefd020056fc552bd87dd75d73\", \"confirmations\": 956001, \"input\": \"0x\", \"received_at\": \"2018-06-25T11:00:26Z\", \"hash\": \"0xb4cb67478d1fbb90b4658157dcc6a3666e519a7c09d2d5381be87f319b309643\", \"cumulative_gas_used\": 2360198, \"block\": {\"time\": \"2018-06-25T11:00:26Z\", \"hash\": \"0x53031f3edbd838b1f4c23550b251de01ec5ef819e97b6e2729e02cbb0e0ce386\", \"height\": 5851415}}";
			
			const core::api::ExtendedKeyAccountCreationInfo ETH_XPUB_INFO(
					0, {"main"}, {"44'/60'/0'"}, {"xpub6E3C4qRJgptBznysvYD9JW7BNXQXvjhNBHYqASZKeF6konKuAH5bAm9Q9444SBasXSAB56BjjYnGCg9z1RYZzcEJeMxe6g4Ppgw8Q7Fo8fh"}
			);

			const core::api::ExtendedKeyAccountCreationInfo ETH_MAIN_XPUB_INFO(
					0, {"main"}, {"44'/60'/0'"}, {"xpub6EswUg1PkDcQqx6xKrwxdVnnf5Wmi7nX6A9KjpufdCrzxMzwqNnnwmGFWnsWzsz3Es8nCqCjGJAGHdP3b8jMSfWmzPWskqrfzHuYbrSFYif"}
			);

			const core::api::AccountCreationInfo ETH_KEYS_INFO(
					0, {"main"}, {"44'/1'/0'"},
					{core::hex::toByteArray("04d1dc4a3180fe2d56a1f02a68b053e59022ce5e107eae879ebef66a46d4ffe04dc3994facd376abcbab49c421599824a2600ee30e8520878e65581f598e2c497a")},
					{core::hex::toByteArray("2d560fcaaedb929eea27d316dec7961eee884259e6483fdf192704db7582ca14")}
			);

			const core::api::AccountCreationInfo ETH_KEYS_INFO_VAULT(
					0, {"main"}, {"44'/1'/0'"},
					{core::hex::toByteArray("045650BE990F3CD39DF6CBAEBB8C06646727B1629509F993883681AE815EE1F3F76CC4628A600F15806D8A25AE164C061BF5EAB3A01BD8A7E8DB3BAAC07629DC67")},
					{core::hex::toByteArray("81F18B05DF5F54E5602A968D39AED1ED4EDC146F5971C4E84AA8273376B05D49")}
			);

			const core::api::AccountCreationInfo ETH_KEYS_INFO_LIVE(
					0, {"main"}, {"44'/60'/0'/0/0"},
					{core::hex::toByteArray("046596fcbe77efedf05755dc7ee2f58748d57254defb098191e5867fcd1fb5e05624bbac4307fa0ee7f297fe0b1f27d46cd59242d12588101692767f9d2de3b6b7")},
					{core::hex::toByteArray("2a224ce46d853d381a68c6b819dabc7d00b14aaa538b6d472963820a48092cff")}
			);

			const core::api::AccountCreationInfo ETC_KEYS_INFO_LIVE(
					0, {"main"}, {"44'/60'/0'/0/0"},
					{core::hex::toByteArray("0408b2ddef4cb4af62412ea70cce188ba1318651bb9c0ad599b2714d245109212fb2c79871ec5d35f479ee502c3d7a927908301823f6152823d37da9bd4cb31de7")},
					{core::hex::toByteArray("2a224ce46d853d381a68c6b819dabc7d00b14aaa538b6d472963820a48092cff")}
			);	

			std::shared_ptr<core::EthereumLikeAccount> inflate(const std::shared_ptr<core::Services>& services, const std::shared_ptr<core::AbstractWallet>& wallet) {
				auto account = std::dynamic_pointer_cast<core::EthereumLikeAccount>(wait(wallet->newAccountWithExtendedKeyInfo(ETH_XPUB_INFO)));
				soci::session sql(services->getDatabaseSessionPool()->getPool());
				sql.begin();
                account->putTransaction(sql, *core::JSONUtils::parse<core::EthereumLikeTransactionParser>(TX_1));
				account->putTransaction(sql, *core::JSONUtils::parse<core::EthereumLikeTransactionParser>(TX_2));
				account->putTransaction(sql, *core::JSONUtils::parse<core::EthereumLikeTransactionParser>(TX_3));
				account->putTransaction(sql, *core::JSONUtils::parse<core::EthereumLikeTransactionParser>(TX_4));
				account->putTransaction(sql, *core::JSONUtils::parse<core::EthereumLikeTransactionParser>(TX_5));
				account->putTransaction(sql, *core::JSONUtils::parse<core::EthereumLikeTransactionParser>(TX_6));
				sql.commit();
				return account;
			}
		}
	}
}
