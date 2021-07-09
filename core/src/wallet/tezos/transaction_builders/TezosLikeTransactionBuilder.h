/*
 *
 * TezosLikeTransactionBuilder
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#ifndef LEDGER_CORE_TEZOSLIKETRANSACTIONBUILDER_H
#define LEDGER_CORE_TEZOSLIKETRANSACTIONBUILDER_H

#include <api/TezosLikeTransactionBuilder.hpp>
#include <api/Currency.hpp>
#include <api/ExecutionContext.hpp>
#include <api/Amount.hpp>
#include <api/TezosOperationTag.hpp>

#include <wallet/common/Amount.h>
#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>

#include <math/BigInt.h>

#include <async/Future.hpp>
#include <spdlog/logger.h>

namespace ledger {
    namespace core {

        struct TezosLikeTransactionBuildRequest {
            TezosLikeTransactionBuildRequest() {
                wipe = false;
                type = api::TezosOperationTag::OPERATION_TAG_TRANSACTION;
            };
            std::string toAddress;
            std::shared_ptr<BigInt> value;
            std::shared_ptr<BigInt> transactionFees;
            std::shared_ptr<BigInt> transactionGasLimit;
            std::shared_ptr<BigInt> storageLimit;
            std::shared_ptr<BigInt> revealFees;
            std::shared_ptr<BigInt> revealGasLimit;
            api::TezosOperationTag type;
            bool wipe;
            std::string correlationId;
        };

        using TezosLikeTransactionBuildFunction = std::function<Future<std::shared_ptr<api::TezosLikeTransaction>>(
                const TezosLikeTransactionBuildRequest &, const std::shared_ptr<TezosLikeBlockchainExplorer> &)>;

        class TezosLikeTransactionBuilder : public api::TezosLikeTransactionBuilder,
                                            public std::enable_shared_from_this<TezosLikeTransactionBuilder> {
        public:

            explicit TezosLikeTransactionBuilder(const std::string &senderAddress,
                                                 const std::shared_ptr<api::ExecutionContext> &context,
                                                 const api::Currency &params,
                                                 const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                                                 const std::shared_ptr<spdlog::logger> &logger,
                                                 const TezosLikeTransactionBuildFunction &buildFunction,
                                                 const std::string &protocolUpdate = "");

            TezosLikeTransactionBuilder(const TezosLikeTransactionBuilder &cpy);

            std::shared_ptr<api::TezosLikeTransactionBuilder> setType(api::TezosOperationTag type) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder> sendToAddress(const std::shared_ptr<api::Amount> &amount,
                                                                            const std::string &address) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder> wipeToAddress(const std::string &address) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder>
            setFees(const std::shared_ptr<api::Amount> &fees) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder>
            setTransactionFees(const std::shared_ptr<api::Amount> &transactionFees) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder>
            setRevealFees(const std::shared_ptr<api::Amount> &revealFees) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder>
            setGasLimit(const std::shared_ptr<api::Amount> & gasLimit) override ;

            std::shared_ptr<api::TezosLikeTransactionBuilder>
            setStorageLimit(const std::shared_ptr<api::BigInt> & storageLimit) override;

            std::shared_ptr<api::TezosLikeTransactionBuilder>
            setCorrelationId(const std::string &correlationId) override;

            void build(const std::shared_ptr<api::TezosLikeTransactionCallback> &callback) override;

            Future<std::shared_ptr<api::TezosLikeTransaction>> build();

            std::shared_ptr<api::TezosLikeTransactionBuilder> clone() override;

            void reset() override;

            static std::shared_ptr<api::TezosLikeTransaction> parseRawTransaction(const api::Currency &currency,
                                                                                  const std::vector<uint8_t> &rawTransaction,
                                                                                  bool isSigned,
                                                                                  const std::string &protocolUpdate);

        private:
            api::Currency _currency;
            std::shared_ptr<TezosLikeBlockchainExplorer> _explorer;
            TezosLikeTransactionBuildFunction _build;
            TezosLikeTransactionBuildRequest _request;
            std::shared_ptr<api::ExecutionContext> _context;
            std::shared_ptr<spdlog::logger> _logger;
            std::string _senderAddress;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKETRANSACTIONBUILDER_H
