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
#include <utils/SingleObjectJsonSaxParser.hpp>
#include "horizon/HorizonAssetParser.hpp"
#include "horizon/HorizonApiParser.hpp"

namespace ledger {
    namespace core {
        using AssetVectorParser = SingleObjectJsonSaxParser<HorizonAssetParser, std::vector<HorizonAssetParser::Result>>;

        HorizonBlockchainExplorer::HorizonBlockchainExplorer(const std::shared_ptr<api::ExecutionContext>& context,
                                                             const std::shared_ptr<HttpClient>& http,
                                                             const std::shared_ptr<api::DynamicObject>& configuration)
            : StellarLikeBlockchainExplorer(context, http) {

        }

        Future<Option<std::shared_ptr<stellar::Asset>>> HorizonBlockchainExplorer::getAsset(const std::string& assetCode) {
            http->GET(fmt::format("/assets?asset_code={}", assetCode))
                .template json<std::vector<stellar::Asset>, Exception>(HorizonApiParser<AssetVectorParser::Result, AssetVectorParser>());
        }

        Future<Option<std::shared_ptr<stellar::Ledger>>> HorizonBlockchainExplorer::getLastLedger() {

        }

        FuturePtr<BigInt> HorizonBlockchainExplorer::getRecommendedFees() {

        }

        Future<std::vector<std::shared_ptr<stellar::Operation>>> HorizonBlockchainExplorer::getOperations(const std::string& address,
                                                              const Option<std::string>& cursor) {

        }

        Future<std::vector<std::shared_ptr<stellar::Transaction>>> HorizonBlockchainExplorer::getTransactions(const std::string& address,
                                                                  const Option<std::string>& cursor) {

        }

    }
}
