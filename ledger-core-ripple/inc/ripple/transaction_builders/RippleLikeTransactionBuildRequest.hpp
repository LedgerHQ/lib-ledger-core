/*
 *
 * RippleLikeTransactionBuildRequest
 *
 * Created by Dimitri Sabadie on 2019/10/17
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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <core/math/BigInt.hpp>
#include <core/utils/Option.hpp>

#include <ripple/api/RippleLikeMemo.hpp>

namespace ledger {
    namespace core {
        struct RippleLikeTransactionBuildRequest {
            RippleLikeTransactionBuildRequest();

            std::string toAddress;
            std::shared_ptr<BigInt> value;
            std::shared_ptr<BigInt> fees;
            BigInt sequence;
            bool wipe;
            std::vector<api::RippleLikeMemo> memos;
            Option<int64_t> destinationTag;
        };
    }
}
