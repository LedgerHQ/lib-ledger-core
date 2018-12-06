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
#pragma once

#include <string>
#include <vector>
#include "Exception.hpp"

namespace ledger {
    namespace core {
        class DerivationPath {
        public:
            explicit DerivationPath(const std::string& path);
            explicit DerivationPath(const std::vector<uint32_t>& path);
            DerivationPath(const DerivationPath& path);
            DerivationPath(DerivationPath&& path);
            DerivationPath& operator=(DerivationPath&& path);
            DerivationPath& operator=(const DerivationPath& path);
            uint32_t getDepth() const;
            uint32_t getLastChildNum() const;
            uint32_t getNonHardenedChildNum(int index) const;
            uint32_t getNonHardenedLastChildNum() const;
            uint32_t operator[](int index) const;
            DerivationPath operator+(const DerivationPath& derivationPath) const;
            bool operator==(const DerivationPath& path) const;
            bool operator!=(const DerivationPath& path) const;
            DerivationPath getParent() const ;
            bool isRoot() const;
            std::string toString(bool addLeadingM = false) const;
            std::vector<uint32_t> toVector() const;
            bool isHardened(int index) const;
            bool isLastChildHardened() const;

        public:
            static std::vector<uint32_t> parse(const std::string& path);
            static DerivationPath fromScheme(
                const std::string& scheme,
                int coinType,
                int account,
                int node,
                int addressIndex,
                Option<std::string> stopAt = Option<std::string>()
            );

        private:
            inline void assertIndexIsValid(uint32_t index, const std::string& method) const;

        private:
            std::vector<uint32_t> _path;
        };
    }
}
