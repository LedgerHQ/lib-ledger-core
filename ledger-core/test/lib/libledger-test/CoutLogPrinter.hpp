/*
 *
 * CoutLogPrinter
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/11/2016.
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
#ifndef LEDGER_CORE_COUTLOGPRINTER_HPP
#define LEDGER_CORE_COUTLOGPRINTER_HPP

#include "api/LogPrinter.hpp"
#include <memory>

class CoutLogPrinter : public ledger::core::api::LogPrinter {
public:
    CoutLogPrinter(const std::shared_ptr<ledger::core::api::ExecutionContext>& context);
    virtual void printError(const std::string &message) override;

    virtual void printInfo(const std::string &message) override;

    virtual void printDebug(const std::string &message) override;

    virtual void printWarning(const std::string &message) override;

    virtual void printApdu(const std::string &message) override;

    virtual void printCriticalError(const std::string &message) override;

    virtual std::shared_ptr<ledger::core::api::ExecutionContext> getContext() override;

private:
    std::shared_ptr<ledger::core::api::ExecutionContext> _context;
};


#endif //LEDGER_CORE_COUTLOGPRINTER_HPP
