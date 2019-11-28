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

#pragma once

#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER)
        #include <libcore_export.h>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

#include <string>
#include <vector>
#include <bigd.h>
#include <memory>

#include <core/api/BigInt.hpp>
#include <core/utils/Endian.hpp>

namespace ledger {
    namespace core {
        /**
         * Helper class used to deal with really big integers.
         * @headerfile BigInt.h <ledger/core/math/BigInt.h>
         */
        class BigInt : public api::BigInt {
        public:
            static LIBCORE_EXPORT const BigInt ZERO;
            static LIBCORE_EXPORT const BigInt ONE;
            static LIBCORE_EXPORT const BigInt TEN;

            typedef unsigned int SimpleInt;
            typedef unsigned long DoubleInt;

            /**
             * Available digits for conversion to and from strings.
             */
            static LIBCORE_EXPORT const std::string DIGITS;

            /**
             * The maximum radix available for conversion to and from strings.
             */
            static LIBCORE_EXPORT const int MIN_RADIX;

            /**
             * The minimum radix available for conversion to and from strings.
             */
            static LIBCORE_EXPORT const int MAX_RADIX;

            /**
             * Creates a new BigInt from the given hexadecimal encoded string.
             * @param str The number encoded in hexadecimal (e.g. "E0A1B3")
             * @return An instance of BigInt
             */
            static LIBCORE_EXPORT BigInt* from_hex(const std::string& str);

            /**
            * Creates a new BigInt from the given hexadecimal encoded string.
            * @param str The number encoded in hexadecimal (e.g. "E0A1B3")
            */
            static LIBCORE_EXPORT BigInt fromHex(const std::string& str);

            /**
             * Creates a new BigInt from the given decimal encoded string.
             * @param str The number encoded in decimal (e.g. "125")
             * @return
             */
            static LIBCORE_EXPORT BigInt* from_dec(const std::string& str);

            /**
            * Creates a new BigInt from the given decimal encoded string.
            * @param str The number encoded in decimal (e.g. "125")
            * @return
            */
            static LIBCORE_EXPORT BigInt fromDecimal(const std::string& str);

            static LIBCORE_EXPORT BigInt fromString(const std::string& str);

            template <typename T>
            static BigInt fromScalar(T value) {
              BigInt result;
              result.assignScalar<T>(value);
              return result;
            }

        public:
            BigInt();
            BigInt(const BigInt& cpy);
            BigInt(BigInt&& mov);

            /**
             * Initializes a new BigInt with the given big endian data.
             * @param data The big integer data formatted in big endian
             * @param length The length of the data
             * @param negative true if the number is negative false otherwise
             */
            BigInt(const void *data, size_t length, bool negative);
            BigInt(const std::vector<uint8_t>& data, bool negative);

            /**
             * Initializes a new BigInt with the runtime int.
             * @param value
             * @param bits
             * @return
             */
            explicit BigInt(int value);
            explicit BigInt(unsigned int value);
            explicit BigInt(unsigned long long value);
            explicit BigInt(int64_t value);

            /**
             * Initializes a new BigInt with the given string representation.
             * @param str
             * @return
             */
            BigInt(const std::string& str);

            virtual ~BigInt();

            /**
             * Converts the BigInt to int
             * @return
             */
            int toInt() const;

            /**
             * Converts the BigInt to unsigned int
             * @return
             */
            unsigned toUnsignedInt() const;

            uint64_t toUint64() const;
            int64_t toInt64() const;

            /**
             * Serializes the BigInt into a decimal std::string.
             * @param str
             * @param radix
             * @return
             */
            std::string toString() const;
            std::string to_string() const {return toString();};

            /**
             * Serializes the BigInt into a hexadecimal std::string.
             * @return
             */
            std::string toHexString() const;

            /**
             * Serializes the BigInt into a Big Endian byte array.
             * @return
             */
            std::vector<uint8_t> toByteArray() const;

            BigInt operator+(const BigInt& rhs) const;
            BigInt operator-(const BigInt& rhs) const;
            BigInt operator*(const BigInt& rhs) const;
            BigInt operator/(const BigInt& rhs) const;
            BigInt operator%(const BigInt& rhs) const;

            BigInt& operator++();
            BigInt  operator++(int);
            BigInt& operator--();
            BigInt  operator--(int);

            void operator=(const BigInt&);

            bool operator<(const BigInt&) const;
            bool operator<=(const BigInt&) const;
            bool operator==(const BigInt&) const;
            bool operator>(const BigInt&) const;
            bool operator>=(const BigInt&) const;

            int compare(const BigInt&) const;

            BigInt powu(unsigned short p) const;

            unsigned long getBitSize() const;
            bool isNegative() const;
            bool isPositive() const;
            bool isZero() const;
            BigInt negative() const;
            BigInt positive() const;

            BigInt& assignI64(int64_t value);

            template <typename T>
            BigInt& assignScalar(T value) {
                auto bytes = endianness::scalar_type_to_array<T>(std::abs(value), endianness::Endianness::BIG);
                bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(bytes), sizeof(value));
                std::free(bytes);
                _negative = value < 0LL;
                return *this;
            }

            // API implementation
            std::shared_ptr<ledger::core::api::BigInt> add(std::shared_ptr<ledger::core::api::BigInt> const& i) override;
            std::shared_ptr<ledger::core::api::BigInt> subtract(std::shared_ptr<ledger::core::api::BigInt> const& i) override;
            std::shared_ptr<ledger::core::api::BigInt> multiply(std::shared_ptr<ledger::core::api::BigInt> const& i) override;
            std::shared_ptr<ledger::core::api::BigInt> divide(std::shared_ptr<ledger::core::api::BigInt> const& i) override;
            std::vector<std::shared_ptr<ledger::core::api::BigInt>> divideAndRemainder(std::shared_ptr<ledger::core::api::BigInt> const& i) override;

            std::shared_ptr<ledger::core::api::BigInt> pow(int32_t exponent) override;

            std::string toDecimalString(
                int32_t precision,
                std::string const& decimalSeparator,
                std::string const& thousandSeparator
            ) override;

            int32_t intValue() override;

            int32_t compare(std::shared_ptr<ledger::core::api::BigInt> const& i) override;

            std::string toString(int32_t radix) override;

        private:
            BigInt(const std::string& str, int radix);

            /// Check that all characters are digits.
            static bool all_digits(std::string const& s);

            BIGD _bigd;
            bool _negative;
        };
    }
}
