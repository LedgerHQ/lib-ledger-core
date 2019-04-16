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

#include "HorizonAssetParser.hpp"

namespace ledger {
    namespace core {

        bool HorizonAssetParser::Null() {
            return true;
        }

        bool HorizonAssetParser::Bool(bool b) {

        }

        bool HorizonAssetParser::Int(int i) {

        }

        bool HorizonAssetParser::Uint(unsigned i) {

        }

        bool HorizonAssetParser::Int64(int64_t i) {

        }

        bool HorizonAssetParser::Uint64(uint64_t i) {

        }

        bool HorizonAssetParser::Double(double d) {

        }

        bool HorizonAssetParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {

        }

        bool HorizonAssetParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {

        }

        bool HorizonAssetParser::StartObject() {

        }

        bool HorizonAssetParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {

        }

        bool HorizonAssetParser::EndObject(rapidjson::SizeType memberCount) {

        }

        bool HorizonAssetParser::StartArray() {

        }

        bool HorizonAssetParser::EndArray(rapidjson::SizeType elementCount) {

        }

        void HorizonAssetParser::init(stellar::Asset *block) {

        }

    }
}
