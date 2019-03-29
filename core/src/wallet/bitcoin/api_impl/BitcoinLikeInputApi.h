/*
 *
 * BitcoinLikeInputApi
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
#ifndef LEDGER_CORE_BITCOINLIKEINPUTAPI_H
#define LEDGER_CORE_BITCOINLIKEINPUTAPI_H

#include <api/BitcoinLikeInput.hpp>
#include <wallet/common/api_impl/OperationApi.h>
#include "../explorers/BitcoinLikeBlockchainExplorer.hpp"


namespace ledger {
    namespace core {
        class BitcoinLikeInputApi : public api::BitcoinLikeInput {
        public:
            BitcoinLikeInputApi(const std::shared_ptr<OperationApi>& operation, int32_t inputIndex);
            std::ledger_exp::optional<std::string> getAddress() override;
            std::shared_ptr<api::Amount> getValue() override;
            bool isCoinbase() override;
            std::ledger_exp::optional<std::string> getCoinbase() override;
            std::ledger_exp::optional<std::string> getPreviousTxHash() override;
            std::ledger_exp::optional<int32_t> getPreviousOutputIndex() override;

            std::vector<std::vector<uint8_t>> getPublicKeys() override;

            std::vector<std::shared_ptr<api::DerivationPath>> getDerivationPath() override;

            std::shared_ptr<api::BitcoinLikeOutput> getPreviousOuput() override;

            std::vector<uint8_t> getScriptSig() override;

            std::shared_ptr<api::BitcoinLikeScript> parseScriptSig() override;

            void setScriptSig(const std::vector<uint8_t> &scriptSig) override;

            void pushToScriptSig(const std::vector<uint8_t> &data) override;

            void setSequence(int32_t sequence) override;

            int64_t getSequence() override;

            void getPreviousTransaction(const std::shared_ptr<api::BinaryCallback> &callback) override;

            void setP2PKHSigScript(const std::vector<uint8_t> &signature) override;

        private:
            inline BitcoinLikeBlockchainExplorerInput& getInput();

        private:
            std::shared_ptr<OperationApi> _operation;
            int32_t _inputIndex;
        };
    }
}

#endif //LEDGER_CORE_BITCOINLIKEINPUTAPI_H
