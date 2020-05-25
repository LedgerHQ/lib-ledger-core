/*
 * AlgorandKeyreg
 *
 * Created by Rémi Barjon on 04/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include <utils/Option.hpp>

#include <cstdint>
#include <string>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    class KeyRegTxnFields
    {
    public:
        KeyRegTxnFields() = default;
        KeyRegTxnFields(Option<bool> nonParticipation,
                        std::string selectionPk,
                        uint64_t voteFirst,
                        uint64_t voteKeyDilution,
                        std::string votePk,
                        uint64_t voteLast)
            : nonParticipation(std::move(nonParticipation))
            , selectionPk(std::move(selectionPk))
            , voteFirst(voteFirst)
            , voteKeyDilution(voteKeyDilution)
            , votePk(std::move(votePk))
            , voteLast(voteLast)
        {}

        Option<bool> nonParticipation;
        std::string selectionPk;
        uint64_t voteFirst;
        uint64_t voteKeyDilution;
        std::string votePk;
        uint64_t voteLast;
    };

} // namespace model
} // namespace ledger
} // namespace core
} // namespace algorand

