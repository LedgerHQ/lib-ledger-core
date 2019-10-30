/*
 *
 * TezosLikeBlockchainExplorer
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


#include "TezosLikeBlockchainExplorer.h"
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
namespace ledger {
    namespace core {

        const std::string rpcNode = "https://mainnet.tezrpc.me/";

        TezosLikeBlockchainExplorer::TezosLikeBlockchainExplorer(
                const std::shared_ptr<ledger::core::api::DynamicObject> &configuration,
                const std::vector<std::string> &matchableKeys) : ConfigurationMatchable(matchableKeys) {
            setConfiguration(configuration);
        }

        Future<std::vector<uint8_t>> TezosLikeBlockchainExplorer::forgeKTOperation(const std::shared_ptr<TezosLikeTransactionApi> &tx,
                                                                                   const std::shared_ptr<api::ExecutionContext> &context,
                                                                                   const std::shared_ptr<HttpClient> &http) {
            std::string params;
            switch (tx->getType()) {
                case api::TezosOperationTag::OPERATION_TAG_TRANSACTION:
                    params = fmt::format("\"parameters\":"
                                                 "{{\"entrypoint\":\"do\",\"value\":["
                                                 "{{\"prim\":\"DROP\"}},"
                                                 "{{\"prim\":\"NIL\",\"args\":[{{\"prim\":\"operation\"}}]}},"
                                                 "{{\"prim\":\"PUSH\",\"args\":[{{\"prim\":\"key_hash\"}},{{\"string\":\"{}\"}}]}},"
                                                 "{{\"prim\":\"IMPLICIT_ACCOUNT\"}},"
                                                 "{{\"prim\":\"PUSH\",\"args\":[{{\"prim\":\"mutez\"}},{{\"int\":\"{}\"}}]}},"
                                                 "{{\"prim\":\"UNIT\"}},"
                                                 "{{\"prim\":\"TRANSFER_TOKENS\"}},"
                                                 "{{\"prim\":\"CONS\"}}]}}",
                                         tx->getReceiver()->toBase58(),
                                         tx->getValue()->toString());
                    break;
                case api::TezosOperationTag::OPERATION_TAG_DELEGATION:
                    params = "\"parameters\":"
                            "{{\"entrypoint\":\"do\",\"value\":["
                            "{{\"prim\":\"DROP\"}},"
                            "{{\"prim\":\"NIL\",\"args\":[{{\"prim\":\"operation\"}}]}},"
                            "{{\"prim\":\"NONE\",\"args\":[{{\"prim\":\"key_hash\"}}]}},"
                            "{{\"prim\":\"SET_DELEGATE\"}},"
                            "{{\"prim\":\"CONS\"}}]}}";
                    break;
                default:
                    throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Only supported operations from originated accounts: Transaction & (Remove) of delegation.");
            }

            // Increment counter if needs revelation
            // Arf BigInts ...
            auto counter = tx->toReveal() ? (BigInt(tx->getCounter()->toString(10)) + BigInt(1)).toString() : tx->getCounter()->toString(10);
            auto bodyString = fmt::format("{{\"branch\":\"{}\","
                                                  "\"contents\":[{{\"kind\":\"transaction\","
                                                  "\"fee\":\"{}\",\"gas_limit\":\"{}\",\"storage_limit\":\"{}\","
                                                  "\"amount\":\"{}\",\"destination\":\"{}\",{},"
                                                  "\"source\":\"{}\",\"counter\":\"{}\"}}]}}",
                                          tx->getBlockHash().value_or(""),
                                          tx->getFees()->toString(),
                                          "27000",
                                          tx->getStorageLimit()->toString(10),
                                          0,
                                          tx->getSender()->toBase58(),
                                          params,
                                          tx->getManagerAddress(),
                                          counter
            );
            const bool parseNumbersAsString = true;
            std::unordered_map<std::string, std::string> headers{{"Content-Type", "application/json"}};
            return http->POST("/chains/main/blocks/head/helpers/forge/operations",
                              std::vector<uint8_t>(bodyString.begin(), bodyString.end()),
                              headers,
                              rpcNode)
                    .json(parseNumbersAsString)
                    .map<std::vector<uint8_t>>(context, [](const HttpRequest::JsonResult &result) {
                        auto &json = *std::get<1>(result);
                        if (!json.IsString()) {
                            throw make_exception(api::ErrorCode::HTTP_ERROR, "Failed to forge operation.");
                        }
                        auto info = json.GetString();
                        return hex::toByteArray(info);
                    });
        }
    }
}