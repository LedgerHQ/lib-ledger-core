/*
 *
 * BitcoinLikeTransactionApi
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
#ifndef LEDGER_CORE_BITCOINLIKETRANSACTIONAPI_H
#define LEDGER_CORE_BITCOINLIKETRANSACTIONAPI_H

#include <api/BitcoinLikeTransaction.hpp>
#include <wallet/common/api_impl/OperationApi.h>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include "BitcoinLikeInputApi.h"
#include "BitcoinLikeOutputApi.h"
#include <api/BitcoinLikeBlock.hpp>
#include "BitcoinLikeBlockApi.h"

namespace ledger {
    namespace core {
        class BitcoinLikeTransactionApi : public api::BitcoinLikeTransaction {
        public:
            BitcoinLikeTransactionApi(const std::shared_ptr<OperationApi>& operation);
            std::vector<std::shared_ptr<api::BitcoinLikeInput>> getInputs() override;
            std::vector<std::shared_ptr<api::BitcoinLikeOutput>> getOutputs() override;
            std::shared_ptr<api::BitcoinLikeBlock> getBlock() override;
            int64_t getLockTime() override;
            std::shared_ptr<api::Amount> getFees() override;
            std::chrono::system_clock::time_point geTime() override;
            std::string getHash() override;

        private:
            std::vector<std::shared_ptr<api::BitcoinLikeInput>> _inputs;
            std::vector<std::shared_ptr<api::BitcoinLikeOutput>> _outputs;
            int64_t _lockTime;
            std::shared_ptr<api::Amount> _fees;
            std::chrono::system_clock::time_point _time;
            std::shared_ptr<BitcoinLikeBlockApi> _block;
            std::string _hash;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKETRANSACTIONAPI_H
