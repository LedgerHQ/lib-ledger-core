/*
 *
 * DerivationPath
 * ledger-core
 *
 * Created by Pierre Pollastri on 16/12/2016.
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
#include "DerivationPath.hpp"
#include <string>
#include <cstdlib>
#include "collections/vector.hpp"
#include <sstream>
#include "fmt/format.h"
#include "collections/strings.hpp"
#include <algorithm>

namespace ledger {
    namespace core {

        static const auto HARD_BIT = 0x80000000;

        DerivationPath::DerivationPath(const std::string &path) : DerivationPath(parse(path)) {

        }

        DerivationPath::DerivationPath(const std::vector<uint32_t> &path) : _path(path) {

        }

        std::vector<uint32_t> DerivationPath::parse(const std::string &path) {
            std::string currentNode = "";
            bool hardened = false;
            bool lastCharWasZero = false;
            bool nextIntIsInHex = false;
            std::vector<uint32_t> result;
            int sepCount = 0;
            int index = 0;

            auto pushSegment = [&] () {
                if (currentNode.size() == 0 && sepCount > 0) {
                    throw Exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Invalid path format (empty segment)");
                }
                if (!currentNode.empty()) {
                    uint32_t value;

                    if (nextIntIsInHex) {
                        value = (uint32_t) std::stoul(currentNode, nullptr, 16);
                    } else {
                        value = (uint32_t) std::stoul(currentNode, nullptr, 10);
                    }
                    if (hardened) {
                        value = HARD_BIT | value;
                    }
                    result.push_back(value);
                }
                nextIntIsInHex = false;
                hardened = false;
                currentNode.clear();
                sepCount += 1;
            };

            for (auto c : path) {
                if (c == '/') {
                   pushSegment();
                } else if (c == '\'') {
                    hardened = true;
                } else if (lastCharWasZero && (c == 'x' || c == 'X')) {
                    nextIntIsInHex = true;
                    lastCharWasZero = false;
                } else if (c == 'm') {
                    if (index > 0) {
                        throw Exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Invalid path format (unexpected master node)");
                    }
                } else if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    currentNode += c;
                    lastCharWasZero = (c == '0');
                } else {
                    std::string error("Invalid path format (unexpected '");
                    error += c;
                    error += "' character)";
                    throw Exception(api::ErrorCode::ILLEGAL_ARGUMENT, error);
                }
                index += 1;
            }
            pushSegment();
            return result;
        }



        uint32_t DerivationPath::getDepth() const {
            return (uint32_t) _path.size();
        }

        uint32_t DerivationPath::getLastChildNum() const {
            assertIndexIsValid(getDepth() - 1, "ledger::core::DerivationPath::getLastChildNum");
            return _path.back();
        }

        uint32_t DerivationPath::getNonHardenedChildNum(int index) const  {
            assertIndexIsValid(index, "ledger::core::DerivationPath::getNonHardenedChildNum");
            if (isHardened(index)) {
                return (*this)[index] & ~HARD_BIT;
            } else {
                return (*this)[index];
            }
        }

        uint32_t DerivationPath::getNonHardenedLastChildNum() const {
            assertIndexIsValid(getDepth() - 1, "ledger::core::DerivationPath::getNonHardenedLastChildNum");
            return getLastChildNum() & ~HARD_BIT;
        }

        uint32_t DerivationPath::operator[](int index) const {
            assertIndexIsValid(index, "ledger::core::DerivationPath::operator[]");
            return _path[index];
        }

        DerivationPath DerivationPath::operator+(const DerivationPath &derivationPath) const {
            return DerivationPath(vector::concat(_path, derivationPath._path));
        }

        bool DerivationPath::operator==(const DerivationPath &path) const {
            if (this->getDepth() != path.getDepth())
                return false;
            return std::equal(_path.begin(), _path.end(), path._path.begin());
        }

        bool DerivationPath::operator!=(const DerivationPath &path) const {
            return !((*this) == path);
        }

        DerivationPath DerivationPath::getParent() const {
            assertIndexIsValid(getDepth() - 1, "ledger::core::DerivationPath::getParent");
            return DerivationPath(std::vector<uint32_t>(_path.begin(), _path.end() - 1));
        }

        bool DerivationPath::isRoot() const {
            return getDepth() == 0;
        }

        std::string DerivationPath::toString(bool addLeadingM) const {
            std::stringstream ss;
            if (addLeadingM) {
                ss << "m/";
            }
            auto index = 0;
            auto depth = getDepth();
            for (const auto& childNum : _path) {
                auto v = ~HARD_BIT & childNum;
                ss << v;
                if ((HARD_BIT & childNum) == HARD_BIT)
                    ss << '\'';
                index += 1;
                if (index < depth)
                    ss << '/';
            }
            return ss.str();
        }

        std::vector<uint32_t> DerivationPath::toVector() const {
            return _path;
        }

        bool DerivationPath::isHardened(int index) const {
            assertIndexIsValid(index, "ledger::core::DerivationPath::isHardened");
            return (_path[index] & HARD_BIT) == HARD_BIT;
        }

        bool DerivationPath::isLastChildHardened() const {
            assertIndexIsValid(getDepth() - 1, "ledger::core::DerivationPath::isLastChildHardened");
            return isHardened(getDepth() - 1);
        }

        void DerivationPath::assertIndexIsValid(int index, const std::string& method) const {
            if (index >= getDepth()) {
                throw Exception(api::ErrorCode::RUNTIME_ERROR, fmt::format("{} - Index ({}) is out of bound. Path depth is {}", method, index, getDepth()));
            } else if (index < 0) {
                throw Exception(api::ErrorCode::RUNTIME_ERROR, fmt::format("{} - Cannot get parent of root derivation path", method));
            }
        }

        DerivationPath::DerivationPath(const DerivationPath &path) : _path(path._path) {

        }

        DerivationPath::DerivationPath(DerivationPath &&path) : _path(path._path) {

        }

        DerivationPath& DerivationPath::operator=(DerivationPath &&path) {
            this->_path = path._path;
            return *this;
        }

        DerivationPath& DerivationPath::operator=(const DerivationPath &path) {
            this->_path = path._path;
            return *this;
        }


    }
}