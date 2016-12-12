/*
 *
 * Base58
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/12/2016.
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
#include "Base58.hpp"
#include "BigInt.h"
#include <sstream>
#include "../utils/vector.hpp"
#include "../crypto/SHA256.hpp"
#include <functional>

static const std::string DIGITS = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const ledger::core::BigInt V_58(58);

static void _encode(const ledger::core::BigInt& v, std::stringstream& ss) {
    if (v == ledger::core::BigInt::ZERO) {
        return ;
    }
    auto r = (v % V_58).toUnsignedInt();
    _encode(v / V_58, ss);
    ss << DIGITS[r];
}

std::string ledger::core::Base58::encode(const std::vector<uint8_t> &bytes) {
    /*
     * var intData = BigInteger.ZERO
    for (i <- 0 until b.length) {
      intData = intData.multiply(BigInteger.valueOf(256)).add(BigInteger.valueOf(b(i) & 0xFF))
    }
    var result = ""
    while (intData.compareTo(BigInteger.ZERO) > 0) {
      val r = intData.mod(BigInteger.valueOf(58)).intValue()
      intData = intData.divide(BigInteger.valueOf(58))
      result = Digits(r).toString + result
    }
    "1" * b.takeWhile(_ == 0).length + result
     */
    BigInt intData(bytes.data(), bytes.size(), false);
    std::stringstream ss;

    for (auto i = 0; i < bytes.size() && bytes[i] == 0; i++) {
        ss << DIGITS[0];
    }
    _encode(intData, ss);
    return ss.str();
}

std::string ledger::core::Base58::encodeWithChecksum(const std::vector<uint8_t> &bytes) {
    return encode(vector::concat<uint8_t>(bytes, computeChecksum(bytes)));
}

std::vector<uint8_t> ledger::core::Base58::decode(const std::string &str) {
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ledger::core::Base58::computeChecksum(const std::vector<uint8_t> &bytes) {
    auto doubleHash = SHA256::bytesToBytesHash(SHA256::bytesToBytesHash(bytes));
    return std::vector<uint8_t>(doubleHash.begin(), doubleHash.begin() + 4);
}

/*
 val Digits = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"

  @throws(classOf[InvalidBase58FormatException])
  def decode(s: String): Array[Byte] = {
    var intData = BigInteger.ZERO
    for (i <- 0 until s.length) {
      val digit = Digits.indexOf(s(i))
      if (digit < 0)
        throw new InvalidBase58FormatException("Invalid character '" + digit + "'")
      intData = intData.multiply(BigInteger.valueOf(58)).add(BigInteger.valueOf(digit))
    }
    Array.fill[Byte](s.takeWhile(_ == '1').length)(0) ++ intData.toByteArray.dropWhile(_ == 0)
  }

  def tryDecode(s: String): Try[Array[Byte]] = Try(decode(s))

  def decodeWithChecksum(s: String): (Array[Byte], Array[Byte]) = {
    val decoded = decode(s)
    (decoded.slice(0, decoded.length - 4), decoded.slice(decoded.length - 4, decoded.length))
  }

  def checkAndDecode(s: String): Option[Array[Byte]] = {
    val (data, checksum) = decodeWithChecksum(s)
    val validChecksum = computeChecksum(data)
    if (validChecksum.deep == checksum.deep)
      Some(data)
    else
      None
  }

  def computeChecksum(b: Array[Byte]): Array[Byte] = {
    val digest = new SHA256Digest
    digest.update(b, 0, b.length)
    val firstPass = new Array[Byte](32)
    val secondPass = new Array[Byte](32)
    digest.doFinal(firstPass, 0)
    digest.reset()
    digest.update(firstPass, 0, firstPass.length)
    digest.doFinal(secondPass, 0)
    secondPass.slice(0, 4)
  }

  def encode(b: Array[Byte]): String = {
    var intData = BigInteger.ZERO
    for (i <- 0 until b.length) {
      intData = intData.multiply(BigInteger.valueOf(256)).add(BigInteger.valueOf(b(i) & 0xFF))
    }
    var result = ""
    while (intData.compareTo(BigInteger.ZERO) > 0) {
      val r = intData.mod(BigInteger.valueOf(58)).intValue()
      intData = intData.divide(BigInteger.valueOf(58))
      result = Digits(r).toString + result
    }
    "1" * b.takeWhile(_ == 0).length + result
  }

  def encodeWitchChecksum(b: Array[Byte]): String = encode(b ++ computeChecksum(b))
  def verify(s: String): Boolean = checkAndDecode(s).isDefined

  class InvalidBase58FormatException(reason: String) extends Exception(reason)
 */