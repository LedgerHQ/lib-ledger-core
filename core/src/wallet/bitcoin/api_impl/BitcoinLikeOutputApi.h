/*
 *
 * BitcoinLikeOutputApi
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/07/2017.
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
#ifndef LEDGER_CORE_BITCOINLIKEOUTPUTAPI_H
#define LEDGER_CORE_BITCOINLIKEOUTPUTAPI_H

#include <api/BitcoinLikeOutput.hpp>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <wallet/common/api_impl/OperationApi.h>
#include <utils/Either.hpp>
#include <wallet/common/Amount.h>

namespace ledger {
    namespace core {
        class BitcoinLikeOutputApi : public api::BitcoinLikeOutput {
        public:
            BitcoinLikeOutputApi(const std::shared_ptr<OperationApi>& operation, int32_t outputIndex);
            BitcoinLikeOutputApi(const BitcoinLikeBlockchainExplorerOutput& output, const api::Currency& currency);
            BitcoinLikeOutputApi(
                    const BitcoinLikeBlockchainExplorerOutput& output,
                    const api::Currency& currency,
                    const std::shared_ptr<api::DerivationPath>& path
            );
            std::string getTransactionHash() override;
            int32_t getOutputIndex() override;
            std::shared_ptr<api::Amount> getValue() override;
            std::vector<uint8_t> getScript() override;
            optional<std::string> getAddress() override;

            std::shared_ptr<api::BitcoinLikeScript> parseScript() override;

            std::shared_ptr<api::DerivationPath> getDerivationPath() override;

            std::experimental::optional<int64_t> getBlockHeight() override;

            const BigInt& value();

            bool isReplaceable() const override;

        private:
            BitcoinLikeBlockchainExplorerOutput& getOutput();
            const BitcoinLikeBlockchainExplorerOutput& getOutput() const;

        private:
            Either<std::shared_ptr<OperationApi>, BitcoinLikeBlockchainExplorerOutput>  _backend;
            std::shared_ptr<api::DerivationPath> _path;
            int32_t _outputIndex;
            api::Currency _currency;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEOUTPUTAPI_H
