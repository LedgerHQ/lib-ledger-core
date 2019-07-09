/*
 *
 * ledger-core
 *
 * Created by Pierre Pollastri.
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

#include "HorizonBlockchainExplorer.hpp"
#include <utils/Exception.hpp>
#include <utils/JSONUtils.h>
#include <wallet/stellar/explorers/horizon/HorizonApiParser.hpp>
#include "horizon/HorizonApiParser.hpp"
#include "horizon/HorizonAccountParser.hpp"
#include "horizon/HorizonLedgerParser.hpp"
#include "horizon/HorizonAssetParser.hpp"
#include "horizon/HorizonTransactionParser.hpp"

namespace ledger {
    namespace core {
        using AssetsParser = HorizonApiParser<std::vector<std::shared_ptr<stellar::Asset>>, HorizonAssetsParser>;
        using AccountParser = HorizonApiParser<stellar::Account, HorizonAccountParser, false>;
        using LedgersParser = HorizonApiParser<std::vector<std::shared_ptr<stellar::Ledger>>, HorizonLedgersParser>;
        using TransactionsParser = HorizonApiParser<std::vector<std::shared_ptr<stellar::Transaction>>, HorizonTransactionsParser>;

        HorizonBlockchainExplorer::HorizonBlockchainExplorer(const std::shared_ptr<api::ExecutionContext>& context,
                                                             const std::shared_ptr<HttpClient>& http,
                                                             const std::shared_ptr<api::DynamicObject>& configuration)
            : StellarLikeBlockchainExplorer(context, http) {

        }

        Future<Option<std::shared_ptr<stellar::Asset>>> HorizonBlockchainExplorer::getAsset(const std::string& assetCode, const std::string& assetIssuer) {
          return http->GET(fmt::format("/assets?asset_code={}&asset_issuer={}", assetCode, assetIssuer))
          .template json<AssetsParser::Result, Exception>(AssetsParser())
                  .map<Option<std::shared_ptr<stellar::Asset>>>(getContext(), [] (const Either<Exception, std::shared_ptr<AssetsParser::Result>>& assets) -> Option<std::shared_ptr<stellar::Asset>> {
                    if (assets.isLeft()) {
                        throw assets.getLeft();
                    }
                      if (assets.getRight()->empty()) {
                        return Option<std::shared_ptr<stellar::Asset>>();
                    } else {
                        return Option<std::shared_ptr<stellar::Asset>>(assets.getRight()->front());
                    }
                  });
        }

        Future<std::shared_ptr<stellar::Ledger>> HorizonBlockchainExplorer::getLastLedger() {
            return http->GET("/ledgers?order=desc&limit=1")
                    .template json<LedgersParser::Result, Exception>(LedgersParser())
                    .map<std::shared_ptr<stellar::Ledger>>(getContext(), [] (const LedgersParser::Response& result) -> std::shared_ptr<stellar::Ledger> {
                        if (result.isLeft()) {
                            throw result.getLeft();
                        }
                        if (result.getRight()->empty())
                            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Unable to fetch last ledger (empty response)");
                        return result.getRight()->front();
                    });
        }

        FuturePtr<BigInt> HorizonBlockchainExplorer::getRecommendedFees() {

        }

        Future<std::vector<std::shared_ptr<stellar::Operation>>> HorizonBlockchainExplorer::getOperations(const std::string& address,
                                                              const Option<std::string>& cursor) {

        }

        Future<std::vector<std::shared_ptr<stellar::Transaction>>> HorizonBlockchainExplorer::getTransactions(const std::string& address,
                                                                  const Option<std::string>& cursor) {

            auto cursorParam = cursor.isEmpty() ? "" : fmt::format("&cursor={}", cursor.getValue());
            return http->GET(fmt::format("/accounts/{}/transactions?limit=50&order=asc&include_failed{}", address, cursorParam))
                    .template json<TransactionsParser ::Result, Exception>(TransactionsParser())
                    .map<std::vector<std::shared_ptr<stellar::Transaction>>>(getContext(), [] (const TransactionsParser::Response& response) -> std::vector<std::shared_ptr<stellar::Transaction>> {
                        if (response.isLeft()) {
                            throw response.getLeft();
                        }
                        return *response.getRight();
                    });
        }

        Future<std::shared_ptr<stellar::Account>>
        HorizonBlockchainExplorer::getAccount(const std::string &accountId) const {
            return http->GET(fmt::format("/accounts/{}", accountId))
                    .template json<AccountParser::Result, Exception>(AccountParser())
                    .map<std::shared_ptr<stellar::Account>>(getContext(), [] (const AccountParser::Response& result) -> std::shared_ptr<stellar::Account> {
                        if (result.isLeft()) {
                            throw result.getLeft();
                        }
                       return result.getRight();
                    });
        }

    }
}
