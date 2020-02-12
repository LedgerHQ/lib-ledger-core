#pragma once

#include <gtest/gtest.h>
#include <unordered_set>

#include <core/Services.hpp>
#include <core/api/Account.hpp>
#include <core/api/BigInt.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/events/LambdaEventReceiver.hpp>
#include <core/utils/JSONUtils.hpp>
#include <core/utils/Hex.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>
#include <core/wallet/CurrencyBuilder.hpp>

#include <bitcoin/api/BitcoinLikeAccount.hpp>
#include <bitcoin/BitcoinLikeAccount.hpp>

#include <async/QtThreadDispatcher.hpp>
#include <async/AsyncWait.hpp>
#include <net/QtHttpClient.hpp>
#include <NativePathResolver.hpp>
#include <CoutLogPrinter.hpp>

namespace ledger {
	namespace testing {
		extern core::api::ExtendedKeyAccountCreationInfo P2PKH_MEDIUM_XPUB_INFO;
		extern core::api::ExtendedKeyAccountCreationInfo P2WPKH_MEDIUM_XPUB_INFO;
		extern core::api::AccountCreationInfo P2PKH_MEDIUM_KEYS_INFO;
		extern core::api::ExtendedKeyAccountCreationInfo P2PKH_BIG_XPUB_INFO;
		extern core::api::ExtendedKeyAccountCreationInfo P2SH_XPUB_INFO;

		extern std::string TX_1;
		extern std::string TX_2;
		extern std::string TX_3;
		extern std::string TX_4;
	}
}
