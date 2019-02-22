/*
 *
 * EthereumLikeTransactionBuilder
 *
 * Created by El Khalil Bellakrid on 12/07/2018.
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


#ifndef LEDGER_CORE_ETHEREUMLIKETRANSACTIONBUILDER_H
#define LEDGER_CORE_ETHEREUMLIKETRANSACTIONBUILDER_H

#include <api/EthereumLikeTransactionBuilder.hpp>
#include <api/Currency.hpp>
#include <api/ExecutionContext.hpp>
#include <api/Amount.hpp>

#include <wallet/common/Amount.h>
#include <wallet/ethereum/explorers/EthereumLikeBlockchainExplorer.h>

#include <math/BigInt.h>

#include <async/Future.hpp>
#include <spdlog/logger.h>

namespace ledger {
    namespace core {

        struct EthereumLikeTransactionBuildRequest {
            EthereumLikeTransactionBuildRequest(){
                wipe = false;
                nonce = std::make_shared<BigInt>(0);
            };
            std::string toAddress;
            std::shared_ptr<BigInt> value;
            std::shared_ptr<BigInt> nonce;
            std::shared_ptr<BigInt> gasPrice;
            std::shared_ptr<BigInt> gasLimit;
            std::vector<uint8_t> inputData;
            bool wipe;
        };

        using EthereumLikeTransactionBuildFunction = std::function<Future<std::shared_ptr<api::EthereumLikeTransaction>> (const EthereumLikeTransactionBuildRequest&, const std::shared_ptr<EthereumLikeBlockchainExplorer> &)>;
        
        class EthereumLikeTransactionBuilder : public api::EthereumLikeTransactionBuilder, public std::enable_shared_from_this<EthereumLikeTransactionBuilder>{
        public:
            
            explicit EthereumLikeTransactionBuilder(const std::shared_ptr<api::ExecutionContext>& context,
                                                    const api::Currency& params,
                                                    const std::shared_ptr<EthereumLikeBlockchainExplorer> &explorer,
                                                    const std::shared_ptr<spdlog::logger>& logger,
                                                    const EthereumLikeTransactionBuildFunction& buildFunction);

            EthereumLikeTransactionBuilder(const EthereumLikeTransactionBuilder& cpy);
            
            std::shared_ptr<api::EthereumLikeTransactionBuilder> sendToAddress(const std::shared_ptr<api::Amount> & amount,
                                                                               const std::string & address) override;

            std::shared_ptr<api::EthereumLikeTransactionBuilder> wipeToAddress(const std::string & address) override;
            
            std::shared_ptr<api::EthereumLikeTransactionBuilder> setGasPrice(const std::shared_ptr<api::Amount> & gasPrice) override;
            std::shared_ptr<api::EthereumLikeTransactionBuilder> setGasLimit(const std::shared_ptr<api::Amount> & gasLimit) override;
            std::shared_ptr<api::EthereumLikeTransactionBuilder> setInputData(const std::vector<uint8_t> & data) override;

            void build(const std::shared_ptr<api::EthereumLikeTransactionCallback> & callback) override;
            Future<std::shared_ptr<api::EthereumLikeTransaction>> build();

            std::shared_ptr<api::EthereumLikeTransactionBuilder> clone() override;
            
            void reset() override;

            static std::shared_ptr<api::EthereumLikeTransaction> parseRawTransaction(const api::Currency & currency,
                                                                              const std::vector<uint8_t> & rawTransaction,
                                                                              bool isSigned);
        private:
            api::Currency _currency;
            std::shared_ptr<EthereumLikeBlockchainExplorer> _explorer;
            EthereumLikeTransactionBuildFunction _build;
            EthereumLikeTransactionBuildRequest _request;
            std::shared_ptr<api::ExecutionContext> _context;
            std::shared_ptr<spdlog::logger> _logger;
            
        };       
    }
}


#endif //LEDGER_CORE_ETHEREUMLIKETRANSACTIONBUILDER_H
