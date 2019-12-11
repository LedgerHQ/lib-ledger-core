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
#include <ripple/api/RippleLikeAccount.hpp>
#include <ripple/api/RippleLikeOperation.hpp>
#include <ripple/api/RippleLikeTransaction.hpp>
#include <ripple/explorers/RippleLikeTransactionParser.hpp>
#include <ripple/RippleLikeWallet.hpp>
#include <ripple/RippleLikeTransactionDatabaseHelper.hpp>
#include <ripple/RippleLikeAccount.hpp>

#include <async/QtThreadDispatcher.hpp>
#include <async/AsyncWait.hpp>
#include <net/QtHttpClient.hpp>
#include <NativePathResolver.hpp>
#include <CoutLogPrinter.hpp>

namespace ledger {
	namespace testing {
		namespace xrp {
			extern core::api::AccountCreationInfo XPUB_INFO;
			extern const std::string TX_1;

			std::shared_ptr<core::RippleLikeAccount> inflate(
                const std::shared_ptr<core::Services>& services,
                const std::shared_ptr<core::AbstractWallet>& wallet
            );
		}
	}
}
