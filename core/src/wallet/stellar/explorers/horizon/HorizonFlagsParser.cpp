/*
 *
 * HorizonFlagsParser.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/07/2019.
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

#include "HorizonFlagsParser.hpp"

using namespace ledger::core;

static const JsonParserPathMatcher AUTH_REQUIRED_MATCHER("/auth_required");
static const JsonParserPathMatcher AUTH_REVOCABLE_MATCHER("/auth_revocable");
static const JsonParserPathMatcher AUTH_IMMUTABLE_MATCHER("/auth_immutable");

namespace ledger {
    namespace core {

        bool HorizonFlagsParser::Bool(bool b) {
            if (_path.match(AUTH_REQUIRED_MATCHER)) {
                _flags->authRequired = b;
            } else if (_path.match(AUTH_REVOCABLE_MATCHER)) {
                _flags->authRevocable = b;
            } else if (_path.match(AUTH_IMMUTABLE_MATCHER)) {
                _flags->authImmutable = b;
            }
            return true;
        }

        bool HorizonFlagsParser::Int(int i) {
            return true;
        }

        bool HorizonFlagsParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonFlagsParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonFlagsParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonFlagsParser::Double(double d) {
            return true;
        }

        bool HorizonFlagsParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonFlagsParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonFlagsParser::StartObject() {
            return true;
        }

        bool HorizonFlagsParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonFlagsParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonFlagsParser::StartArray() {
            return true;
        }

        bool HorizonFlagsParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonFlagsParser::init(stellar::Flags *flags) {
            _flags = flags;
        }

        void HorizonFlagsParser::setPathView(const JsonParserPathView &path) {
            _path = path;
        }
    }
}
