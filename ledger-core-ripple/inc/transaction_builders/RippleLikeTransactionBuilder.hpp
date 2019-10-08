/*
 *
 * RippleLikeTransactionBuilder
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#pragma once

#include <spdlog/logger.h>

#include <api/RippleLikeTransactionBuilder.hpp>
#include <core/api/Amount.hpp>
#include <core/api/Currency.hpp>
#include <core/api/ExecutionContext.hpp>
#include <core/async/Future.hpp>
#include <core/math/BigInt.hpp>
#include <core/wallet/Amount.hpp>

#include <explorers/RippleLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {
        struct RippleLikeTransactionBuildRequest {
            RippleLikeTransactionBuildRequest() {
                wipe = false;
            };
            std::string toAddress;
            std::shared_ptr<BigInt> value;
            std::shared_ptr<BigInt> fees;
            BigInt sequence;
            bool wipe;
            std::vector<api::RippleLikeMemo> memos;
            Option<int64_t> destinationTag;
        };

        using RippleLikeTransactionBuildFunction = std::function<Future<std::shared_ptr<api::RippleLikeTransaction>>(
                const RippleLikeTransactionBuildRequest &, const std::shared_ptr<RippleLikeBlockchainExplorer> &)>;

        class RippleLikeTransactionBuilder
                : public api::RippleLikeTransactionBuilder,
                  public std::enable_shared_from_this<RippleLikeTransactionBuilder> {
        public:

            explicit RippleLikeTransactionBuilder(const std::shared_ptr<api::ExecutionContext> &context,
                                                  const api::Currency &params,
                                                  const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer,
                                                  const std::shared_ptr<spdlog::logger> &logger,
                                                  const RippleLikeTransactionBuildFunction &buildFunction);

            RippleLikeTransactionBuilder(const RippleLikeTransactionBuilder &cpy);

            std::shared_ptr<api::RippleLikeTransactionBuilder> sendToAddress(const std::shared_ptr<api::Amount> &amount,
                                                                             const std::string &address) override;

            std::shared_ptr<api::RippleLikeTransactionBuilder> wipeToAddress(const std::string &address) override;

            std::shared_ptr<api::RippleLikeTransactionBuilder> setFees(const std::shared_ptr<api::Amount> & fees) override;

            std::shared_ptr<api::RippleLikeTransactionBuilder> addMemo(const api::RippleLikeMemo& memo) override;

            std::shared_ptr<api::RippleLikeTransactionBuilder> setDestinationTag(int64_t tag) override;

            void build(const std::function<void(std::shared_ptr<api::RippleLikeTransaction>, std::experimental::optional<api::Error>)> & callback) override;

            Future<std::shared_ptr<api::RippleLikeTransaction>> build();

            std::shared_ptr<api::RippleLikeTransactionBuilder> clone() override;

            void reset() override;

            static std::shared_ptr<api::RippleLikeTransaction> parseRawTransaction(const api::Currency &currency,
                                                                                   const std::vector<uint8_t> &rawTransaction,
                                                                                   bool isSigned);

        private:
            api::Currency _currency;
            std::shared_ptr<RippleLikeBlockchainExplorer> _explorer;
            RippleLikeTransactionBuildFunction _build;
            RippleLikeTransactionBuildRequest _request;
            std::shared_ptr<api::ExecutionContext> _context;
            std::shared_ptr<spdlog::logger> _logger;

        };
    }
}
