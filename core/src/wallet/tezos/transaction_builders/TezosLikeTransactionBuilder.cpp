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


#include "TezosLikeTransactionBuilder.h"
#include <math/BigInt.h>
#include <api/TezosLikeTransactionCallback.hpp>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
#include <bytes/BytesReader.h>
#include <wallet/currencies.hpp>
#include <crypto/SHA512.hpp>

namespace ledger {
    namespace core {

        TezosLikeTransactionBuilder::TezosLikeTransactionBuilder(
                const std::shared_ptr<api::ExecutionContext> &context,
                const api::Currency &currency,
                const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                const std::shared_ptr<spdlog::logger> &logger,
                const TezosLikeTransactionBuildFunction &buildFunction) {
            _context = context;
            _currency = currency;
            _explorer = explorer;
            _build = buildFunction;
            _logger = logger;
            _request.wipe = false;
        }

        TezosLikeTransactionBuilder::TezosLikeTransactionBuilder(const TezosLikeTransactionBuilder &cpy) {
            _currency = cpy._currency;
            _build = cpy._build;
            _request = cpy._request;
            _context = cpy._context;
            _logger = cpy._logger;
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::sendToAddress(const std::shared_ptr<api::Amount> &amount,
                                                   const std::string &address) {
            _request.value = std::make_shared<BigInt>(amount->toString());
            _request.toAddress = address;
            return shared_from_this();
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::wipeToAddress(const std::string &address) {
            _request.toAddress = address;
            _request.wipe = true;
            return shared_from_this();
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::setFees(const std::shared_ptr<api::Amount> &fees) {
            _request.fees = std::make_shared<BigInt>(fees->toString());
            return shared_from_this();
        }

        void TezosLikeTransactionBuilder::build(const std::shared_ptr<api::TezosLikeTransactionCallback> &callback) {
            build().callback(_context, callback);
        }

        Future<std::shared_ptr<api::TezosLikeTransaction>> TezosLikeTransactionBuilder::build() {
            return _build(_request, _explorer);
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder> TezosLikeTransactionBuilder::clone() {
            return std::make_shared<TezosLikeTransactionBuilder>(*this);
        }

        void TezosLikeTransactionBuilder::reset() {
            _request = TezosLikeTransactionBuildRequest();
        }

        std::shared_ptr<api::TezosLikeTransaction>
        api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(const api::Currency &currency,
                                                                      const std::vector<uint8_t> &rawTransaction) {
            return ::ledger::core::TezosLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, false);
        }

        std::shared_ptr<api::TezosLikeTransaction>
        api::TezosLikeTransactionBuilder::parseRawSignedTransaction(const api::Currency &currency,
                                                                    const std::vector<uint8_t> &rawTransaction) {
            return ::ledger::core::TezosLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, true);
        }

        std::shared_ptr<api::TezosLikeTransaction>
        TezosLikeTransactionBuilder::parseRawTransaction(const api::Currency &currency,
                                                         const std::vector<uint8_t> &rawTransaction,
                                                         bool isSigned) {
            auto tx = std::make_shared<TezosLikeTransactionApi>(currencies::TEZOS);
            BytesReader reader(rawTransaction);
            return tx;
        }
    }
}