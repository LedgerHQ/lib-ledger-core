/*
 *
 * BigIntImpl
 * ledger-core
 *
 * Created by Pierre Pollastri on 04/11/2016.
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
#include "BigIntImpl.hpp"
#include <fmt/string.h>

namespace ledger { namespace core { namespace api {

            std::shared_ptr<ledger::core::api::BigInt>
            BigIntImpl::add(const std::shared_ptr<ledger::core::api::BigInt> &i) {
                auto result = this->_bigi + std::dynamic_pointer_cast<BigIntImpl>(i)->_bigi;
                return std::shared_ptr<BigIntImpl>(new BigIntImpl(result));
            }

            std::shared_ptr<ledger::core::api::BigInt>
            BigIntImpl::subtract(const std::shared_ptr<ledger::core::api::BigInt> &i) {
                auto result = this->_bigi - std::dynamic_pointer_cast<BigIntImpl>(i)->_bigi;
                return std::shared_ptr<BigIntImpl>(new BigIntImpl(result));
            }

            std::shared_ptr<ledger::core::api::BigInt>
            BigIntImpl::multiply(const std::shared_ptr<ledger::core::api::BigInt> &i) {
                auto result = this->_bigi * std::dynamic_pointer_cast<BigIntImpl>(i)->_bigi;
                return std::shared_ptr<BigIntImpl>(new BigIntImpl(result));
            }

            std::shared_ptr<ledger::core::api::BigInt>
            BigIntImpl::divide(const std::shared_ptr<ledger::core::api::BigInt> &i) {
                auto result = this->_bigi / std::dynamic_pointer_cast<BigIntImpl>(i)->_bigi;
                return std::shared_ptr<BigIntImpl>(new BigIntImpl(result));
            }

            std::vector<std::shared_ptr<ledger::core::api::BigInt>>
            BigIntImpl::divideAndRemainder(const std::shared_ptr<ledger::core::api::BigInt> &i) {
                auto res1 = std::shared_ptr<BigIntImpl>(new BigIntImpl(this->_bigi / std::dynamic_pointer_cast<BigIntImpl>(i)->_bigi));
                auto res2 = std::shared_ptr<BigIntImpl>(new BigIntImpl(this->_bigi + std::dynamic_pointer_cast<BigIntImpl>(i)->_bigi));
                return std::vector<std::shared_ptr<ledger::core::api::BigInt>>({res1, res2});
            }

            std::shared_ptr<ledger::core::api::BigInt> BigIntImpl::pow(int32_t exponent) {
                return std::shared_ptr<BigIntImpl>(new BigIntImpl(_bigi.pow((unsigned short) exponent)));
            }

            std::string BigIntImpl::toDecimalString(int32_t precision, const std::string &decimalSeparator,
                                                    const std::string &thousandSeparator) {
                return "";
            }

            int32_t BigIntImpl::intValue() {
                return _bigi.toInt();
            }

            int32_t BigIntImpl::compare(const std::shared_ptr<ledger::core::api::BigInt> &i) {
                return _bigi.compare(dynamic_cast<BigIntImpl *>(i.get())->_bigi);
            }

            std::string BigIntImpl::toString(int32_t radix) {
                if (radix == 10)
                    return _bigi.toString();
                else
                    return _bigi.toHexString();
            }

            std::shared_ptr<BigInt> BigInt::fromDecimalString(const std::string &s, int32_t precision,
                                                              const std::string &decimalSeparator) {
                fmt::StringWriter writer;
                fmt::StringWriter decimaleWriter;
                auto hasReachedDecimalPart = false;
                auto d = 0;
                for (auto i = 0; i < s.length(); i++) {
                    auto c = s[i];
                    if (c >= '0' && c <= '9' && !hasReachedDecimalPart) {
                        writer << c;
                    } else if (c == decimalSeparator[0] && !hasReachedDecimalPart) {
                        hasReachedDecimalPart = true;
                    } else if (c >= '0' && c <= '9' && hasReachedDecimalPart) {
                        decimaleWriter << c;
                    } else {
                        d += 1;
                    }
                }
                while (decimaleWriter.size() < precision) {
                    decimaleWriter << '0';
                }
                writer << decimaleWriter.c_str();
                return fromIntegerString(writer.c_str(), 10);
            }

            std::shared_ptr<BigInt> BigInt::fromIntegerString(const std::string &s, int32_t radix) {
                if (radix == 10) {
                    return std::shared_ptr<BigIntImpl>(new BigIntImpl(ledger::core::BigInt::fromDecimal(s)));
                } else {
                    return std::shared_ptr<BigIntImpl>(new BigIntImpl(ledger::core::BigInt::fromHex(s)));
                }
            }

        }
    }
}
