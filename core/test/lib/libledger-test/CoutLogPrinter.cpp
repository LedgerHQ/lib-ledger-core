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
#include "CoutLogPrinter.hpp"
#include <iostream>
/* FOREGROUND */
static const std::string RST = "\x1B[0m";
static const std::string KRED = "\x1B[31m";
static const std::string KGRN = "\x1B[32m";
static const std::string KYEL = "\x1B[33m";
static const std::string KBLU = "\x1B[34m";
static const std::string KMAG = "\x1B[35m";
static const std::string KCYN = "\x1B[36m";
static const std::string KWHT = "\x1B[37m";

#define FRED(x) KRED + x + RST
#define FGRN(x) KGRN + x + RST
#define FYEL(x) KYEL + x + RST
#define FBLU(x) KBLU + x + RST
#define FMAG(x) KMAG + x + RST
#define FCYN(x) KCYN + x + RST
#define FWHT(x) KWHT + x + RST

#define BOLD(x) std::string("\x1B[1m") + x + RST
#define UNDL(x) std::string("\x1B[4m") + x + RST

CoutLogPrinter::CoutLogPrinter(const std::shared_ptr<ledger::core::api::ExecutionContext> &context) {
    _context = context;
}

void CoutLogPrinter::printError(const std::string &message) {
    std::cout << FRED(message);
}

void CoutLogPrinter::printInfo(const std::string &message) {
    std::cout << FGRN(message);
}

void CoutLogPrinter::printDebug(const std::string &message) {
    std::cout << message;
}

void CoutLogPrinter::printWarning(const std::string &message) {
    std::cout << FYEL(message);
}

void CoutLogPrinter::printApdu(const std::string &message) {
    std::cout << FWHT(message);
}

void CoutLogPrinter::printCriticalError(const std::string &message) {
    std::cout << FRED(message) << std::endl;
}

std::shared_ptr<ledger::core::api::ExecutionContext> CoutLogPrinter::getContext() {
    return _context;
}

