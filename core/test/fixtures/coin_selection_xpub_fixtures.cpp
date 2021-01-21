// This file was GENERATED by command:
//     generate_bitcoin_fixtures.py
// DO NOT EDIT BY HAND!!!
#include "coin_selection_xpub_fixtures.h"

namespace ledger {
	namespace testing {
		namespace coin_selection_xpub {
			core::api::ExtendedKeyAccountCreationInfo XPUB_INFO(
			        0, {"btc_testnet"}, {"49'/1'/0'"}, {"tpubDCgRuwHE2JfZH1b6mCKwzFSDWc9JoMHQsxSbR3wDeD3Gi47yvkC5qv1tSXwUaHi4VThmemR5QUvBT9oP89Z9X8bB38fQNe4dJQAwAhuG4cM"}
			);
			std::shared_ptr<core::BitcoinLikeAccount> inflate(const std::shared_ptr<core::WalletPool>& pool, const std::shared_ptr<core::AbstractWallet>& wallet) {
				auto account = std::dynamic_pointer_cast<core::BitcoinLikeAccount>(uv::wait(wallet->newAccountWithExtendedKeyInfo(XPUB_INFO)));
                std::vector<core::Operation> operations;
                account->interpretTransaction(*core::JSONUtils::parse<core::TransactionParser>(TX_1), operations, true);
                account->interpretTransaction(*core::JSONUtils::parse<core::TransactionParser>(TX_2), operations, true);
                account->interpretTransaction(*core::JSONUtils::parse<core::TransactionParser>(TX_3), operations, true);
                account->interpretTransaction(*core::JSONUtils::parse<core::TransactionParser>(TX_4), operations, true);
                account->interpretTransaction(*core::JSONUtils::parse<core::TransactionParser>(TX_5), operations, true);
                account->bulkInsert(operations);
				return account;
			}
			const std::string TX_1 = "{\"inputs\": [{\"value\": 19990100, \"output_index\": 1, \"output_hash\": \"c154162c627bfe417ee61ff0e4d27c46fdff14454ff44b4c3c4f5f5932dd8520\", \"input_index\": 0, \"address\": \"2NE7JzQphDhWXNLxS5DtvZiQahFRBWtPzuN\", \"txinwitness\": [\"3044022052a52b5c3a47d5521c85ad6421244a2b3fd3a85b61f8144a14bd9180da0a0831022075affac9f31f36d1104db15edfc5aa57d53af588be3d252f4acb96b241f21a2201\", \"0276b2430c1ecef6eadc7b61d0958568aff8a62b65974307c5cd980320f8d6a9b6\"], \"script_signature\": \"1600143a2ff3bb2732adbc5b84affe1f796f2667c34839\"}, {\"value\": 30000000, \"output_index\": 0, \"output_hash\": \"62cf92d94342d6ee2606e60039e47aa16b77ee53f60aeaa526c0cfba8f16a045\", \"input_index\": 1, \"address\": \"2N5sFyyzrfZ57QKjPi4mLnpNSDBtsvv45Sa\", \"txinwitness\": [\"3045022100da0683ca8b8a82434def03aae450dcc161e8d97c101dfdec3bad68d6e55c54dc0220234158a34f4e63f64428a8281a6ab5d7d0483b6f397f0d46eb1cb251d2a4036b01\", \"02e34ef26b9a6109c2ae4844de8fb6ac0e2c3524e313c0a07ea9cb0cccdc2ad861\"], \"script_signature\": \"1600146ad92475080928622779a58e101d4906ee66cc4a\"}], \"lock_time\": 1348882, \"hash\": \"feb3e8d04d1a26a552a77b2a22cae6cc4e5984cde8d12925d39287d2d8aa87e1\", \"outputs\": [{\"script_hex\": \"a9143c8eb6688c3fae732f83639fab5f96e2e977e91e87\", \"output_index\": 0, \"value\": 40000000, \"address\": \"2MxmRVdkLwuUyzUAaApzTZuFvGR7JdPaqab\"}, {\"script_hex\": \"a9143a5fc8c0e01ef294e73b5c94217be6452ad52df887\", \"output_index\": 1, \"value\": 9986800, \"address\": \"2MxZsvcUnFc7ZnYiG96aiRqeJuNsD8PxXhA\"}], \"amount\": 49986800, \"confirmations\": 0, \"fees\": 3300, \"received_at\": \"2018-07-02T10:13:30Z\", \"block\": null}";
			const std::string TX_2 = "{\"inputs\": [{\"value\": 76932, \"output_index\": 1, \"output_hash\": \"fc11c3c83eb23ec5dca8128a7bd6541a3e8a99f9d730c6cabecd507b312606f0\", \"input_index\": 0, \"address\": \"2NCxPhiBGpvug2nLCX2pdjf8LXYFu2HPEq2\", \"txinwitness\": [\"30450221009547423e7f91d5bb197250cc2674bf5c8b8f1f830dcc4ee8079d632111c280e9022056795031a30639a8ea0e22b667d6aa36e293724601254f4604efe02746d5b29d01\", \"02052d00872391756daccbd497d855e719d760b3e9b34bf669928e7d455a921db9\"], \"script_signature\": \"16001429b59e03ad8ced4206a209184a40bf3e9f5b5b0b\"}, {\"value\": 12127507, \"output_index\": 1, \"output_hash\": \"239e9dd322ebc098c6bc272910024a6a3dedefff6581a3e687d0752eac6963f4\", \"input_index\": 1, \"address\": \"2MtsJMoGawkTJzWuS4sbDMP2jPiyBncHwmM\", \"txinwitness\": [\"304402200a49b04fafe81c5ca8ef5b3af5550fff66da970d79fb1d157e038e3cceefc6ec02200a9a32e32fa1af7a301b6a01799052f0bbfbb14dd70d013a80514bbfb023e09001\", \"03e155b6c04b8fce46fe7f89eb0a2da1f20bf43626b6fdd3a3ff14fd70454004f5\"], \"script_signature\": \"160014f34b8f7dba5d5d76f4972d66921c8045c9bef565\"}, {\"value\": 19997560, \"output_index\": 1, \"output_hash\": \"62cf92d94342d6ee2606e60039e47aa16b77ee53f60aeaa526c0cfba8f16a045\", \"input_index\": 2, \"address\": \"2N4Gz66CudsjG81UHHVKDzMCucem3yCLN5Z\", \"txinwitness\": [\"304402201fe4155f95a3840caec292915457d0d813ed6fdd3000ab19c465e62bc895343c02200974bc81362961181501963415e30a7487ec6b6c3dedc477d75b4195c13d1ace01\", \"039e919188890646fc0e0c44308f8264812d083a547cb9305bd053aa5b01ddb292\"], \"script_signature\": \"160014ba363d16c999ea5458a9bc11bd1a3664b51fb040\"}, {\"value\": 32500000, \"output_index\": 1, \"output_hash\": \"9c636e0f3a6cfaa70989ae671c7f1bdb609c6acd2102f81479273eb5740717c8\", \"input_index\": 3, \"address\": \"2NEwNeSp45EqbqWtmKU5VfYKRy6pEUBLv6m\", \"txinwitness\": [\"3045022100adc807e16f2b5e85e97a3406c1f43ec03ef06570d9ae5f57f458a1a3c406914d022018fa00d300c8c8a46b5afe7d7068cc015a1eebd498dab711b63cbea837bf2e6601\", \"03b755316b1d173d4878444585d3c9cd06d7dd81361f435778240002a95983730c\"], \"script_signature\": \"160014f1b2c405281f2fb6f925cdf566b96934eb372395\"}], \"lock_time\": 1348882, \"hash\": \"1175060a9fd2063cf9f4a838b6655b40a6422ab0bffd05befd8173af47919691\", \"outputs\": [{\"script_hex\": \"a9143c8eb6688c3fae732f83639fab5f96e2e977e91e87\", \"output_index\": 0, \"value\": 50000000, \"address\": \"2MxmRVdkLwuUyzUAaApzTZuFvGR7JdPaqab\"}, {\"script_hex\": \"a914ce55485e55fb7e8ec68dc64f8be4ba7b12b4a08987\", \"output_index\": 1, \"value\": 14696979, \"address\": \"2NC4DMM2kLDLJu6qEkMHui5aT9G8suTtByB\"}], \"amount\": 64696979, \"confirmations\": 0, \"fees\": 5020, \"received_at\": \"2018-07-02T10:14:25Z\", \"block\": null}";
			const std::string TX_3 = "{\"inputs\": [{\"value\": 50000000, \"output_index\": 0, \"output_hash\": \"6196855456be3c75aba558e8341700950c7e23bb0610f59c1bec98a8c4c981de\", \"input_index\": 0, \"address\": \"2MuhZ8X1noZWqoPJ3JUSbMYbt4yQrVZbTh1\", \"txinwitness\": [\"304402204ae13666b0fdc3f8bf70342fbf7cb2bf66f2f479f86adc69a62cd7ceb57bff5c022052d263936c5430e5924d264306197a0b8e08852e744fdc74b70fdabe3ac1731501\", \"03645b166c3a6fffc6de27165f3eda6266b8422ab1b55c98837f7e8a5fce07b535\"], \"script_signature\": \"160014f0f872e44749efd470b9e3dafde6615d312e10a7\"}], \"lock_time\": 1348881, \"hash\": \"532025b81f7e5c45854af520faf36ed06efe946e79cf09893b3673b5e257c4be\", \"outputs\": [{\"script_hex\": \"a9143c8eb6688c3fae732f83639fab5f96e2e977e91e87\", \"output_index\": 0, \"value\": 10000000, \"address\": \"2MxmRVdkLwuUyzUAaApzTZuFvGR7JdPaqab\"}, {\"script_hex\": \"a91420bb0f4bb062b2ee309f851f8f7beb8ab3032aba87\", \"output_index\": 1, \"value\": 39997560, \"address\": \"2MvEHhZXsiCqHG2bLWTiDL86BDHJX42rZc9\"}], \"amount\": 49997560, \"confirmations\": 2, \"fees\": 2440, \"received_at\": \"2018-07-02T10:05:54Z\", \"block\": {\"time\": \"2018-07-02T10:12:25Z\", \"hash\": \"00000000000002a3d50abf521eac25796953cfcdd906e11dae0883dcbc53d370\", \"height\": 1348882}}";
			const std::string TX_4 = "{\"inputs\": [{\"value\": 9993400, \"output_index\": 1, \"output_hash\": \"6196855456be3c75aba558e8341700950c7e23bb0610f59c1bec98a8c4c981de\", \"input_index\": 0, \"address\": \"2MwwW2GNkN9zsdJzCfZt1SPuPJxbv3mwVXG\", \"txinwitness\": [\"3045022100f54a61530667c793d4f9815c2445b32b5c962af8f7fb4fa1d88eac0c157fd46d022039ae85ea647e24c6823b29b289e82954721ce4162ebe6c7ef01fa837dd0a121901\", \"02c62972da51fb4020152659bf52371b2d192bc23b7c64219388d8e46a42704381\"], \"script_signature\": \"160014d2a82bd37b996d0b91ebe34781777a62c6aa743d\"}, {\"value\": 30000000, \"output_index\": 0, \"output_hash\": \"879cdf8c89e9dd30307c03a3e60034aedf26cd13cdde6393930a7e186b7b8a9a\", \"input_index\": 1, \"address\": \"2Mv88FZveoQiXdhEzQMZi1ij5V32LcqddHc\", \"txinwitness\": [\"30440220435063feb72a11a359cb38087400b9ab7a974862b35061600852f1ec2df5815b022060e2f218058e9242b64555bfd44755673d93dbba5eb2eb64ada3154afc25228d01\", \"037a7e51b371b8ec48fca551fb83c92aef4f921609fb34e4da6a19b8b9ef8545f3\"], \"script_signature\": \"1600143d0d1c6769fbe08395719a729fd0a9b89155e16a\"}], \"lock_time\": 1348881, \"hash\": \"c154162c627bfe417ee61ff0e4d27c46fdff14454ff44b4c3c4f5f5932dd8520\", \"outputs\": [{\"script_hex\": \"a9143c8eb6688c3fae732f83639fab5f96e2e977e91e87\", \"output_index\": 0, \"value\": 20000000, \"address\": \"2MxmRVdkLwuUyzUAaApzTZuFvGR7JdPaqab\"}, {\"script_hex\": \"a914e4db7d48bde699e5a65bda2751713203b6112fb587\", \"output_index\": 1, \"value\": 19990100, \"address\": \"2NE7JzQphDhWXNLxS5DtvZiQahFRBWtPzuN\"}], \"amount\": 39990100, \"confirmations\": 2, \"fees\": 3300, \"received_at\": \"2018-07-02T10:06:36Z\", \"block\": {\"time\": \"2018-07-02T10:12:25Z\", \"hash\": \"00000000000002a3d50abf521eac25796953cfcdd906e11dae0883dcbc53d370\", \"height\": 1348882}}";
			const std::string TX_5 = "{\"inputs\": [{\"value\": 50000000, \"output_index\": 0, \"output_hash\": \"239e9dd322ebc098c6bc272910024a6a3dedefff6581a3e687d0752eac6963f4\", \"input_index\": 0, \"address\": \"2N46HxqkAbCrPGKyobjzvpAevsL5cGEHtdN\", \"txinwitness\": [\"3044022035edc69e71818e6c58cbcc7eecd4765210ff33d686fcfd18dfa8251d7619d87a022005f892d9db70548886c5ace2a6fc9abb96addf4062cece68cd9bd776e1b520a801\", \"039897deecd59cc450033d68f882ff485785be5bee0a3b3de814a979dc6ec07315\"], \"script_signature\": \"1600148e916e7c0655b61d4a597c5a065206cb4609816a\"}], \"lock_time\": 1348881, \"hash\": \"5feae3154a84d256be4cc824b0d1271bcf35052e1dafac82ba29a75ec3ea6abb\", \"outputs\": [{\"script_hex\": \"a9143c8eb6688c3fae732f83639fab5f96e2e977e91e87\", \"output_index\": 0, \"value\": 30000000, \"address\": \"2MxmRVdkLwuUyzUAaApzTZuFvGR7JdPaqab\"}, {\"script_hex\": \"a914f8ed56e459b571e5bf45ccd8eb46f4dd13d6af9687\", \"output_index\": 1, \"value\": 19997560, \"address\": \"2NFwRtVNhmLqQ6RhLDTYWKnWAjKfR8yp3wU\"}], \"amount\": 49997560, \"confirmations\": 2, \"fees\": 2440, \"received_at\": \"2018-07-02T10:07:25Z\", \"block\": {\"time\": \"2018-07-02T10:12:25Z\", \"hash\": \"00000000000002a3d50abf521eac25796953cfcdd906e11dae0883dcbc53d370\", \"height\": 1348882}}";
		}
	}
}
