/*
 *
 * BitcoinLikeWritableInputApi.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/04/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#ifndef LEDGER_CORE_BITCOINLIKEWRITABLEINPUTAPI_H
#define LEDGER_CORE_BITCOINLIKEWRITABLEINPUTAPI_H

#include <api/BitcoinLikeInput.hpp>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <wallet/common/api_impl/DerivationPathApi.h>


namespace ledger {
    namespace core {
        class BitcoinLikeWritableInputApi : public api::BitcoinLikeInput {
        public:
            BitcoinLikeWritableInputApi(
                    const std::shared_ptr<ledger::core::BitcoinLikeBlockchainExplorer>& explorer,
                    const std::shared_ptr<api::ExecutionContext>& context,
                    uint32_t sequence,
                    const std::vector<std::vector<uint8_t> >& pubKeys,
                    const std::vector<std::shared_ptr<api::DerivationPath>>& paths,
                    const std::string& address,
                    const std::shared_ptr<api::Amount>& amount,
                    const std::string& previousTxHash,
                    int32_t index,
                    const std::vector<uint8_t>& scriptSig,
                    const std::shared_ptr<api::BitcoinLikeOutput>& previousOutput
            );
            optional<std::string> getAddress() override;
            std::vector<std::vector<uint8_t>> getPublicKeys() override;
            std::shared_ptr<api::Amount> getValue() override;

            std::vector<std::shared_ptr<api::DerivationPath> > getDerivationPath() override;

            bool isCoinbase() override;
            optional<std::string> getCoinbase() override;
            optional<std::string> getPreviousTxHash() override;
            optional<int32_t> getPreviousOutputIndex() override;
            std::shared_ptr<api::BitcoinLikeOutput> getPreviousOuput() override;
            std::vector<uint8_t> getScriptSig() override;
            std::shared_ptr<api::BitcoinLikeScript> parseScriptSig() override;
            void setScriptSig(const std::vector<uint8_t> &scriptSig) override;
            void pushToScriptSig(const std::vector<uint8_t> &data) override;
            void setSequence(int32_t sequence) override;
            int64_t getSequence() override;
            void getPreviousTransaction(const std::shared_ptr<api::BinaryCallback> &callback) override;
            Future<std::vector<uint8_t>> getPreviousTransaction();
            void setP2PKHSigScript(const std::vector<uint8_t> &signature) override;



        private:
            std::shared_ptr<ledger::core::BitcoinLikeBlockchainExplorer> _explorer;
            std::shared_ptr<ledger::core::api::ExecutionContext> _context;
            uint32_t _sequence;
            std::vector<std::vector<uint8_t> > _pubKeys;
            std::vector<std::shared_ptr<api::DerivationPath>> _paths;
            std::string _address;
            std::shared_ptr<api::Amount> _amount;
            std::string _previousHash;
            int32_t  _index;
            std::vector<uint8_t> _scriptSig;
            std::shared_ptr<api::BitcoinLikeOutput> _previousScript;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEWRITABLEINPUTAPI_H
