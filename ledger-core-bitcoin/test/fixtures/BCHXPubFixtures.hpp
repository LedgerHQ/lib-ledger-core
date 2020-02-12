#pragma once

#include <unordered_set>

#include <soci.h>

#include <gtest/gtest.h>

#include <NativePathResolver.hpp>
#include <CoutLogPrinter.hpp>

#include <net/QtHttpClient.hpp>

#include <async/QtThreadDispatcher.hpp>
#include <async/AsyncWait.hpp>

#include <core/Services.hpp>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>
#include <core/utils/JSONUtils.hpp>
#include <core/api/BigInt.hpp>
#include <core/events/LambdaEventReceiver.hpp>
#include <core/api/Account.hpp>

#include <bitcoin/BitcoinLikeWallet.hpp>
#include <bitcoin/database/BitcoinLikeWalletDatabase.hpp>
#include <bitcoin/database/BitcoinLikeTransactionDatabaseHelper.hpp>
#include <bitcoin/transactions/BitcoinLikeTransactionParser.hpp>
#include <bitcoin/api/BitcoinLikeAccount.hpp>
#include <bitcoin/BitcoinLikeAccount.hpp>
#include <bitcoin/api/BitcoinLikeOperation.hpp>
#include <bitcoin/api/BitcoinLikeTransaction.hpp>
#include <bitcoin/api/BitcoinLikeInput.hpp>
#include <bitcoin/api/BitcoinLikeOutput.hpp>

namespace ledger {
	namespace testing {
		namespace bch_xpub {
			extern core::api::ExtendedKeyAccountCreationInfo XPUB_INFO;

			extern const std::string TX_1;

			std::shared_ptr<core::BitcoinLikeAccount> inflate(const std::shared_ptr<core::Services>& services, const std::shared_ptr<core::AbstractWallet>& wallet);
		}
	}
}

