/*
 * AlgorandBlockchainObserver
 *
 * Created by Hakim Aammar on 20/04/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#ifndef LEDGER_CORE_ALGORANDBLOCKCHAINOBSERVER_H
#define LEDGER_CORE_ALGORANDBLOCKCHAINOBSERVER_H

#include "model/transactions/AlgorandTransaction.hpp"

#include <core/async/DedicatedContext.hpp>
#include <core/utils/ConfigurationMatchable.hpp>
#include <core/observers/AbstractBlockchainObserver.hpp>
#include <core/observers/AbstractLedgerApiBlockchainObserver.hpp>

#include <core/api/Block.hpp>

namespace ledger {
namespace core {
namespace algorand {

    class Account;

    using AlgorandAbstractBlockchainObserver = AbstractBlockchainObserver<Account, model::Transaction, api::Block>;

    class BlockchainObserver : public AlgorandAbstractBlockchainObserver,
                                       public AbstractLedgerApiBlockchainObserver,
                                       public DedicatedContext,
                                       public ConfigurationMatchable,
                                       public std::enable_shared_from_this<BlockchainObserver> {

    public:

        BlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                    const std::shared_ptr<api::DynamicObject> &configuration,
                                    const std::shared_ptr<spdlog::logger> &logger,
                                    const api::Currency &currency,
                                    const std::vector<std::string> &matchableKeys);

        BlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                   const std::shared_ptr<WebSocketClient> &client,
                                   const std::shared_ptr<api::DynamicObject> &configuration,
                                   const std::shared_ptr<spdlog::logger> &logger,
                                   const api::Currency &currency);

    protected:

        void putTransaction(const model::Transaction &tx) override;

        void putBlock(const api::Block &block) override;

        void onStart() override;

        void onStop() override;

        const api::Currency &getCurrency() const {
            return _currency;
        };

        std::shared_ptr<api::DynamicObject> getConfiguration() const {
            return _configuration;
        };

    private:

        void connect() override;

        void reconnect() override;

        void onMessage(const std::string &message) override;

        std::shared_ptr<spdlog::logger> logger() const override;

        api::Currency _currency;
        std::shared_ptr<api::DynamicObject> _configuration;
        std::shared_ptr<WebSocketClient> _client;
        WebSocketEventHandler _handler;
    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDBLOCKCHAINOBSERVER_H
