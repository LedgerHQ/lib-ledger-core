/*
 *
 * HorizonAssetsParser.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/06/2019.
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

#include "HorizonAssetsParser.hpp"

namespace ledger {
    namespace core {

        bool HorizonAssetsParser::Null() {
            return true;
        }

        bool HorizonAssetsParser::Bool(bool b) {
            return true;
        }

        bool HorizonAssetsParser::Int(int i) {
            return true;
        }

        bool HorizonAssetsParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonAssetsParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonAssetsParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonAssetsParser::Double(double d) {
            return true;
        }

        bool HorizonAssetsParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonAssetsParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonAssetsParser::StartObject() {
            return true;
        }

        bool HorizonAssetsParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonAssetsParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonAssetsParser::StartArray() {
            return true;
        }

        bool HorizonAssetsParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonAssetsParser::init(std::vector<stellar::Asset> *assets) {
            _assets = assets;
        }
    }
}