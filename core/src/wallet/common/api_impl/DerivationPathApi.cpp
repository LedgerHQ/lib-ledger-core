/*
 *
 * DerivationPathApi.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 23/03/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "DerivationPathApi.h"

int32_t ledger::core::DerivationPathApi::getDepth() {
    return _path.getDepth();
}

int32_t ledger::core::DerivationPathApi::getChildNum(int32_t index) {
    return _path[index];
}

int32_t ledger::core::DerivationPathApi::getUnhardenedChildNum(int32_t index) {
    return _path.getNonHardenedChildNum(index);
}

bool ledger::core::DerivationPathApi::isHardened(int32_t index) {
    return _path.isHardened(index);
}

std::string ledger::core::DerivationPathApi::toString() {
    return _path.toString(false);
}

std::shared_ptr<ledger::core::api::DerivationPath> ledger::core::DerivationPathApi::getParent() {
    return std::make_shared<ledger::core::DerivationPathApi>(_path.getParent());
}

std::vector<int32_t> ledger::core::DerivationPathApi::toArray() {
    auto vector = _path.toVector();
    return std::vector<int32_t>(vector.begin(), vector.end());
}

std::shared_ptr<ledger::core::api::DerivationPath> ledger::core::api::DerivationPath::parse(const std::string &path) {
    return std::make_shared<DerivationPathApi>(ledger::core::DerivationPath(path));
}