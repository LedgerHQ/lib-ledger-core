/*
 *
 * BaseConverter.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 05/03/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#ifndef LEDGER_CORE_BASECONVERTER_HPP
#define LEDGER_CORE_BASECONVERTER_HPP

#include "BigInt.h"
#include <sstream>
#include <utils/Exception.hpp>
#include <collections/vector.hpp>
#include <algorithm>

namespace ledger {
    namespace core {
        /**
         * An helper class to encode/decode byte array to an arbitraty base.
         */
        class BaseConverter {
        public:

            /**
             * A function taking a character in parameter and mapping it to a character present in the encoder dictionary.
             */
            using CharNormalizer = std::function<char (char)>;
            /**
             * A function taking the number of missing bytes in a block and returning the padding to append to the encoding result.
             */
            using PaddingPolicy = std::function<void (int, std::stringstream&)>;

            /**
             * Parameters used by the algorithm to encode or decode a byte array
             * @tparam Base The base you want to use to run the encoding/decoding algorithm
             */
            template <int Base, int BlockBitSize, int ValueBitSize>
            struct Params {
                BigInt base;
                /**
                 * The dictionary of characters used to encode to the given base. Each character must be ordered depending
                 * on their underlying numeric value for example the decimal alphabet is "0123456789"
                 */
                char dictionnary[Base];

                /**
                 * Size in bits of each blocks of data to encode.
                 */
                int blockBitSize;

                /**
                 * Size in bits of each value in blocks.
                 */
                int valueBitSize;

                PaddingPolicy padder;


                CharNormalizer normalizeChar;

                Params(const char dict[Base], const CharNormalizer &normalizer, const PaddingPolicy& padding)
                    :   blockBitSize(BlockBitSize),
                        valueBitSize(ValueBitSize),
                        padder(padding),
                        normalizeChar(normalizer),
                        base(Base) {
                    memcpy(dictionnary, dict, Base);
                };
            };

            using Base32Params = Params<32, 40, 5>;


            static Base32Params BASE32_RFC4648;
            static Base32Params BASE32_RFC4648_NO_PADDING;

            /**
             * Decode the given string to byte array.
             * @tparam Base The base used to encode the given string.
             * @param string The string to decode.
             * @param params The parameters used to decode the given string.
             * @param The decoded version of the given string.
             */
            template <int Base, int BlockBitSize, int ValueBitSize>
            static void decode(const std::string& string, const Params<Base, BlockBitSize, ValueBitSize>& params, std::vector<uint8_t>& out) {
                const auto blockCharSize = BlockBitSize / ValueBitSize;
                const auto stringSize = string.size();

                for (auto index = 0; index < stringSize; index += blockCharSize) {
                    decodeBlock(string.c_str() + index, std::min(blockCharSize, (int)(stringSize - index)), params, out);
                }
            }

            /**
             * Encode the given byte array to the given base.
             * @tparam Base The used to encode the given data.
             * @param bytes The bytes to encode.
             * @param params The parameters used to encode the given bytes.
             * @return The encoded version of the given bytes.
             */
            template <int Base, int BlockBitSize, int ValueBitSize>
            static std::string encode(const std::vector<uint8_t>& bytes, const Params<Base, BlockBitSize, ValueBitSize>& params) {
                const auto blockSize = BlockBitSize / 8;
                uint8_t block[BlockBitSize / 8];
                std::stringstream ss;
                auto offset = 0;
                // Cut and encode input into blocks.
                for (const auto& byte : bytes) {
                    if (offset == blockSize) {
                        encodeBlock(block, offset, params, ss);
                        offset = 0;
                    }
                    block[offset] = byte;
                    offset += 1;
                }
                encodeBlock(block, offset, params, ss);
                return ss.str();
            }

        private:
            template <int Base, int BlockBitSize, int ValueBitSize>
            static void encodeBlock(const uint8_t* block, int size, const Params<Base, BlockBitSize, ValueBitSize>& params, std::stringstream& ss) {
                const auto blockSize = BlockBitSize / 8;
                const auto mask = (1 << ValueBitSize) - 1;
                int index = 0;
                auto bufferSize = 8; // Size of remaining untouched bit on the current offset
                auto offset = 0; // Index in the block

                // Split the block into values of ValueBitSize bits. We'll use this value as an index into our dictionary.

                for (auto remaining = size * 8; remaining > 0; remaining -= ValueBitSize) {
                    auto remainingBits = bufferSize - ValueBitSize;
                    if (remainingBits < 0) {
                        // For the sake of understand what we are doing we set remainingBits to a positive value
                        remainingBits = -remainingBits;
                        // Move the bits from the buffer to the index
                        auto bufferMask = (1 << bufferSize) - 1;
                        index = (block[offset] & bufferMask) << remainingBits;

                        // Now we advance the cursor
                        offset += 1;
                        bufferSize= 8 - remainingBits;

                        // If we don't overflow complete the index with missing bits
                        if (offset < size) {
                            auto missingBitsMask = (1 << remainingBits) - 1;
                            index = index | ((block[offset] >> bufferSize) & missingBitsMask);
                        }
                    } else {
                        bufferSize = remainingBits;
                        index = (block[offset] >> bufferSize) & mask;
                    }
                    ss << params.dictionnary[index];
                }

                // Add padding if the actual size of the block is below blockSize
                auto padding = blockSize - size;
                if (padding != 0)
                    params.padder(padding, ss);
            }

            template <int Base, int BlockBitSize, int ValueBitSize>
            static void decodeBlock(const char* str, int size, const Params<Base, BlockBitSize, ValueBitSize>& params, std::vector<uint8_t>& out) {
                int buffer = 0;
                int bufferSize = 0;
                int mask = (1 << ValueBitSize) - 1;
                auto pullFromBuffer = [&] () {
                    if (bufferSize >= 8) {
                        bufferSize = bufferSize - 8;
                        auto value = (uint8_t)((buffer >> bufferSize) & 0xFF);
                        out.push_back(value);
                        auto bufferMask = (1 << bufferSize) - 1;
                        buffer = buffer & bufferMask;
                    }
                };

                for (auto offset = 0; offset < size; offset++) {
                    auto value = getCharIndex(params.normalizeChar(str[offset]), params);
                    if (value == -1)
                        return;

                    buffer = (buffer << ValueBitSize) | (value & mask);
                    bufferSize += ValueBitSize;
                    pullFromBuffer();
                }
                pullFromBuffer();
            }

            template <int Base, int BlockBitSize, int ValueBitSize>
            static int getCharIndex(char c, const Params<Base, BlockBitSize, ValueBitSize>& params) {
                for (auto index = 0; index < Base; index++) {
                    if (params.dictionnary[index] == c)
                        return index;
                }
                return -1;
            }

        };
    }
}


#endif //LEDGER_CORE_BASECONVERTER_HPP
