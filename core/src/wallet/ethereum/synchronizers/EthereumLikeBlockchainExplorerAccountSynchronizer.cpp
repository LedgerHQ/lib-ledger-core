/*
 *
 * EthereumLikeBlockchainExplorerAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 29/07/2018.
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


#include "EthereumLikeBlockchainExplorerAccountSynchronizer.h"

#include <wallet/ethereum/EthereumLikeAccount.h>

namespace ledger {
    namespace core {
        EthereumLikeBlockchainExplorerAccountSynchronizer::EthereumLikeBlockchainExplorerAccountSynchronizer(const std::shared_ptr<WalletPool> &pool,
                                                                                                             const std::shared_ptr<EthereumLikeBlockchainExplorer> &explorer) :
                DedicatedContext(pool->getDispatcher()->getThreadPoolExecutionContext("synchronizers")) {
            _explorer = explorer;
        }

        void EthereumLikeBlockchainExplorerAccountSynchronizer::updateCurrentBlock(std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer::SynchronizationBuddy> &buddy,
                                                                                   const std::shared_ptr<api::ExecutionContext> &context) {
            _explorer->getCurrentBlock().onComplete(context, [buddy] (const TryPtr<EthereumLikeBlockchainExplorer::Block>& block) {
                if (block.isSuccess()) {
                    soci::session sql(buddy->account->getWallet()->getDatabase()->getPool());
                    soci::transaction tr(sql);
                    try {
                        buddy->account->putBlock(sql, *block.getValue());
                        tr.commit();
                    } catch(...) {
                        tr.rollback();
                    }
                }
            });
        }

        void EthereumLikeBlockchainExplorerAccountSynchronizer::updateTransactionsToDrop(soci::session &sql,
                                                                       std::shared_ptr<SynchronizationBuddy> &buddy,
                                                                       const std::string &accountUid) {
            //Get all transactions in DB that may be dropped (txs without block_uid)
            soci::rowset<soci::row> rows = (sql.prepare << "SELECT op.uid, eth_op.transaction_hash FROM operations AS op "
                    "LEFT OUTER JOIN ethereum_operations AS eth_op ON eth_op.uid = op.uid "
                    "WHERE op.block_uid IS NULL AND op.account_uid = :uid ", soci::use(accountUid));

            for (auto &row : rows) {
                if (row.get_indicator(0) != soci::i_null && row.get_indicator(1) != soci::i_null) {
                    buddy->transactionsToDrop.insert(std::pair<std::string, std::string>(row.get<std::string>(1), row.get<std::string>(0)));
                }
            }
        }

        std::shared_ptr<ProgressNotifier<BlockchainExplorerAccountSynchronizationResult>>
        EthereumLikeBlockchainExplorerAccountSynchronizer::synchronize(const std::shared_ptr<EthereumLikeAccount>& account) {
            return synchronizeAccount(account);
        }

        bool EthereumLikeBlockchainExplorerAccountSynchronizer::isSynchronizing() const {
            return _notifier != nullptr;
        }

        void EthereumLikeBlockchainExplorerAccountSynchronizer::reset(const std::shared_ptr<EthereumLikeAccount> &account,
                                                                      const std::chrono::system_clock::time_point &toDate) {

        }

        std::shared_ptr<EthereumBlockchainAccountSynchronizer> EthereumLikeBlockchainExplorerAccountSynchronizer::getSharedFromThis() {
            return shared_from_this();
        }

        std::shared_ptr<api::ExecutionContext> EthereumLikeBlockchainExplorerAccountSynchronizer::getSynchronizerContext() {
            return getContext();
        }

        int EthereumLikeBlockchainExplorerAccountSynchronizer::putTransaction(soci::session &sql,
                                                                              const EthereumLikeBlockchainExplorerTransaction &transaction,
                                                                              const std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer<EthereumLikeAccount, EthereumLikeAddress, EthereumLikeKeychain, EthereumLikeBlockchainExplorer>::SynchronizationBuddy> &buddy) {
            return buddy->account->putTransaction(sql, transaction);
        }
    }
}
