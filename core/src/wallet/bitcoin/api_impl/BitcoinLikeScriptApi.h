/*
 *
 * BitcoinLikeScriptApi.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 09/04/2018.
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

#ifndef LEDGER_CORE_BITCOINLIKESCRIPTAPI_H
#define LEDGER_CORE_BITCOINLIKESCRIPTAPI_H

#include <api/BitcoinLikeScript.hpp>
#include <api/BitcoinLikeScriptChunk.hpp>
#include <api/BitcoinLikeOperator.hpp>
#include <wallet/bitcoin/scripts/BitcoinLikeScript.h>

namespace ledger {
    namespace core {

        class BitcoinLikeScriptApi;
        class BitcoinLikeScriptChunkApi : public api::BitcoinLikeScriptChunk {
        public:
            BitcoinLikeScriptChunkApi(const std::shared_ptr<BitcoinLikeScriptApi>& script, int index);
            bool isOperator() override;

            bool isPushedData() override;

            std::ledger_exp::optional<api::BitcoinLikeOperator> getOperator() override;

            std::ledger_exp::optional<std::vector<uint8_t>> getPushedData() override;

            std::shared_ptr<api::BitcoinLikeScriptChunk> next() override;

            bool hasNext() override;

            inline const ledger::core::BitcoinLikeScriptChunk& getChunk() const;

        private:
            int _index;
            std::shared_ptr<BitcoinLikeScriptApi> _script;
            const ledger::core::BitcoinLikeScriptChunk& _chunk;
        };

        class BitcoinLikeScriptApi : public api::BitcoinLikeScript, public std::enable_shared_from_this<BitcoinLikeScriptApi> {
        public:
            explicit BitcoinLikeScriptApi(const ledger::core::BitcoinLikeScript& script);
            std::shared_ptr<api::BitcoinLikeScriptChunk> head() override;
            std::string toString() override;
            const ledger::core::BitcoinLikeScript& getScript() const;

        private:
            ledger::core::BitcoinLikeScript _script;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKESCRIPTAPI_H
