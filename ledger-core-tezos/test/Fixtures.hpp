#pragma once

#include <gtest/gtest.h>
#include <unordered_set>
#include <soci.h>

#include <CoutLogPrinter.hpp>
#include <NativePathResolver.hpp>
#include <async/QtThreadDispatcher.hpp>
#include <async/AsyncWait.hpp>
#include <net/QtHttpClient.hpp>

#include <core/Services.hpp>
#include <core/api/Account.hpp>
#include <core/api/BigInt.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/events/LambdaEventReceiver.hpp>
#include <core/utils/Hex.hpp>
#include <core/utils/JSONUtils.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>
#include <core/wallet/CurrencyBuilder.hpp>
#include <core/wallet/WalletDatabaseHelper.hpp>

#include <tezos/TezosLikeAccount.hpp>
#include <tezos/TezosLikeWallet.hpp>
#include <tezos/api/TezosLikeAccount.hpp>
#include <tezos/api/TezosLikeOperation.hpp>
#include <tezos/api/TezosLikeTransaction.hpp>
#include <tezos/database/TezosLikeTransactionDatabaseHelper.hpp>
#include <tezos/explorers/TezosLikeTransactionParser.hpp>

namespace ledger {
	namespace testing {
		namespace xtz {
			extern core::api::AccountCreationInfo XPUB_INFO;
			extern const std::string TX_1;
			extern const std::string TX_2;
			extern const std::string TX_3;
			extern const std::string TX_4;
			extern const std::string TX_5;
			extern const std::string TX_6;
			extern const std::string TX_7;
			extern const std::string TX_8;

			std::shared_ptr<core::TezosLikeAccount> inflate(const std::shared_ptr<core::Services>& services, const std::shared_ptr<core::AbstractWallet>& wallet);
		}
	}
}
