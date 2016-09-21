/*
 *
 * hex
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/09/2016.
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
#include "../../include/utils/hex.h"


namespace ledger {
    namespace core {
        namespace hex {

            std::vector<uint8_t> toByteArray(const std::string &str) {
                std::vector<uint8_t> bytes(str.length() / 2 + str.length() % 2);
                auto offset = str.length() % 2 != 0 ? 1 : 0;
                uint8_t byte = 0;
                for (auto index = 0; index < bytes.size(); index++) {
                    if (index == 0 && str.length() % 2 != 0) {
                        byte = digitToByte(str[0]);
                    } else {
                        byte = digitToByte(str[index * 2 - offset]) << 4;
                        byte = byte + (digitToByte(str[index * 2 + 1 - offset]));
                    }
                    bytes[index] = byte;
                }
                return bytes;
            }

            std::string toString(const std::vector<uint8_t>& data) {
                return toString(data, false);
            }

            uint8_t digitToByte(char c) {
                if (c >= '0' && c <= '9') {
                    return (uint8_t)(c - '0');
                } else if (c >= 'a' && c <= 'f') {
                    return (uint8_t)(0x0A + c - 'a');
                } else if (c >= 'A' && c <= 'F') {
                    return (uint8_t)(0x0A + c - 'A');
                }
                throw std::invalid_argument("Invalid hex character");
            }

            std::string toString(const std::vector<uint8_t>& data, bool uppercase) {
                std::string str(data.size() * 2, '0');
                for (auto index = 0; index < data.size(); index++) {
                    str[index * 2] = byteToDigit(data[index] >> 4, uppercase);
                    str[index * 2 + 1] = byteToDigit((uint8_t) (data[index] & 0xF), uppercase);
                }
                return str;
            }

            char byteToDigit(uint8_t byte, bool uppercase) {
                byte = 0xF < byte ? 0xF : byte;
                if (byte < 0xA) {
                    return '0' + byte;
                } else if (uppercase) {
                    return (char)('A' + (byte - 0xA));
                } else {
                    return (char)('a' + (byte - 0xA));
                }
            }
        }
    }
}