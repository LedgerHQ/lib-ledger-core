/*
 *
 * BitcoinLikeBlockchainExplorer
 * ledger-core
 *
 * Created by Pierre Pollastri on 17/01/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "BitcoinLikeBlockchainExplorer.hpp"

namespace ledger {
    namespace core {
        BitcoinLikeBlockchainExplorer::BitcoinLikeBlockchainExplorer(const std::shared_ptr<ledger::core::api::DynamicObject> &configuration,
                                                                     const std::vector<std::string> &matchableKeys) : ConfigurationMatchable(matchableKeys) {
            setConfiguration(configuration);
        }

        Future<String> BitcoinLikeBlockchainExplorer::pushTransactionToNode(const std::vector<uint8_t> &rawTx,
                                                                            const std::shared_ptr<api::ExecutionContext> &context,
                                                                            const std::shared_ptr<HttpClient> &client,
                                                                            const std::string &rpcNode) {
            std::stringstream body;
            body << '{"jsonrpc": "2.0", "id": "broadcastTx", "method": "sendrawtransaction", "params": ["' << hex::toString(rawTx) <<'"] }';
            auto bodyString = body.str();
            return client->POST("",
                               std::vector<uint8_t>(bodyString.begin(), bodyString.end()),
                                                    std::unordered_map<std::string, std::string>{},
                                                    rpcNode)
                    .json().template map<String>(context, [] (const HttpRequest::JsonResult& result) -> String {
                        auto& json = *std::get<1>(result);
                        return json["result"].GetString();
                    });
        }
    }
}
