/*
 *
 * EthereumLikeAccount
 *
 * Created by El Khalil Bellakrid on 12/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */


#ifndef LEDGER_CORE_ETHEREUMLIKEACCOUNT_H
#define LEDGER_CORE_ETHEREUMLIKEACCOUNT_H

#include <time.h>
#include <api/AddressListCallback.hpp>
#include <api/Address.hpp>
#include <api/EthereumLikeAccount.hpp>
#include <api/EthereumLikeTransactionBuilder.hpp>
#include <api/StringCallback.hpp>
#include <api/Event.hpp>
#include <api/BigIntCallback.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/common/Amount.h>
#include <wallet/ethereum/explorers/EthereumLikeBlockchainExplorer.h>
#include <wallet/ethereum/synchronizers/EthereumLikeAccountSynchronizer.h>
#include <wallet/ethereum/observers/EthereumLikeBlockchainObserver.h>
#include <wallet/ethereum/keychains/EthereumLikeKeychain.hpp>
#include <wallet/ethereum/ERC20/ERC20LikeAccount.h>
#include <wallet/ethereum/database/EthereumLikeAccountDatabaseEntry.h>
namespace ledger {
    namespace core {
        class EthereumLikeAccount : public api::EthereumLikeAccount, public AbstractAccount {
        public:

            static const int FLAG_TRANSACTION_IGNORED = 0x00;
            static const int FLAG_TRANSACTION_CREATED_SENDING_OPERATION = 0x01;
            static const int FLAG_TRANSACTION_CREATED_RECEPTION_OPERATION = 0x01 << 1;
            static const int FLAG_TRANSACTION_CREATED_EXTERNAL_OPERATION = 0x01 << 2; // When a tx from other account creates side effects on current account

            EthereumLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                int32_t index,
                                const std::shared_ptr<EthereumLikeBlockchainExplorer>& explorer,
                                const std::shared_ptr<EthereumLikeBlockchainObserver>& observer,
                                const std::shared_ptr<EthereumLikeAccountSynchronizer>& synchronizer,
                                const std::shared_ptr<EthereumLikeKeychain>& keychain);

            FuturePtr<EthereumLikeBlockchainExplorerTransaction> getTransaction(const std::string& hash);
            void inflateOperation(Operation &out,
                                  const std::shared_ptr<const AbstractWallet>& wallet,
                                  const EthereumLikeBlockchainExplorerTransaction &tx);
            int putTransaction(soci::session& sql, const EthereumLikeBlockchainExplorerTransaction &transaction);
            void updateERC20Accounts(soci::session &sql, const Operation &operation);
            bool putBlock(soci::session& sql, const EthereumLikeBlockchainExplorer::Block& block);

            std::shared_ptr<EthereumLikeKeychain> getKeychain() const;

            FuturePtr<Amount> getBalance() override ;
            Future<AbstractAccount::AddressList> getFreshPublicAddresses() override ;
            Future<std::vector<std::shared_ptr<api::Amount>>>
            getBalanceHistory(const std::string & start,
                              const std::string & end,
                              api::TimePeriod precision) override ;
            Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point & date) override ;

            bool isSynchronizing() override;
            std::shared_ptr<api::EventBus> synchronize() override ;
            void startBlockchainObservation() override ;
            void stopBlockchainObservation() override ;
            bool isObservingBlockchain() override ;
            std::string getRestoreKey() override ;

            static EthereumLikeBlockchainExplorerTransaction getETHLikeBlockchainExplorerTxFromRawTx(const std::shared_ptr<EthereumLikeAccount> &account,
                                                                                                     const std::string &txHash,
                                                                                                     const std::vector<uint8_t> &rawTx);

            void broadcastRawTransaction(const std::vector<uint8_t> & transaction, const std::shared_ptr<api::StringCallback> & callback) override;

            void broadcastTransaction(const std::shared_ptr<api::EthereumLikeTransaction> & transaction,
                                      const std::shared_ptr<api::StringCallback> & callback) override;

            std::shared_ptr<api::EthereumLikeTransactionBuilder> buildTransaction() override;
            std::shared_ptr<api::OperationQuery> queryOperations() override;

            std::shared_ptr<api::EthereumLikeAccount> asEthereumLikeAccount() override ;

            std::vector<std::shared_ptr<api::ERC20LikeAccount>> getERC20Accounts() override ;

            void getGasPrice(const std::shared_ptr<api::BigIntCallback> & callback) override;

            void getEstimatedGasLimit(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) override ;

            void addERC20Accounts(soci::session &sql,
                                  const std::vector<ERC20LikeAccountDatabaseEntry> &erc20Entries);
        private:
            std::shared_ptr<EthereumLikeAccount> getSelf();
            std::shared_ptr<EthereumLikeKeychain> _keychain;
            std::string _accountAddress;
            std::shared_ptr<Preferences> _internalPreferences;
            std::shared_ptr<Preferences> _externalPreferences;
            std::shared_ptr<EthereumLikeBlockchainExplorer> _explorer;
            std::shared_ptr<EthereumLikeAccountSynchronizer> _synchronizer;
            std::shared_ptr<EthereumLikeBlockchainObserver> _observer;
            std::shared_ptr<api::EventBus> _currentSyncEventBus;
            std::mutex _synchronizationLock;
            uint64_t _currentBlockHeight;
            std::vector<ERC20LikeAccountDatabaseEntry> erc20Entries;
            std::vector<std::shared_ptr<api::ERC20LikeAccount> >_erc20LikeAccounts;
        };
    }
}

#endif //LEDGER_CORE_ETHEREUMLIKEACCOUNT_H
