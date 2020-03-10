/*
 *
 * CosmosLikeTransactionBuilder
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


#ifndef LEDGER_CORE_COSMOSLIKETRANSACTIONBUILDER_H
#define LEDGER_CORE_COSMOSLIKETRANSACTIONBUILDER_H

#include <spdlog/logger.h>

#include <core/api/Currency.hpp>
#include <core/api/ExecutionContext.hpp>
#include <core/api/Amount.hpp>
#include <core/wallet/Amount.hpp>
#include <core/math/BigInt.hpp>
#include <core/async/Future.hpp>

#include <cosmos/api/CosmosLikeTransactionBuilder.hpp>
#include <cosmos/explorers/CosmosLikeBlockchainExplorer.hpp>
#include <cosmos/api/CosmosLikeTransactionCallback.hpp>

namespace ledger {
    namespace core {

        struct CosmosLikeTransactionBuildRequest {
            CosmosLikeTransactionBuildRequest() {
                wipe = false;
            };
            std::shared_ptr<BigInt> fee;
            std::shared_ptr<BigInt> gas;
            std::string memo;
            std::string sequence;
            std::vector<std::shared_ptr<api::CosmosLikeMessage>> messages;

            bool wipe;
        };

        using CosmosLikeTransactionBuildFunction = std::function<Future<std::shared_ptr<api::CosmosLikeTransaction>>(
                const CosmosLikeTransactionBuildRequest &, const std::shared_ptr<CosmosLikeBlockchainExplorer> &)>;

        class CosmosLikeTransactionBuilder
                : public api::CosmosLikeTransactionBuilder,
                  public std::enable_shared_from_this<CosmosLikeTransactionBuilder> {
        public:

            explicit CosmosLikeTransactionBuilder(const std::shared_ptr<api::ExecutionContext> &context,
                                                  const api::Currency &params,
                                                  const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer,
                                                  const std::shared_ptr<spdlog::logger> &logger,
                                                  const CosmosLikeTransactionBuildFunction &buildFunction);

            CosmosLikeTransactionBuilder(const CosmosLikeTransactionBuilder &cpy);

            std::shared_ptr<api::CosmosLikeTransactionBuilder> setMemo(const std::string & memo) override;

            std::shared_ptr<api::CosmosLikeTransactionBuilder> setSequence(const std::string & sequence) override;

            std::shared_ptr<api::CosmosLikeTransactionBuilder> addMessage(const std::shared_ptr<api::CosmosLikeMessage> & msg) override;

            std::shared_ptr<api::CosmosLikeTransactionBuilder> setGas(const std::shared_ptr<api::Amount> & gas) override;

            std::shared_ptr<api::CosmosLikeTransactionBuilder> setFee(const std::shared_ptr<api::Amount> & fee) override;

            void build(const std::shared_ptr<api::CosmosLikeTransactionCallback> &callback) override;

            Future<std::shared_ptr<api::CosmosLikeTransaction>> build();

            std::shared_ptr<api::CosmosLikeTransactionBuilder> clone() override;

            void reset() override;

            static std::shared_ptr<api::CosmosLikeTransaction> parseRawTransaction(const api::Currency &currency,
                                                                                   const std::string &rawTransaction,
                                                                                   bool isSigned);

        private:
            api::Currency _currency;
            std::shared_ptr<CosmosLikeBlockchainExplorer> _explorer;
            CosmosLikeTransactionBuildFunction _build;
            CosmosLikeTransactionBuildRequest _request;
            std::shared_ptr<api::ExecutionContext> _context;
            std::shared_ptr<spdlog::logger> _logger;

        };
    }
}

#endif //LEDGER_CORE_COSMOSLIKETRANSACTIONBUILDER_H
