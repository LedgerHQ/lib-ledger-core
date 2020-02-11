/*
 *
 * ledger-core
 *
 * Created by Pierre Pollastri.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#ifndef LEDGER_CORE_HORIZONASSETPARSER_HPP
#define LEDGER_CORE_HORIZONASSETPARSER_HPP

#include <rapidjson/reader.h>
#include <wallet/stellar/stellar.hpp>
#include <utils/JsonParserPath.hpp>
#include "HorizonFlagsParser.hpp"
#include "HorizonArrayParser.hpp"

namespace ledger {
    namespace core {

        class HorizonAssetParser {
        public:
            typedef stellar::Asset Result;
            bool Null();
            bool Bool(bool b);
            bool Int(int i);
            bool Uint(unsigned i);
            bool Int64(int64_t i);
            bool Uint64(uint64_t i);
            bool Double(double d);
            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy);
            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy);
            bool StartObject();
            bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy);
            bool EndObject(rapidjson::SizeType memberCount);
            bool StartArray();
            bool EndArray(rapidjson::SizeType elementCount);
            void init(stellar::Asset *asset);
            void setPathView(const JsonParserPathView& view);

        private:
            JsonParserPathView _path;
            stellar::Asset* _asset;
            HorizonFlagsParser _flagsParser;
        };

        using HorizonAssetsParser = HorizonArrayParser<HorizonAssetParser, stellar::Asset>;

    }
}

#endif //LEDGER_CORE_HORIZONASSETPARSER_HPP