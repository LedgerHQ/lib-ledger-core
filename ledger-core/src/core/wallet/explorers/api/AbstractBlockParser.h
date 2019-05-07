/*
 *
 * AbstractBlockParser
 *
 * Created by El Khalil Bellakrid on 16/11/2018.
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


#ifndef LEDGER_CORE_ABSTRACTBLOCKPARSER_H
#define LEDGER_CORE_ABSTRACTBLOCKPARSER_H

#include <rapidjson/reader.h>

#include <collections/collections.hpp>
#include <math/BigInt.h>
#include <net/HttpClient.hpp>
#include <utils/DateUtils.hpp>

namespace ledger {
    namespace core {
        template <typename BlockchainExplorerTransactionBlock>
        class AbstractBlockParser {
        public:

            bool Null() {
                return true;
            }

            bool Bool(bool b) {
                return true;
            }

            bool Int(int i) {
                return true;
            }

            bool Uint(unsigned i) {
                return true;
            }

            bool Int64(int64_t i) {
                return true;
            }

            bool Uint64(uint64_t i) {
                return true;
            }

            bool Double(double d) {
                return true;
            }

            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                if (getLastKey() == "height") {
                    std::string number(str, length);
                    BigInt value = BigInt::fromString(number);
                    _block->height = value.toUint64();
                }
                return true;
            }

            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                std::string value = std::string(str, length);
                if (getLastKey() == "hash") {
                    _block->hash = value;
                } else if (getLastKey() == "time") {
                    _block->time = DateUtils::fromJSON(value);
                }
                return true;
            }

            bool StartObject() {
                return true;
            }

            bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                return true;
            }

            bool EndObject(rapidjson::SizeType memberCount) {
                return true;
            }

            bool StartArray() {
                return true;
            }

            bool EndArray(rapidjson::SizeType elementCount) {
                return true;
            }

            void init(BlockchainExplorerTransactionBlock *block) {
                _block = block;
            }

        protected:
            virtual std::string &getLastKey() = 0;
            BlockchainExplorerTransactionBlock* _block;
        };
    }
}


#endif //LEDGER_CORE_ABSTRACTBLOCKPARSER_H
