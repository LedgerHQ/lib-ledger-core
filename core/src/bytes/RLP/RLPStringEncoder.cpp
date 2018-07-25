/*
 *
 * RLPStringEncoder
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


#include "RLPStringEncoder.h"
#include "../../utils/Exception.hpp"

namespace ledger {
    namespace core {
        //TODO : other constructors (operators)
        RLPStringEncoder::RLPStringEncoder(const std::string &data) : _data(data.begin(),data.end()) {
        }

        RLPStringEncoder::RLPStringEncoder(const std::vector<uint8_t> &data) : _data(data){

        }

        std::vector<uint8_t> RLPStringEncoder::encode() {
            BytesWriter result;
            if (_data.size() == 1 && (_data[0] & 0xFF) < 0x80) {
                result.writeByte(_data[0]);
            } else {
                encodeLength(_data.size(), 0x80, result);
                result.writeByteArray(_data);
            }
            return result.toByteArray();
        }

        void RLPStringEncoder::append(const std::string &str) {
            std::vector<uint8_t> strVector(str.begin(), str.end());
            _data.insert(_data.end(), strVector.begin(), strVector.end());
        }

        void RLPStringEncoder::append(const std::shared_ptr<RLPEncoder> &child) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "RLP string encoder: Impossible to append child");
        }

        bool RLPStringEncoder::isList() {
            return false;
        }

        std::string RLPStringEncoder::toString() {
            return std::string(_data.begin(), _data.end());
        }

        std::vector<std::shared_ptr<RLPEncoder>> RLPStringEncoder::getChildren() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "RLP string encoder: string encoder has no children");
        }
    }
}