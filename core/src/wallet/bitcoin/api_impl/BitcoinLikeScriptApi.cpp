/*
 *
 * BitcoinLikeScriptApi.cpp
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

#include "BitcoinLikeScriptApi.h"

namespace ledger {
    namespace core {

        static inline const ledger::core::BitcoinLikeScriptChunk& chunkOf(const std::shared_ptr<BitcoinLikeScriptApi> &script,
                              int index) {
            auto it = script->getScript().toList().begin();
            while (index > 0) {
                index--;
                it++;
            }
            return *it;
        }

        std::shared_ptr<api::BitcoinLikeScriptChunk> BitcoinLikeScriptApi::head() {
            return std::make_shared<BitcoinLikeScriptChunkApi>(shared_from_this(), 0);
        }

        std::string BitcoinLikeScriptApi::toString() {
            return _script.toString();
        }

        BitcoinLikeScriptApi::BitcoinLikeScriptApi(const ledger::core::BitcoinLikeScript &script) : _script(script) {

        }

        const ledger::core::BitcoinLikeScript &BitcoinLikeScriptApi::getScript() const {
            return _script;
        }

        bool BitcoinLikeScriptChunkApi::isOperator() {
            return getChunk().isOpCode();
        }

        bool BitcoinLikeScriptChunkApi::isPushedData() {
            return getChunk().isBytes();
        }

        BitcoinLikeScriptChunkApi::BitcoinLikeScriptChunkApi(const std::shared_ptr<BitcoinLikeScriptApi> &script,
                                                             int index) : _chunk(chunkOf(script, index)) {
            _script = script;
            _index = index;
        }

        std::ledger_exp::optional<api::BitcoinLikeOperator> BitcoinLikeScriptChunkApi::getOperator() {
            auto &chunk = getChunk();
            if (chunk.isBytes()) {
                api::BitcoinLikeOperator op(btccore::GetOpName(chunk.getOpCode()), (uint8_t) chunk.getOpCode());
                return Option<api::BitcoinLikeOperator>(op).toOptional();
            }
            return Option<api::BitcoinLikeOperator>().toOptional();
        }

        std::ledger_exp::optional<std::vector<uint8_t>> BitcoinLikeScriptChunkApi::getPushedData() {
            auto &chunk = getChunk();
            if (chunk.isBytes()) return Option<std::vector<uint8_t> >(chunk.getBytes()).toOptional();
            return Option<std::vector<uint8_t> >().toOptional();
        }

        std::shared_ptr<api::BitcoinLikeScriptChunk> BitcoinLikeScriptChunkApi::next() {
            return std::make_shared<BitcoinLikeScriptChunkApi>(_script, _index + 1);
        }

        bool BitcoinLikeScriptChunkApi::hasNext() {
            return _index + 1 < _script->getScript().toList().size();
        }

        const ledger::core::BitcoinLikeScriptChunk &BitcoinLikeScriptChunkApi::getChunk() const {
            return _chunk;
        }

        std::shared_ptr<api::BitcoinLikeScript> api::BitcoinLikeScript::parse(const std::vector<uint8_t> &data) {
            auto result = ledger::core::BitcoinLikeScript::parse(data);
            if (result.isFailure())
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, result.getFailure().getMessage());
            return std::make_shared<BitcoinLikeScriptApi>(result.getValue());
        }

    }
}