/*
 *
 * BitcoinLikeExplorer
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/03/2017.
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
#pragma once

#include <wallet/NetworkTypes.hpp>

#include <api/BitcoinLikeNetworkParameters.hpp>
#include <collections/collections.hpp>
#include <async/Future.hpp>
#include <net/HttpClient.hpp>
#include <async/DedicatedContext.hpp>
#include <wallet/Explorer.hpp>

namespace ledger {
    namespace core {
		namespace bitcoin {
			class BitcoinLikeExplorer : public ExplorerV2<BitcoinLikeNetwork>, DedicatedContext, public ConfigurationMatchable, public std::enable_shared_from_this<BitcoinLikeExplorer> {
			public:
				BitcoinLikeExplorer(
					const std::shared_ptr<api::ExecutionContext>& context,
					const std::shared_ptr<HttpClient>& http,
					const api::BitcoinLikeNetworkParameters& parameters,
					const std::shared_ptr<api::DynamicObject>& configuration
				);
				Future<void *> startSession() override;
				Future<Unit> killSession(void *session) override;
				Future<Bytes> getRawTransaction(const std::string& transactionHash) override;
				Future<std::string> pushTransaction(const std::vector<uint8_t>& transaction) override;

				FuturePtr<TransactionBulk>
					getTransactions(const std::vector<std::string> &addresses, Option<std::string> fromBlockHash = Option<std::string>(),
						Option<void *> session = Option<void *>()) override;

				FuturePtr<Block> getCurrentBlock() override;

				FuturePtr<Transaction> getTransactionByHash(const std::string &transactionHash) override;

				Future<int64_t > getTimestamp() override;

			private:
				std::shared_ptr<HttpClient> _http;
				api::BitcoinLikeNetworkParameters _parameters;
                
			};
		}
    }
}

