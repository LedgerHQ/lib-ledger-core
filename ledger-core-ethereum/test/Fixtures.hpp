#pragma once

#include <gtest/gtest.h>
#include <unordered_set>

#include <CoutLogPrinter.hpp>
#include <NativePathResolver.hpp>
#include <async/AsyncWait.hpp>
#include <async/QtThreadDispatcher.hpp>
#include <net/QtHttpClient.hpp>

#include <core/Services.hpp>
#include <core/api/Account.hpp>
#include <core/api/BigInt.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/events/LambdaEventReceiver.hpp>
#include <core/utils/JSONUtils.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>
#include <core/wallet/CurrencyBuilder.hpp>

#include <ethereum/EthereumLikeAccount.hpp>
#include <ethereum/EthereumLikeWallet.hpp>
#include <ethereum/api/EthereumLikeAccount.hpp>
#include <ethereum/api/EthereumLikeOperation.hpp>
#include <ethereum/api/EthereumLikeTransaction.hpp>
#include <ethereum/explorers/EthereumLikeTransactionParser.hpp>
#include <ethereum/database/EthereumLikeTransactionDatabaseHelper.hpp>

namespace ledger {
	namespace testing {
		namespace eth_xpub {
			extern core::api::ExtendedKeyAccountCreationInfo XPUB_INFO;
			extern const std::string TX_1;
			extern const std::string TX_2;
			extern const std::string TX_3;
			extern const std::string TX_4;
			extern const std::string TX_5;
			extern const std::string TX_6;

			std::shared_ptr<core::EthereumLikeAccount> inflate(const std::shared_ptr<core::Services>& services, const std::shared_ptr<core::AbstractWallet>& wallet);
		}
	}
}
