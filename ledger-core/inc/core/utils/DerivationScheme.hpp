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

#pragma once

#include <string>
#include <vector>

#include <core/utils/DerivationPath.hpp>

namespace ledger {
    namespace core {
        enum DerivationSchemeLevel {
            COIN_TYPE,
            ACCOUNT_INDEX,
            NODE,
            ADDRESS_INDEX,
            UNDEFINED
        };

        struct DerivationSchemeNode {
            bool hardened;
            DerivationSchemeLevel level;
            uint32_t value;
        };

        class DerivationScheme {
        public:
            DerivationScheme(const std::string& scheme);
            DerivationScheme(const std::vector<DerivationSchemeNode>& nodes);
            DerivationScheme(const DerivationScheme& cpy);
            DerivationScheme getSchemeFrom(DerivationSchemeLevel level);
            DerivationScheme getSchemeTo(DerivationSchemeLevel level);
            DerivationScheme getSchemeToDepth(size_t depth) const;

            DerivationScheme shift(int n = 1);

            DerivationPath getPath();

            int getCoinType() const;
            int getAccountIndex() const;
            int getNode() const;
            int getAddressIndex() const;

            int getPositionForLevel(DerivationSchemeLevel level) const;

            DerivationScheme& setCoinType(int type);
            DerivationScheme& setAccountIndex(int index);
            DerivationScheme& setNode(int node);
            DerivationScheme& setAddressIndex(int index);

            std::string toString() const;

        private:
            DerivationScheme& setVariable(DerivationSchemeLevel level, int value);
            int getVariable(DerivationSchemeLevel level) const;

        private:
            std::vector<DerivationSchemeNode> _scheme;
        };
    }
}