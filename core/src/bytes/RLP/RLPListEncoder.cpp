/*
 *
 * RLPListEncoder
 *
 * Created by El Khalil Bellakrid on 24/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#include "RLPListEncoder.h"
#include "RLPStringEncoder.h"

namespace ledger {
    namespace core {

        RLPListEncoder::RLPListEncoder(const std::shared_ptr<RLPListEncoder> &child) {
            if(child) {
                _children.push_back(child);
            }
        }

        RLPListEncoder::RLPListEncoder(const std::string &childString) {
            auto child = std::make_shared<RLPStringEncoder>(childString);
            _children.push_back(child);
        }

        RLPListEncoder::RLPListEncoder(const std::vector<uint8_t> &childBytes) {
            auto child = std::make_shared<RLPStringEncoder>(childBytes);
            _children.push_back(child);
        }
        std::vector<uint8_t> RLPListEncoder::encode() {

            BytesWriter result, childWriter;
            for (auto &child : _children) {
                auto encodedChild = child->encode();
                childWriter.writeByteArray(std::move(encodedChild));
            }
            auto encodedChildren = childWriter.toByteArray();
            RLPEncoder::encodeLength(encodedChildren.size(), 0xc0, result);
            result.writeByteArray(encodedChildren);
            return result.toByteArray();
        }

        void RLPListEncoder::append(const std::shared_ptr<RLPEncoder> &child) {
            if (child) {
                _children.push_back(child);
            }
        }

        void RLPListEncoder::append(const std::string &str) {
            _children.push_back(std::make_shared<RLPStringEncoder>(str));
        }

        bool RLPListEncoder::isList() {
            return true;
        }

        std::string RLPListEncoder::toString() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "RLP list encoder: Impossible to get string from list encoder");
        }

        std::vector<std::shared_ptr<RLPEncoder>> RLPListEncoder::getChildren() {
            return _children;
        }
    }
}