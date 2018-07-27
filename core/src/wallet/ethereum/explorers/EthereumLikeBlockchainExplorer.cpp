/*
 *
 * EthereumLikeBlockchainExplorer
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


#include "EthereumLikeBlockchainExplorer.h"

#include <api/Configuration.hpp>

namespace ledger {
    namespace core {
//        EthereumLikeBlockchainExplorer::EthereumLikeBlockchainExplorer(const std::shared_ptr<api::DynamicObject>& configuration,
//                                                                      const std::vector<std::string> &matchableKeys):
//                                                                      ConfigurationMatchable(matchableKeys) {
//
//        }

//        EthereumLikeBlockchainExplorer::EthereumLikeBlockchainExplorer(const std::shared_ptr<api::ExecutionContext>& context,
//                                                                       const std::shared_ptr<HttpClient>& http,
//                                                                       const api::EthereumLikeNetworkParameters& parameters,
//                                                                       const std::shared_ptr<api::DynamicObject>& configuration) :
//                DedicatedContext(context), EthereumLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}){
//
//        }

        EthereumLikeBlockchainExplorer::EthereumLikeBlockchainExplorer(const std::shared_ptr<api::ExecutionContext>& context,
                                                                       const std::shared_ptr<HttpClient>& http,
                                                                       const api::EthereumLikeNetworkParameters& parameters,
                                                                       const std::shared_ptr<api::DynamicObject>& configuration) :
                DedicatedContext(context), ConfigurationMatchable({api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}) {
            _http = http;
            _parameters = parameters;
        }

        Future<std::shared_ptr<BigInt>> EthereumLikeBlockchainExplorer::getNonce(const std::string &address) {
            return _http->GET(fmt::format("/blockchain/v2/{}/addresses/{}/nonce?noToken=true",_parameters.Identifier, address))
                    .json().map<std::shared_ptr<BigInt>>(getContext(), [address] (const HttpRequest::JsonResult& result) {
                        auto& json = *std::get<1>(result);

                        if (!json.IsArray() || json.Size() != 1 || !json[0].HasMember("nonce") || !json[0]["nonce"].IsUint64()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to get nonce for {}", address);
                        }

                        auto nonce = json[0]["nonce"].GetUint64();
                        //auto nonce = json[0]["nonce"].GetInt64();
                        return std::make_shared<BigInt>(nonce);
                    });
        }

    }
}