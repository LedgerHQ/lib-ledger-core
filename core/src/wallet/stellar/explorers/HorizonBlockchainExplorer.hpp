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

#include "StellarLikeBlockchainExplorer.hpp"

#ifndef LEDGER_CORE_HORIZON_EXPLORER_HPP
#define LEDGER_CORE_HORIZON_EXPLORER_HPP

namespace ledger {
    namespace core {
        class HorizonBlockchainExplorer : public StellarLikeBlockchainExplorer, public std::enable_shared_from_this<HorizonBlockchainExplorer> {
        public:
            HorizonBlockchainExplorer(const std::shared_ptr<api::ExecutionContext>& context,
                                      const std::shared_ptr<HttpClient>& http,
                                      const std::shared_ptr<api::DynamicObject>& configuration);
            virtual Future<Option<std::shared_ptr<stellar::Asset>>> getAsset(const std::string& assetCode, const std::string& assetIssuer)  override;
            virtual Future<std::shared_ptr<stellar::Ledger>> getLastLedger()  override;
            virtual FuturePtr<stellar::FeeStats> getRecommendedFees()  override;
            virtual Future<std::vector<std::shared_ptr<stellar::Operation>>> getOperations(const std::string& address, const Option<std::string>& cursor)  override;
            virtual Future<std::vector<std::shared_ptr<stellar::Transaction>>> getTransactions(
                                                                                               const std::string& address,
                                                                                               const Option<std::string>& cursor)  override;

            Future<std::shared_ptr<stellar::Account>> getAccount(const std::string &accountId) const override;

            Future<std::string> postTransaction(const std::vector<uint8_t> &tx) override;
        };
    }
}

#endif // LEDGER_CORE_HORIZON_EXPLORER_HPP
