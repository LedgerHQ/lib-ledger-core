/*
 *
 * JSONUtils
 * ledger-core
 *
 * Created by Pierre Pollastri on 31/05/2017.
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
#ifndef LEDGER_CORE_JSONUTILS_H
#define LEDGER_CORE_JSONUTILS_H

#include <rapidjson/reader.h>
#include <string>
#include <wallet/common/explorers/LedgerApiParser.hpp>

namespace ledger {
    namespace core {

        class JSONUtils {
          public:
            template <typename Parser>
            static std::shared_ptr<typename Parser::Result> parse(const std::string &json) {
                LedgerApiParser<typename Parser::Result, Parser> parser;
                parser.attach("", 200);
                rapidjson::Reader reader;
                rapidjson::StringStream is(json.data());
                reader.Parse<rapidjson::ParseFlag::kParseNumbersAsStringsFlag>(is, parser);
                auto result = parser.build();
                if (result.isLeft()) {
                    throw result.getLeft();
                }
                return result.getRight();
            };
        };

    } // namespace core
} // namespace ledger

#endif // LEDGER_CORE_JSONUTILS_H
