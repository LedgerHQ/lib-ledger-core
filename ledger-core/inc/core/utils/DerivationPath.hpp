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

#include <core/api/DerivationPath.hpp>
#include <core/utils/Exception.hpp>

namespace ledger {
    namespace core {
        class DerivationPath : public api::DerivationPath {
        public:
            explicit DerivationPath(const std::string& path);
            explicit DerivationPath(const std::vector<int32_t>& path);
            DerivationPath(const DerivationPath& path);
            DerivationPath(DerivationPath&& path);
            DerivationPath& operator=(DerivationPath&& path);
            DerivationPath& operator=(const DerivationPath& path);

            int32_t getDepth() const override;
            int32_t getChildNum(int32_t index) const override;
            int32_t getUnhardenedChildNum(int32_t index) const override;
            bool isHardened(int32_t index) const override;
            std::string toString(bool addLeadingM = false) const override;
            std::shared_ptr<api::DerivationPath> getAbstractParent() const override;
            std::vector<int32_t> toVector() const override;

            DerivationPath getParent() const;
            int32_t getLastChildNum() const;
            int32_t getNonHardenedChildNum(int index) const;
            int32_t getNonHardenedLastChildNum() const;
            bool isRoot() const;
            bool isLastChildHardened() const;
 
            int32_t operator[](int32_t index) const;
            DerivationPath operator+(const DerivationPath& derivationPath) const;
            bool operator==(const DerivationPath& path) const;
            bool operator!=(const DerivationPath& path) const;

        public:
            static std::vector<int32_t> parse(const std::string& path);
            static DerivationPath fromScheme(
                const std::string& scheme,
                int coinType,
                int account,
                int node,
                int addressIndex,
                Option<std::string> stopAt = Option<std::string>()
            );

        private:
            inline void assertIndexIsValid(int32_t index, const std::string& method) const;

            std::vector<int32_t> _path;
        };
    }
}