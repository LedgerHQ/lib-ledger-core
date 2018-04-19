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
#include <api/EstimatedSize.hpp>
#include <wallet/bitcoin/api_impl/BitcoinLikeWritableInputApi.h>

namespace ledger {
    namespace core {
        class BitcoinLikeTransactionApi : public api::BitcoinLikeTransaction {
        public:
            explicit BitcoinLikeTransactionApi(const api::Currency& currency);
            explicit BitcoinLikeTransactionApi(const std::shared_ptr<OperationApi>& operation);
            std::vector<std::shared_ptr<api::BitcoinLikeInput>> getInputs() override;
            std::vector<std::shared_ptr<api::BitcoinLikeOutput>> getOutputs() override;
            std::shared_ptr<api::BitcoinLikeBlock> getBlock() override;
            int64_t getLockTime() override;
            std::shared_ptr<api::Amount> getFees() override;
            std::string getHash() override;
            std::chrono::system_clock::time_point getTime() override;
            optional<int32_t> getTimestamp() override;
            std::vector<uint8_t> serialize() override;
            optional<std::vector<uint8_t>> getWitness() override;
            api::EstimatedSize getEstimatedSize() override;

            BitcoinLikeTransactionApi& addInput(const std::shared_ptr<BitcoinLikeWritableInputApi>& input);
            BitcoinLikeTransactionApi& addOutput(const std::shared_ptr<api::BitcoinLikeOutput>& output);
            BitcoinLikeTransactionApi& setLockTime(uint32_t lockTime);

        public:
            static api::EstimatedSize estimateSize(std::size_t inputCount,
                                            std::size_t outputCount,
                                            bool hasTimestamp,
                                            bool useSegwit
            );

        private:
            inline bool isWriteable() const;
            inline bool isReadOnly() const;

        private:
            int32_t _version;
            std::vector<std::shared_ptr<api::BitcoinLikeInput>> _inputs;
            std::vector<std::shared_ptr<api::BitcoinLikeOutput>> _outputs;
            int32_t _lockTime;
            std::shared_ptr<api::Amount> _fees;
            std::chrono::system_clock::time_point _time;
            std::shared_ptr<BitcoinLikeBlockApi> _block;
            std::string _hash;
            api::Currency _currency;
            Option<int32_t> _timestamp;
            bool _writable;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKETRANSACTIONAPI_H
