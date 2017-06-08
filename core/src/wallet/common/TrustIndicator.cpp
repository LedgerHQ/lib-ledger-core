/*
 *
 * TrustIndicator
 * ledger-core
 *
 * Created by Pierre Pollastri on 08/06/2017.
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
#include "TrustIndicator.h"

using namespace rapidjson;

namespace ledger {
    namespace core {

        TrustIndicator::TrustIndicator() {
            _weight = 0;
            _level = api::TrustLevel::DROPPED;
        }

        TrustIndicator &TrustIndicator::setTrustWeight(int32_t weight) {
            _weight = weight;
            return *this;
        }

        TrustIndicator &TrustIndicator::setTrustLevel(api::TrustLevel level) {
            _level = level;
            return *this;
        }

        TrustIndicator &TrustIndicator::addConflictingOperationUid(const std::string &uid) {
            auto it = std::find(_conflicts.begin(), _conflicts.end(), uid);
            if (it == _conflicts.end()) {
                _conflicts.push_back(uid);
            }
            return *this;
        }

        TrustIndicator &TrustIndicator::removeConflictingOperationUid(const std::string &uid) {
            auto it = std::find(_conflicts.begin(), _conflicts.end(), uid);
            if (it != _conflicts.end()) {
                _conflicts.erase(it);
            }
            return *this;
        }

        int32_t TrustIndicator::getTrustWeight() {
            return _weight;
        }

        api::TrustLevel TrustIndicator::getTrustLevel() {
            return _level;
        }

        std::vector<std::string> TrustIndicator::getConflictingOperationUids() {
            return _conflicts;
        }

        std::string TrustIndicator::getOrigin() {
            return _origin;
        }

        TrustIndicator &TrustIndicator::setOrigin(const std::string &origin) {
            _origin = origin;
            return *this;
        }

    }
}