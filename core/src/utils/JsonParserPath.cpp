/*
 *
 * JsonParserPath.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 26/06/2019.
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

#include "JsonParserPath.hpp"
#include <sstream>

namespace ledger {
    namespace core {

        JsonParserPath::JsonParserPath() {

        }

        void JsonParserPath::startArray() {
            if (_path.back().type == JsonParserPathNodeType::OBJECT) {

            } else {

            }
        }

        void JsonParserPath::startObject() {

        }

        void JsonParserPath::endArray() {
            if (_path.back().type != JsonParserPathNodeType::ARRAY)
                throw std::runtime_error("Can't end a non existing array");
            _path.pop_back();
        }

        void JsonParserPath::endObject() {
            if (_path.back().type != JsonParserPathNodeType::OBJECT)
                throw std::runtime_error("Can't end a non existing object");
            _path.pop_back();
        }

        void JsonParserPath::key(const std::string &key) {
            _lastKey = key;
        }

        std::string JsonParserPath::toString() const {
            std::stringstream ss;
            for (const auto& node : _path) {
                switch (node.type) {
                    case JsonParserPathNodeType::ARRAY:
                        ss << node.key;
                        break;
                    case JsonParserPathNodeType::OBJECT:
                        ss << node.index;
                        break;
                }
            }
            return ss.str();
        }

        bool JsonParserPath::match(const std::string &path) const {
            return false;
        }
    }
}