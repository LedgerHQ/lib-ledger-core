/*
 *
 * BitcoinLikeTransactionBuilder.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/03/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include <api/BitcoinLikeTransactionBuilder.hpp>
#include <api/BitcoinLikePickingStrategy.hpp>
#include <api/BitcoinLikeNetworkParameters.hpp>
#include <api/BitcoinLikeTransaction.hpp>
#include <api/Amount.hpp>
#include <functional>
#include <list>
#include <async/Future.hpp>
#include <math/BigInt.h>
#include <spdlog/logger.h>
#include <api/Currency.hpp>

namespace ledger {
    namespace core {

        struct BitcoinLikeTransactionBuildRequest {
            BitcoinLikeTransactionBuildRequest(const std::shared_ptr<BigInt>& minChange);
            std::list<std::tuple<std::string, int32_t, uint32_t>> inputs;
            std::list<std::tuple<std::shared_ptr<BigInt>, std::shared_ptr<api::BitcoinLikeScript>>> outputs;
            std::list<std::string> changePaths;
            std::list<std::tuple<std::string, int32_t>> excludedUtxo;
            int32_t changeCount;
            std::shared_ptr<BigInt> feePerByte;
            Option<std::tuple<api::BitcoinLikePickingStrategy, uint32_t>> utxoPicker;
            std::shared_ptr<BigInt> maxChange;
            std::shared_ptr<BigInt> minChange;
            bool wipe;
        };

        using BitcoinLikeTransactionBuildFunction = std::function<Future<std::shared_ptr<api::BitcoinLikeTransaction>> (const BitcoinLikeTransactionBuildRequest&)>;

        class BitcoinLikeTransactionBuilder : public api::BitcoinLikeTransactionBuilder, public std::enable_shared_from_this<BitcoinLikeTransactionBuilder> {
        public:
            explicit BitcoinLikeTransactionBuilder(
                    const std::shared_ptr<api::ExecutionContext>& context,
                    const api::Currency& params,
                    const std::shared_ptr<spdlog::logger>& logger,
                    const BitcoinLikeTransactionBuildFunction& buildFunction);
            BitcoinLikeTransactionBuilder(const BitcoinLikeTransactionBuilder& cpy);
            std::shared_ptr<api::BitcoinLikeTransactionBuilder>
            addInput(const std::string &transactionHash, int32_t index, int32_t sequence) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder> addOutput(const std::shared_ptr<api::Amount> &amount,
                                                                          const std::shared_ptr<api::BitcoinLikeScript> &script) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder> addChangePath(const std::string &path) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder>
            excludeUtxo(const std::string &transactionHash, int32_t outputIndex) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder> setNumberOfChangeAddresses(int32_t count) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder>
            pickInputs(api::BitcoinLikePickingStrategy strategy, int32_t sequence) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder>
            sendToAddress(const std::shared_ptr<api::Amount> &amount, const std::string &address) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder>
            wipeToAddress(const std::string &address) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder>
            setFeesPerByte(const std::shared_ptr<api::Amount> &fees) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder>
            setMaxAmountOnChange(const std::shared_ptr<api::Amount> &amount) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder>
            setMinAmountOnChange(const std::shared_ptr<api::Amount> &amount) override;

            std::shared_ptr<api::BitcoinLikeTransactionBuilder> clone() override;

            void reset() override;

            void build(const std::shared_ptr<api::BitcoinLikeTransactionCallback> &callback) override;
            Future<std::shared_ptr<api::BitcoinLikeTransaction>> build();
        private:
            api::Currency _currency;
            std::shared_ptr<api::BitcoinLikeScript> createSendScript(const std::string &address);
            BitcoinLikeTransactionBuildFunction _build;
            BitcoinLikeTransactionBuildRequest _request;
            std::shared_ptr<api::ExecutionContext> _context;
            std::shared_ptr<spdlog::logger> _logger;

        };
    }
}
