/*
 *
 * EthereumBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
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


#ifndef LEDGER_CORE_ETHEREUMBLOCKCHAINEXPLORER_H
#define LEDGER_CORE_ETHEREUMBLOCKCHAINEXPLORER_H

#include <string>

#include <net/HttpClient.hpp>
#include <api/DynamicObject.hpp>
#include <api/ExecutionContext.hpp>
#include <api/EthereumLikeNetworkParameters.hpp>

#include <utils/ConfigurationMatchable.h>
#include <utils/Option.hpp>
#include <wallet/common/Block.h>
#include <async/DedicatedContext.hpp>
#include <collections/DynamicObject.hpp>
#include <math/BigInt.h>

namespace ledger {
    namespace core {
        class EthereumLikeBlockchainExplorer : public DedicatedContext, public ConfigurationMatchable, public std::enable_shared_from_this<EthereumLikeBlockchainExplorer>{
        public:

            typedef ledger::core::Block Block;

            struct Transaction {
                std::string hash;
                std::chrono::system_clock::time_point receivedAt;
                BigInt value;
                BigInt gasPrice;
                BigInt gasLimit;
                Option<BigInt> gasUsed;
                std::string receiver;
                std::string sender;
                uint64_t nonce;
                Option<Block> block;
                uint64_t confirmations;
                std::vector<uint8_t> inputData;
                Transaction() {
                    nonce = 0;
                    confirmations = 0;
                }
            };

            //EthereumLikeBlockchainExplorer(const std::shared_ptr<api::DynamicObject>& configuration, const std::vector<std::string> &matchableKeys);

            EthereumLikeBlockchainExplorer(const std::shared_ptr<api::ExecutionContext>& context,
                                           const std::shared_ptr<HttpClient>& http,
                                           const api::EthereumLikeNetworkParameters& parameters,
                                           const std::shared_ptr<api::DynamicObject>& configuration);
            Future<std::shared_ptr<BigInt>> getNonce(const std::string &address);

        private:
            std::shared_ptr<HttpClient> _http;
            api::EthereumLikeNetworkParameters _parameters;
        };
    }
}


#endif //LEDGER_CORE_ETHEREUMBLOCKCHAINEXPLORER_H
