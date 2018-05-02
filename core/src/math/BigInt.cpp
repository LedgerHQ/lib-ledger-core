/*
 *
 * BigInt
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/09/2016.
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

#include "BigInt.h"
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include "../collections/collections.hpp"


#include <cstdlib>

namespace ledger {
    namespace core {

        const BigInt BigInt::ZERO = BigInt(0);
        const BigInt BigInt::ONE = BigInt(1);
        const BigInt BigInt::TEN = BigInt(10);
        const std::string BigInt::DIGITS = std::string("0123456789abcdefghijklmnopqrstuvwxyz");
        const int BigInt::MIN_RADIX = 2;
        const int BigInt::MAX_RADIX = 36;

        BigInt::BigInt() {
            _bigd = bdNew();
            _negative = false;
        }

        BigInt::BigInt(const BigInt& cpy) {
            _bigd = bdNew();
            _negative = cpy._negative;
            bdSetEqual(_bigd, cpy._bigd);
        }

        BigInt::BigInt(const void *data, size_t length, bool negative) : BigInt() {
            bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(data), length);
            _negative = negative;
        }

        BigInt::BigInt(const std::vector<uint8_t> &data, bool negative)
        : BigInt(data.data(), data.size(), negative){

        }

        BigInt::BigInt(int value)
                : BigInt() {
            bdSetShort(_bigd, (bdigit_t)std::abs(value));
            if (value < 0) {
                _negative = true;
            }
        }

        BigInt::BigInt(unsigned int value) : BigInt() {
            bdSetShort(_bigd, (bdigit_t)value);
            _negative = false;
        }

        BigInt::BigInt(unsigned long long value) : BigInt() {
            auto bytes = endianness::scalar_type_to_array<unsigned long long>(value, endianness::Endianness::BIG);
            bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(bytes), sizeof(unsigned long long));
            std::free(bytes);
            _negative = false;
        }

        BigInt::BigInt(int64_t value) : BigInt() {
            auto bytes = endianness::scalar_type_to_array<int64_t >(std::abs(value), endianness::Endianness::BIG);
            bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(bytes), sizeof(int64_t));
            std::free(bytes);
            _negative = value < 0LL;
        }

        BigInt& BigInt::assignI64(int64_t value) {
            auto bytes = endianness::scalar_type_to_array<int64_t >(std::abs(value), endianness::Endianness::BIG);
            bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(bytes), sizeof(int64_t));
            std::free(bytes);
            _negative = value < 0LL;
            return *this;
        }

        BigInt::BigInt(const std::string &str, int radix) : BigInt() {
           if (radix == 10) {
               _negative = str[0] == '-';
               bdConvFromDecimal(_bigd, str.c_str());
           } else if (radix == 16) {
               bdConvFromHex(_bigd, str.c_str());
           } else {
               throw std::invalid_argument("Cannot handle radix");
           }
        }

        BigInt::~BigInt() {
            if (_bigd != nullptr) {
                bdFree(&_bigd);
            }
        }

        int BigInt::toInt() const {
            return bdToShort(_bigd) * (_negative ? -1 : 1);
        }

        unsigned int BigInt::toUnsignedInt() const {
            return bdToShort(_bigd);
        }

        std::string BigInt::toString() const {
            size_t nchars = bdConvToDecimal(_bigd, NULL, 0);
            auto s = std::shared_ptr<char>(new char[nchars + 1]);
            bdConvToDecimal(_bigd, s.get(), nchars + 1);
            auto out = std::string(s.get());
            if (this->isNegative()) {
                out = "-" + out;
            }
            return out;
        }

        std::string BigInt::toHexString() const {
            size_t nchars = bdConvToHex(_bigd, NULL, 0);
            auto s = std::shared_ptr<char>(new char[nchars + 1]);
            bdConvToHex(_bigd, s.get(), nchars + 1);
            auto out = std::string(s.get());
            if (out.length() % 2 != 0) {
                out = "0" + out;
            }
            return out;
        }

        unsigned long BigInt::getBitSize() const {
            return bdSizeof(_bigd) * sizeof(SimpleInt) * 8;
        }

        BigInt *BigInt::from_hex(const std::string &str) {
            return new BigInt(str, 16);
        }

        BigInt *BigInt::from_dec(const std::string &str) {
            return new BigInt(str, 10);
        }

        BigInt BigInt::operator+(const BigInt &rhs) const {
            if (rhs.isNegative() && !this->isNegative()) {
                return *this - rhs.positive();
            } else if (this->isNegative() && !rhs.isNegative()) {
                return rhs - this->positive();
            }
            BigInt result;
            bdAdd(result._bigd, this->_bigd, rhs._bigd);
            result._negative = rhs.isNegative() && this->isNegative();
            return result;
        }

        BigInt BigInt::operator-(const BigInt &rhs) const {
            if (this->isPositive() && rhs.isNegative()) {
                return *this + rhs.positive();
            } else if (this->isNegative() && rhs.isPositive()) {
                return *this + rhs.negative();
            } else if (rhs > *this) {
                return (rhs - *this).negative();
            }
            BigInt result;
            bdSubtract(result._bigd, this->_bigd, rhs._bigd);
            return result;
        }

        BigInt BigInt::operator*(const BigInt &rhs) const {
            BigInt result;
            bdMultiply(result._bigd, this->_bigd, rhs._bigd);
            result._negative = this->isNegative() != rhs.isNegative();
            return result;
        }

        BigInt BigInt::operator/(const BigInt &rhs) const {
            BigInt result;
            BigInt remainder;
            bdDivide(result._bigd, remainder._bigd, this->_bigd, rhs._bigd);
            result._negative = this->isNegative() != rhs.isNegative();
            return result;
        }

        BigInt BigInt::operator%(const BigInt &rhs) const {
            BigInt result;
            BigInt remainder;
            bdDivide(result._bigd, remainder._bigd, this->_bigd, rhs._bigd);
            remainder._negative = this->isNegative();
            return remainder;
        }

        BigInt &BigInt::operator++() {
            if (this->isNegative()) {
                bdDecrement(_bigd);
            } else {
                bdIncrement(_bigd);
            }
            return *this;
        }

        BigInt BigInt::operator++(int) {
            BigInt temp = *this;
            ++*this;
            return temp;
        }

        BigInt &BigInt::operator--() {
            if (this->isPositive()) {
                bdDecrement(_bigd);
            } else {
                bdIncrement(_bigd);
            }
            return *this;
        }

        BigInt BigInt::operator--(int) {
            BigInt temp = *this;
            ++*this;
            return temp;
        }

        void BigInt::operator=(const BigInt &a) {
            bdSetEqual(_bigd, a._bigd);
            _negative = a._negative;
        }

        bool BigInt::isNegative() const {
            return _negative && !this->isZero();
        }

        bool BigInt::isPositive() const {
            return !_negative || this->isZero();
        }

        bool BigInt::isZero() const {
            return bdIsZero(_bigd) != 0;
        }

        BigInt BigInt::negative() const {
            BigInt result = *this;
            result._negative = true;
            return result;
        }

        BigInt BigInt::positive() const {
            BigInt result = *this;
            result._negative = false;
            return result;
        }

        bool BigInt::operator<(const BigInt &rhs) const {
            if (this->isNegative() && rhs.isPositive()) {
                return true;
            } else if (this->isPositive() && rhs.isNegative()) {
                return false;
            } else if (this->isNegative() && rhs.isNegative()) {
                return bdCompare(this->_bigd, rhs._bigd) == 1;
            }
            return bdCompare(this->_bigd, rhs._bigd) == -1;
        }

        bool BigInt::operator<=(const BigInt &rhs) const {
            if (this->isNegative() && rhs.isPositive()) {
                return true;
            } else if (this->isPositive() && rhs.isNegative()) {
                return false;
            } else if (this->isNegative() && rhs.isNegative()) {
                return bdCompare(this->_bigd, rhs._bigd) >= 0;
            }
            return bdCompare(this->_bigd, rhs._bigd) <= 0;
        }

        bool BigInt::operator==(const BigInt &rhs) const {
            return this->_negative == rhs._negative && bdCompare(this->_bigd, rhs._bigd) == 0;
        }

        bool BigInt::operator>(const BigInt &rhs) const {
            return rhs < *this;
        }

        bool BigInt::operator>=(const BigInt &) const {
            return false;
        }

        BigInt BigInt::pow(unsigned short p) const {
            BigInt result;
            bdPower(result._bigd, _bigd, p);
            result._negative = isNegative() && (p % 2 != 0 || p == 0);
            return result;
        }

        std::vector<uint8_t> BigInt::toByteArray() const {
            size_t nchars = bdConvToOctets(_bigd, NULL, 0);
            std::vector<uint8_t> out = std::vector<uint8_t >(nchars);
            bdConvToOctets(_bigd, reinterpret_cast<unsigned char *>(out.data()), nchars);
            return out;
        }

        uint64_t BigInt::toUint64() const {
            std::vector<uint8_t> result(sizeof(uint64_t));
            bdConvToOctets(_bigd, result.data(), sizeof(uint64_t));
            if (ledger::core::endianness::isSystemLittleEndian()) {
                std::reverse(result.begin(), result.end());
            }
            return reinterpret_cast<uint64_t *>(result.data())[0];
        }

        int64_t BigInt::toInt64() const {
            std::vector<uint8_t> result(sizeof(uint64_t));
            bdConvToOctets(_bigd, result.data(), sizeof(uint64_t));
            if (ledger::core::endianness::isSystemLittleEndian()) {
                std::reverse(result.begin(), result.end());
            }
            return reinterpret_cast<int64_t *>(result.data())[0] * (_negative ? -1 : 1);
        }

        int BigInt::compare(const BigInt &rhs) const {
            if (this->isNegative() && rhs.isPositive()) {
                return -1;
            } else if (this->isPositive() && rhs.isNegative()) {
                return 1;
            } else if (this->isNegative() && rhs.isNegative()) {
                return -bdCompare(this->_bigd, rhs._bigd);
            }
            return bdCompare(this->_bigd, rhs._bigd);
        }

        BigInt BigInt::fromHex(const std::string &str) {
            return BigInt(str, 16);
        }

        BigInt BigInt::fromDecimal(const std::string &str) {
            return BigInt(str, 10);
        }

        BigInt BigInt::fromString(const std::string &str) {
            if (strings::startsWith(str, "0x")) {
                return BigInt::fromHex(str.substr(2, str.length()));
            } else {
                return BigInt::fromDecimal(str);
            }
        }

        BigInt::BigInt(BigInt &&mov) {
            _bigd = mov._bigd;
            _negative = mov._negative;
            mov._bigd = nullptr;
        }

    }
}
