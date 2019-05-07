/*
 *
 * DerivationPathApi.h
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

#ifndef LEDGER_CORE_DERIVATIONPATHAPI_H
#define LEDGER_CORE_DERIVATIONPATHAPI_H

#include <api/DerivationPath.hpp>
#include <utils/DerivationPath.hpp>

namespace ledger {
    namespace core {
        class DerivationPathApi : public api::DerivationPath {
        public:
            explicit DerivationPathApi(const ledger::core::DerivationPath& path) : _path(path) {};
            int32_t getDepth() override;

            int32_t getChildNum(int32_t index) override;

            int32_t getUnhardenedChildNum(int32_t index) override;

            bool isHardened(int32_t index) override;

            std::string toString() override;

            std::shared_ptr<api::DerivationPath> getParent() override;

            std::vector<int32_t> toArray() override;

        private:
            ledger::core::DerivationPath _path;
        };
    }
}


#endif //LEDGER_CORE_DERIVATIONPATHAPI_H
