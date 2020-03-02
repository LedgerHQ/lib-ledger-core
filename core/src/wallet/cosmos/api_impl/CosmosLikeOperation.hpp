/*
 *
 * CosmosLikeOperation
 *
 * Created by El Khalil Bellakrid on  14/06/2019.
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


#ifndef LEDGER_CORE_COSMOSLIKEOPERATION_H
#define LEDGER_CORE_COSMOSLIKEOPERATION_H


#include <cosmos/cosmos.hpp>
#include <cosmos/api/CosmosLikeOperation.hpp>
#include <cosmos/api/CosmosLikeTransaction.hpp>
#include <cosmos/api/CosmosLikeMessage.hpp>

#include <core/operation/Operation.hpp>
#include <core/wallet/AbstractWallet.hpp>
#include <core/wallet/AbstractAccount.hpp>

namespace ledger {
    namespace core {
        class CosmosLikeOperation : public api::CosmosLikeOperation, public Operation {

            public:

                CosmosLikeOperation() = default;

                CosmosLikeOperation(ledger::core::cosmos::Transaction const& tx,
                                    ledger::core::cosmos::Message const& msg);

                void setTransactionData(ledger::core::cosmos::Transaction const& txData);

                void setMessageData(ledger::core::cosmos::Message const& msgData);

                virtual std::shared_ptr<api::CosmosLikeTransaction> getTransaction() override;
                virtual std::shared_ptr<api::CosmosLikeMessage> getMessage() override;

                virtual void refreshUid(const std::string &msgIndex) override;

            private:

                std::shared_ptr<api::CosmosLikeTransaction> _txApi {nullptr};
                std::shared_ptr<api::CosmosLikeMessage> _msgApi {nullptr};

        };

    }
}

#endif //LEDGER_CORE_COSMOSLIKEOPERATION_H
