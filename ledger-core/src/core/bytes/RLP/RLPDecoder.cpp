/*
 *
 * RLPDecoder
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

#include <core/bytes/RLP/RLPDecoder.h>
#include <core/bytes/RLP/RLPStringEncoder.h>
#include <core/bytes/RLP/RLPListEncoder.h>

/*
 * Reursive Length Prefix Decoder
 * Reference: https://github.com/ethereum/wiki/wiki/RLP
 */

namespace ledger {
    namespace core {
        std::shared_ptr<RLPEncoder> RLPDecoder::decode(const std::vector<uint8_t> &data,
                                                       std::shared_ptr<RLPEncoder> &parent) {
            std::shared_ptr<RLPEncoder> result;
            if (data.empty()) {
                return result;
            }

            auto tuple = decodeLength(data);
            if (data.size() < std::get<0>(tuple) + std::get<1>(tuple)) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "RLP decoder: Invalid decoded length");
            }
            std::vector<uint8_t> subBytes(data.begin() + std::get<0>(tuple), data.begin() + std::get<0>(tuple) + std::get<1>(tuple));

            auto type = std::get<2>(tuple);

            std::shared_ptr<RLPEncoder> tmpResult;
            switch (type) {
                case RLP_TYPES::bytes: {
                    tmpResult = std::make_shared<RLPStringEncoder>(subBytes);
                    if (parent->isList()) {
                        parent->append(tmpResult);
                        result = parent;
                    } else {
                        result = tmpResult;
                    }
                    break;
                }
                case RLP_TYPES::bytesVector: {
                    auto tmpParent = std::static_pointer_cast<RLPEncoder>(std::make_shared<RLPListEncoder>());
                    auto child = decode(subBytes, tmpParent);
                    if(parent->isList()) {
                        parent->append(tmpParent);
                        result = parent;
                    } else {
                        result = tmpParent;
                    }
                    break;
                }
            }


            if (data.size() > std::get<0>(tuple) + std::get<1>(tuple)) {
                std::vector<uint8_t> tail(data.begin() + std::get<0>(tuple) + std::get<1>(tuple), data.end());
                auto decodeRemaining = decode(tail, parent);
                result = parent;
            }

            return result;
        }

        std::shared_ptr<RLPEncoder> RLPDecoder::decode(const std::vector<uint8_t> &data) {
            auto parent = std::static_pointer_cast<RLPEncoder>(std::make_shared<RLPStringEncoder>(""));
            return decode(data, parent);
        }

        rlp_tuple RLPDecoder::decodeLength(const std::vector<uint8_t> &data) {
            auto length = data.size();
            if (length == 0) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "RLP decoder: Input is null");
            }
            auto prefix = data[0];
            if (prefix <= 0x7F) {
                return std::make_tuple(0, 1, RLP_TYPES::bytes);
            } else if (prefix <= 0xB7 && length > prefix - 0x80) {
                auto strLength = prefix - 0x80;
                return std::make_tuple(1, strLength, RLP_TYPES::bytes);
            } else if (prefix <= 0xBF && length > prefix - 0xB7) {
                auto lenOfstrLength = prefix - 0xB7;
                std::vector<uint8_t> subBytes(data.begin() + 1, data.begin() + 1 + lenOfstrLength);
                if (length > prefix - 0xB7 + toInteger(subBytes)) {
                    return std::make_tuple(1 + lenOfstrLength, toInteger(subBytes), RLP_TYPES::bytes);
                }
            } else if (prefix <= 0xF7 && length > prefix - 0xC0) {
                auto vectorLength = prefix - 0xC0;
                return std::make_tuple(1, vectorLength, RLP_TYPES::bytesVector);
            } else if (prefix <= 0xFF && length > prefix - 0xF7) {
                auto lenOfVecLength = prefix - 0xF7;
                std::vector<uint8_t> subBytes(data.begin() + 1, data.begin() + 1 + lenOfVecLength);
                if (length > prefix - 0xF7 + toInteger(subBytes)) {
                    return std::make_tuple(1 + lenOfVecLength, toInteger(subBytes), RLP_TYPES::bytesVector);
                }
            }
            throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "RLP decoder: Input don't conform RLP encoding form");
        }



        uint32_t RLPDecoder::toInteger(std::vector<uint8_t> &data) {
            auto length = data.size();
            if (length == 0) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "RLP decoder: Input is null");
            } else if (length == 1) {
                return data[0];
            } else {
                std::vector<uint8_t> subBytes(data.begin(), data.end() - 1);
                auto last = data[length - 1];
                return (last + toInteger(subBytes))*256;
            }
        }
    }
}
