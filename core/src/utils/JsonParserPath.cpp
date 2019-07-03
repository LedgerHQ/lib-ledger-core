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
#include <utils/Exception.hpp>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>

namespace ledger {
    namespace core {

        JsonParserPath::JsonParserPath() {

        }

        void JsonParserPath::startArray() {
            value();
            _path.emplace_back(JsonParserPathNode(JsonParserPathNodeType::ARRAY));
        }

        void JsonParserPath::startObject() {
            value();
            _path.emplace_back(JsonParserPathNode(JsonParserPathNodeType::OBJECT));
        }

        void JsonParserPath::endArray() {
            if (getParent().type != JsonParserPathNodeType::ARRAY && getCurrent().type != JsonParserPathNodeType::ARRAY)
                throw std::runtime_error("Can't end a non existing array");
            if (getCurrent().type == JsonParserPathNodeType::VALUE)
                _path.pop_back();
            _path.pop_back();
        }

        void JsonParserPath::endObject() {
            if (getParent().type != JsonParserPathNodeType::OBJECT && getCurrent().type != JsonParserPathNodeType::OBJECT)
                throw std::runtime_error("Can't end a non existing array");
            if (getCurrent().type == JsonParserPathNodeType::VALUE)
                _path.pop_back();
            _path.pop_back();
        }

        void JsonParserPath::key(const std::string &key) {
            _lastKey = key;
        }

        std::string JsonParserPath::toString() const {
            std::stringstream ss;
            const JsonParserPathNode *parent = nullptr;
            for (const auto& node : _path) {
                switch (node.type) {
                    case JsonParserPathNodeType::OBJECT:
                        ss << "/";
                        break;
                    case JsonParserPathNodeType::ARRAY:
                        ss << "[";
                        break;
                    case JsonParserPathNodeType::VALUE:
                        if (parent != nullptr && parent->type == JsonParserPathNodeType::OBJECT) {
                            ss << node.key();
                        } else if (parent != nullptr && parent->type == JsonParserPathNodeType::ARRAY) {
                            ss << node.index() << "]";
                        }
                        break;
                }
                parent = &node;
            }
            return ss.str();
        }

        bool JsonParserPath::match(const JsonParserPathMatcher& matcher, int depth) const {
            if ((_path.size() - depth) < matcher.getElements().size())
                return false;
            auto node = _path.begin();
            std::advance(node, depth);
            const JsonParserPathNode *parent = nullptr;
            auto element = matcher.getElements().begin();
            while (node != _path.end()) {
                if (element->filter == JsonParserPathMatcherFilter::MATCH_ALL)
                    return true;
                if (node->type != element->node.type)
                    return false;
                if (node->type == JsonParserPathNodeType::VALUE &&
                    element->filter == JsonParserPathMatcherFilter::EXACT) {
                    if (!parent)
                        return false;
                    if (parent->type == JsonParserPathNodeType::ARRAY &&
                        node->index() != element->node.index()) {
                        return false;
                    } else if (parent->type == JsonParserPathNodeType::OBJECT &&
                               node->key() != element->node.key()) {
                        return false;
                    }
                }
                parent = &(*node);
                node++;
                element++;
            }
            return true;
        }

        void JsonParserPath::value() {
            if (_path.empty()) return ;
            switch (_path.back().type) {
               case JsonParserPathNodeType::OBJECT:
                    _path.emplace_back(JsonParserPathNode(JsonParserPathNodeType::VALUE, _lastKey));
                    break;
               case JsonParserPathNodeType::ARRAY:
                   _path.emplace_back(JsonParserPathNode(JsonParserPathNodeType::VALUE, 0));
                   break;
               case JsonParserPathNodeType::VALUE:
                    if (getParent().type == JsonParserPathNodeType::ARRAY) {
                        _path.back().content = _path.back().index() + 1;
                    } else {
                        _path.back().content = _lastKey;
                    }
                    break;
           }
        }

        const JsonParserPathNode &JsonParserPath::getCurrent() const {
            return _path.back();
        }

        const JsonParserPathNode &JsonParserPath::getParent() const {
            return *(++_path.rbegin());
        }

        JsonParserPathNode &JsonParserPath::getCurrent() {
            return _path.back();
        }

        JsonParserPathNode &JsonParserPath::getParent() {
            return *(++_path.rbegin());
        }

        JsonParserPathMatcher::JsonParserPathMatcher(const std::string &filter) {
            char buffer[64]; // Assume max key size is sizeof(buffer)
            auto offset = 0;
            auto emplace_value = [&] () {
                if (offset == 0 || _elements.empty()) return ;
                std::string s(buffer, offset);
                if ((_elements.back().node.type == JsonParserPathNodeType::ARRAY && s == "*") ||
                    (_elements.back().node.type == JsonParserPathNodeType::OBJECT && s == "*")) {
                    _elements.emplace_back(JsonParserPathMatcherElement(
                            JsonParserPathMatcherFilter::WILDCARD,
                            JsonParserPathNode(JsonParserPathNodeType::VALUE)
                    ));
                } else if (_elements.back().node.type == JsonParserPathNodeType::ARRAY) {
                    auto i = boost::lexical_cast<int>(s);
                    _elements.emplace_back(JsonParserPathMatcherElement(
                            JsonParserPathMatcherFilter::EXACT,
                            JsonParserPathNode(JsonParserPathNodeType::VALUE, i)
                    ));
                } else if (s != "?") {
                    _elements.emplace_back(JsonParserPathMatcherElement(
                            JsonParserPathMatcherFilter::EXACT,
                            JsonParserPathNode(JsonParserPathNodeType::VALUE, s)
                    ));
                }
                offset = 0;
            };
            for (auto& c : filter) {
                if (_elements.size() > 0 && _elements.back().filter == JsonParserPathMatcherFilter::MATCH_ALL)
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "? digit can only be put at the end of the filter string");
                if (c == '?') {
                    _elements.emplace_back(JsonParserPathMatcherElement(
                            JsonParserPathMatcherFilter::MATCH_ALL,
                            JsonParserPathNode(JsonParserPathNodeType::VALUE)
                    ));
                } else if (c == '/') {
                    emplace_value();
                    _elements.emplace_back(JsonParserPathMatcherElement(
                            JsonParserPathMatcherFilter::EXACT,
                            JsonParserPathNode(JsonParserPathNodeType::OBJECT)
                            ));
                } else if (c == '[') {
                    emplace_value();
                    _elements.emplace_back(JsonParserPathMatcherElement(
                            JsonParserPathMatcherFilter::EXACT,
                            JsonParserPathNode(JsonParserPathNodeType::ARRAY)
                    ));
                } else if (c != ']') {
                    if (offset < sizeof(buffer))
                        buffer[offset++] = c;
                }
            }
            emplace_value();
        }

        const std::list<JsonParserPathMatcherElement>& JsonParserPathMatcher::getElements() const {
            return _elements;
        }

        std::string JsonParserPathMatcher::toString() const {
            std::stringstream ss;
            const JsonParserPathNode* parent = nullptr;
            for (auto& elem : _elements) {
                switch (elem.node.type) {
                    case JsonParserPathNodeType::OBJECT:
                        ss << "/";
                        break;
                    case JsonParserPathNodeType::ARRAY:
                        ss << "[";
                        break;
                    case JsonParserPathNodeType::VALUE:
                        if (parent->type == JsonParserPathNodeType::OBJECT) {
                            ss << (elem.filter == JsonParserPathMatcherFilter::WILDCARD ? "*" : elem.node.key());
                        } else if (elem.filter != JsonParserPathMatcherFilter::WILDCARD) {
                            ss << elem.node.index();
                        }
                        break;
                }
                parent = &elem.node;
            }
            return ss.str();
        }

        bool JsonParserPathView::match(const JsonParserPathMatcher &matcher) const {
            return _owner->match(matcher, _depth);
        }

        JsonParserPathView JsonParserPathView::view(int depth) {
            return JsonParserPathView(_owner, _depth + 1);
        }

        JsonParserPathView JsonParserPath::view() {
            if (_path.empty())  {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Attempt to create a view at an invalid range");
            }
            return {this, (int)_path.size() - 1};
        }

        JsonParserPathView JsonParserPath::view(int depth) {
            if (depth < 0 || depth >= _path.size())  {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Attempt to create a view at an invalid range");
            }
            return  {this, 0};
        }


    }
}