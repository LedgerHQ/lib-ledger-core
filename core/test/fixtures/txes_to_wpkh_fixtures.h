#ifndef LEDGER_FIXTURES_TXES_TO_WPKH
#define LEDGER_FIXTURES_TXES_TO_WPKH
#include <CoutLogPrinter.hpp>
#include <CppHttpLibClient.hpp>
#include <NativePathResolver.hpp>
#include <UvThreadDispatcher.hpp>
#include <api/Account.hpp>
#include <api/BigInt.hpp>
#include <api/BitcoinLikeAccount.hpp>
#include <api/BitcoinLikeInput.hpp>
#include <api/BitcoinLikeOperation.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BitcoinLikeTransaction.hpp>
#include <events/LambdaEventReceiver.hpp>
#include <gtest/gtest.h>
#include <soci.h>
#include <src/api/DynamicObject.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <src/wallet/pool/WalletPool.hpp>
#include <unordered_set>
#include <utils/JSONUtils.h>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>
#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>
#include <wallet/common/CurrencyBuilder.hpp>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>

namespace ledger {
    namespace testing {
        namespace txes_to_wpkh {
            extern core::api::ExtendedKeyAccountCreationInfo XPUB_INFO;

            std::shared_ptr<core::BitcoinLikeAccount> inflate(const std::shared_ptr<core::WalletPool> &pool, const std::shared_ptr<core::AbstractWallet> &wallet);
        } // namespace txes_to_wpkh
    }     // namespace testing
} // namespace ledger

#endif // LEDGER_FIXTURES_TXES_TO_WPKH
