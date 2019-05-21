/*
 *
 * DerivationScheme
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/04/2017.
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

#include <boost/lexical_cast.hpp>
#include <sstream>

#include <core/collections/collections.hpp>
#include <core/utils/DerivationScheme.hpp>

namespace ledger {
    namespace core {
        DerivationScheme::DerivationScheme(const std::string &scheme) {
            auto segments = strings::split(scheme, "/");
            if (segments.size() == 0) {
                throw Exception(api::ErrorCode::INVALID_DERIVATION_SCHEME, "Derivation scheme is empty");
            }
            if (segments.front() == "m") {
                segments.erase(segments.begin(), segments.begin() + 1);
            }
            for (auto& segment : segments) {
                if (segment.length() == 0) {
                    throw Exception(api::ErrorCode::INVALID_DERIVATION_SCHEME, "Derivation scheme segment is empty");
                }
                DerivationSchemeNode node;
                node.hardened = segment.back() == '\'';
                node.value = 0;
                if (node.hardened) {
                    segment.erase(segment.end() - 1, segment.end());
                }
                if (segment == "<coin_type>") {
                    node.level = DerivationSchemeLevel::COIN_TYPE;
                } else if (segment == "<account>") {
                    node.level = DerivationSchemeLevel::ACCOUNT_INDEX;
                } else if (segment == "<node>") {
                    node.level = DerivationSchemeLevel::NODE;
                } else if (segment == "<address>") {
                    node.level = DerivationSchemeLevel::ADDRESS_INDEX;
                } else {
                    node.level = DerivationSchemeLevel::UNDEFINED;
                    try {
                        if (strings::startsWith(segment, "0x")) {
                            std::istringstream is(segment);
                            is >> std::hex >> node.value;
                        } else {
                            node.value = boost::lexical_cast<unsigned int>(segment);
                        }
                    } catch (...) {
                        throw make_exception(api::ErrorCode::INVALID_DERIVATION_SCHEME, "Unable to parse {} as number", segment);
                    }
                }
                _scheme.push_back(node);
            }
        }

        DerivationScheme::DerivationScheme(const std::vector<DerivationSchemeNode> &nodes) {
            _scheme = nodes;
        }

        DerivationScheme::DerivationScheme(const DerivationScheme &cpy) {
            _scheme = cpy._scheme;
        }

        DerivationScheme DerivationScheme::getSchemeFrom(DerivationSchemeLevel level) {
            auto it = _scheme.begin();
            auto end = _scheme.end();
            while (it != end) {
                if (it->level == level) {
                    auto scheme = std::vector<DerivationSchemeNode>(it, end);
                    return DerivationScheme(scheme);
                }
                it++;
            }
            return *this;
        }

        DerivationScheme DerivationScheme::shift(int n) {
            auto it = _scheme.begin() + n;
            auto end = _scheme.end();
            auto scheme = std::vector<DerivationSchemeNode>(it, end);
            return DerivationScheme(scheme);
        }

        DerivationScheme DerivationScheme::getSchemeTo(DerivationSchemeLevel level) {
            auto it = _scheme.begin();
            auto end = _scheme.end();
            while (it != end) {
                if (it->level == level) {
                    auto scheme = std::vector<DerivationSchemeNode>(_scheme.begin(), it + 1);
                    return DerivationScheme(scheme);
                }
                it++;
            }
            return *this;
        }

        DerivationPath DerivationScheme::getPath() {
            std::function<uint32_t (const DerivationSchemeNode&)> map = [] (const DerivationSchemeNode &item) -> uint32_t {
                return item.value | (item.hardened ? 0x80000000 : 0x00);
            };
            auto segments = functional::map(_scheme, map);
            return DerivationPath(segments);
        }

        DerivationScheme &DerivationScheme::setCoinType(int type) {
            return setVariable(DerivationSchemeLevel::COIN_TYPE, type);
        }

        DerivationScheme &DerivationScheme::setAccountIndex(int index) {
            return setVariable(DerivationSchemeLevel::ACCOUNT_INDEX, index);
        }

        DerivationScheme &DerivationScheme::setNode(int node) {
            return setVariable(DerivationSchemeLevel::NODE, node);
        }

        DerivationScheme &DerivationScheme::setAddressIndex(int index) {
            return setVariable(DerivationSchemeLevel::ADDRESS_INDEX, index);
        }

        DerivationScheme &DerivationScheme::setVariable(DerivationSchemeLevel level, int value) {
            for (auto& item : _scheme) {
                if (item.level == level) {
                    item.value = value;
                }
            }
            return *this;
        }

        int DerivationScheme::getCoinType() const {
            return getVariable(DerivationSchemeLevel::COIN_TYPE);
        }

        int DerivationScheme::getAccountIndex() const {
            return getVariable(DerivationSchemeLevel::ACCOUNT_INDEX);
        }

        int DerivationScheme::getNode() const {
            return getVariable(DerivationSchemeLevel::NODE);
        }

        int DerivationScheme::getAddressIndex() const {
            return getVariable(DerivationSchemeLevel::ADDRESS_INDEX);
        }

        int DerivationScheme::getVariable(DerivationSchemeLevel level) const {
            for (auto& i : _scheme) {
                if (i.level == level)
                    return i.value;
            }
            return 0;
        }

        std::string DerivationScheme::toString() const {
            std::stringstream ss;
            bool first = true;
            for (auto& item : _scheme) {
                if (!first) {
                    ss << "/";
                }
                first = false;
                switch (item.level) {
                    case DerivationSchemeLevel::UNDEFINED:
                        ss << item.value;
                        break;
                    case DerivationSchemeLevel::COIN_TYPE:
                        ss << "<coin_type>";
                        break;
                    case DerivationSchemeLevel::ACCOUNT_INDEX:
                        ss << "<account>";
                        break;
                    case DerivationSchemeLevel::NODE:
                        ss << "<node>";
                        break;
                    case DerivationSchemeLevel::ADDRESS_INDEX:
                        ss << "<address>";
                        break;
                }
                if (item.hardened) {
                    ss << "'";
                }
            }
            return ss.str();
        }

        int DerivationScheme::getPositionForLevel(DerivationSchemeLevel level) const {
            auto index = 0;
            for (auto& i : _scheme) {
                if (i.level == level)
                    return index;
                index += 1;
            }
            return -1;
        }
    }
}
