/*
 *
 * CosmosLikeBlockchainExplorerAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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


#include <cosmos/CosmosLikeAccount.hpp>
#include <cosmos/synchronizers/CosmosLikeBlockchainExplorerAccountSynchronizer.hpp>

namespace ledger {
namespace core {
CosmosLikeBlockchainExplorerAccountSynchronizer::CosmosLikeBlockchainExplorerAccountSynchronizer(
    const std::shared_ptr<Services> &services,
    const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer)
    : DedicatedContext(services->getDispatcher()->getThreadPoolExecutionContext("synchronizers")) {
    _explorer = explorer;
    _database = services->getDatabaseSessionPool();
}

std::shared_ptr<ProgressNotifier<Unit>>
CosmosLikeBlockchainExplorerAccountSynchronizer::synchronize(
    const std::shared_ptr<CosmosLikeAccount> &account) {
    return synchronizeAccount(account);
}

bool CosmosLikeBlockchainExplorerAccountSynchronizer::isSynchronizing() const {
    return _notifier != nullptr;
}

void CosmosLikeBlockchainExplorerAccountSynchronizer::reset(
    const std::shared_ptr<CosmosLikeAccount> &account,
    const std::chrono::system_clock::time_point &toDate) {}

void CosmosLikeBlockchainExplorerAccountSynchronizer::updateCurrentBlock(
    std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer::SynchronizationBuddy> &buddy,
    const std::shared_ptr<api::ExecutionContext> &context) {
    _explorer->getCurrentBlock().onComplete(
        context, [buddy](const TryPtr<api::Block> &block) {
            if (block.isSuccess()) {
             soci::session sql(buddy->account->getWallet()->getDatabase()->getPool());
             buddy->account->putBlock(sql, *block.getValue());
            }
        });
}

void CosmosLikeBlockchainExplorerAccountSynchronizer::updateTransactionsToDrop(
    soci::session &sql,
    std::shared_ptr<SynchronizationBuddy> &buddy,
    const std::string &accountUid) {
    // Get all transactions in DB that may be dropped (txs without block_uid)
    soci::rowset<soci::row> rows =
        (sql.prepare
             << "SELECT op.uid, atom_msg.transaction_uid FROM operations AS op "
                "LEFT OUTER JOIN cosmos_operations AS atom_op ON atom_op.uid = op.uid "
                "LEFT OUTER JOIN cosmos_messages AS atom_msg ON atom_op.message_uid = atom_msg.uid "
                "WHERE op.block_uid IS NULL AND op.account_uid = :uid ",
         soci::use(accountUid));

    for (auto &row : rows) {
        if (row.get_indicator(0) != soci::i_null && row.get_indicator(1) != soci::i_null) {
            buddy->transactionsToDrop.insert(std::pair<std::string, std::string>(
                row.get<std::string>(1), row.get<std::string>(0)));
        }
    }
}

std::shared_ptr<CosmosBlockchainAccountSynchronizer>
CosmosLikeBlockchainExplorerAccountSynchronizer::getSharedFromThis() {
    return shared_from_this();
}

std::shared_ptr<api::ExecutionContext>
CosmosLikeBlockchainExplorerAccountSynchronizer::getSynchronizerContext() {
    return getContext();
}
}  // namespace core
}  // namespace ledger
