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

#include "RippleLikeTransactionBuilder.h"
#include <math/BigInt.h>
#include <api/RippleLikeTransactionCallback.hpp>
#include <wallet/ripple/api_impl/RippleLikeTransactionApi.h>

namespace ledger {
    namespace core {

        RippleLikeTransactionBuilder::RippleLikeTransactionBuilder(
                const std::shared_ptr<api::ExecutionContext> &context,
                const api::Currency &currency,
                const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer,
                const std::shared_ptr<spdlog::logger> &logger,
                const RippleLikeTransactionBuildFunction &buildFunction) {
            _context = context;
            _currency = currency;
            _explorer = explorer;
            _build = buildFunction;
            _logger = logger;
            _request.wipe = false;
        }

        RippleLikeTransactionBuilder::RippleLikeTransactionBuilder(const RippleLikeTransactionBuilder &cpy) {
            _currency = cpy._currency;
            _build = cpy._build;
            _request = cpy._request;
            _context = cpy._context;
            _logger = cpy._logger;
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder>
        RippleLikeTransactionBuilder::sendToAddress(const std::shared_ptr<api::Amount> &amount,
                                                    const std::string &address) {
            _request.value = std::make_shared<BigInt>(amount->toString());
            _request.toAddress = address;
            return shared_from_this();
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder>
        RippleLikeTransactionBuilder::wipeToAddress(const std::string &address) {
            _request.toAddress = address;
            _request.wipe = true;
            return shared_from_this();
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder>
        RippleLikeTransactionBuilder::setFees(const std::shared_ptr<api::Amount> & fees) {
            _request.fees = std::make_shared<BigInt>(fees->toString());
            return shared_from_this();
        }

        void RippleLikeTransactionBuilder::build(const std::shared_ptr<api::RippleLikeTransactionCallback> &callback) {
            build().callback(_context, callback);
        }

        Future<std::shared_ptr<api::RippleLikeTransaction>> RippleLikeTransactionBuilder::build() {
            return _build(_request, _explorer);
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder> RippleLikeTransactionBuilder::clone() {
            return std::make_shared<RippleLikeTransactionBuilder>(*this);
        }

        void RippleLikeTransactionBuilder::reset() {
            _request = RippleLikeTransactionBuildRequest();
        }

        std::shared_ptr<api::RippleLikeTransaction>
        api::RippleLikeTransactionBuilder::parseRawUnsignedTransaction(const api::Currency &currency,
                                                                       const std::vector<uint8_t> &rawTransaction) {
            return ::ledger::core::RippleLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, false);
        }

        std::shared_ptr<api::RippleLikeTransaction>
        api::RippleLikeTransactionBuilder::parseRawSignedTransaction(const api::Currency &currency,
                                                                     const std::vector<uint8_t> &rawTransaction) {
            return ::ledger::core::RippleLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, true);
        }

        std::shared_ptr<api::RippleLikeTransaction>
        RippleLikeTransactionBuilder::parseRawTransaction(const api::Currency &currency,
                                                          const std::vector<uint8_t> &rawTransaction,
                                                          bool isSigned) {
            return nullptr;
        }

    }
}