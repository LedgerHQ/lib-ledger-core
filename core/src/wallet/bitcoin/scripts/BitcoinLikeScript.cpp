/*
 *
 * BitcoinLikeScript.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/04/2018.
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

#include "BitcoinLikeScript.h"
#include <bytes/BytesReader.h>
#include <bytes/BytesWriter.h>
#include <utils/hex.h>

namespace ledger {
    namespace core {

        Try<BitcoinLikeScript> BitcoinLikeScript::parse(const std::vector<uint8_t> &script) {
            return Try<BitcoinLikeScript>::from([&] () -> BitcoinLikeScript {
                BytesReader reader(script);
                BitcoinLikeScript s;
                while (reader.hasNext()) {
                    auto byte = reader.readNextByte();
                    if (byte >= 0x01 && byte <= 0x4b) {
                        s << reader.read(byte);
                    } else if (byte == btccore::OP_PUSHDATA1) {
                        s << reader.read(reader.readNextByte());
                    } else if (byte == btccore::OP_PUSHDATA2) {
                        s << reader.read(reader.readNextBeBigInt(2).toUnsignedInt());
                    } else if (byte == btccore::OP_PUSHDATA4) {
                        s << reader.read(reader.readNextBeBigInt(4).toUnsignedInt());
                    } else {
                        s << (btccore::opcodetype) byte;
                    }
                }
                return s;
            });
        }

        BitcoinLikeScript &BitcoinLikeScript::operator<<(btccore::opcodetype op_code) {
            _chunks.push_back(BitcoinLikeScriptChunk(op_code));
            return *this;
        }

        BitcoinLikeScript &BitcoinLikeScript::operator<<(const std::vector<uint8_t> &bytes) {
            _chunks.push_back(BitcoinLikeScriptChunk(bytes));
            return *this;
        }

        std::string BitcoinLikeScript::toString() const {
            std::stringstream ss;
            bool first = true;
            for (auto& chunk : _chunks) {
                if (!first) {
                    ss << " ";
                } else {
                    first = false;
                }
                if (chunk.isBytes()) {
                    const auto& bytes = chunk.getBytes();
                    ss << "PUSHDATA(" << bytes.size() << ")[" << hex::toString(bytes) << "]";
                } else {
                    ss << btccore::GetOpName(chunk.getOpCode());
                }
            }
            return ss.str();
        }

        std::vector<uint8_t> BitcoinLikeScript::serialize() const {
            BytesWriter writer;
            for (auto& chunk : _chunks) {
                if (chunk.isBytes()) {
                    auto& bytes = chunk.getBytes();
                    auto size = bytes.size();
                    if (size <= 0x75) {
                        writer.writeByte((uint8_t) size).writeByteArray(bytes);
                    } else if (size <= 0xFF) {
                        writer.writeByte((uint8_t) size).writeByteArray(bytes);
                    } else if (size <= 0xFFFF) {
                        writer.writeLeValue<uint16_t>((uint16_t) size).writeByteArray(bytes);
                    } else {
                        writer.writeLeValue<uint32_t>((uint32_t) size).writeByteArray(bytes);
                    }
                } else {
                    writer.writeByte(chunk.getOpCode());
                }
            }
            return writer.toByteArray();
        }

        const std::list<BitcoinLikeScriptChunk> &BitcoinLikeScript::toList() const {
            return _chunks;
        }


        BitcoinLikeScriptChunk::BitcoinLikeScriptChunk(BitcoinLikeScriptOpCode op) : _value(op) {

        }

        BitcoinLikeScriptChunk::BitcoinLikeScriptChunk(const std::vector<uint8_t> &bytes) : _value(bytes) {

        }

        const std::vector<uint8_t> &BitcoinLikeScriptChunk::getBytes() const {
            return _value.getLeft();
        }

        bool BitcoinLikeScriptChunk::isBytes() const {
            return _value.isLeft();
        }

        BitcoinLikeScriptOpCode BitcoinLikeScriptChunk::getOpCode() const {
            return _value.getRight();
        }

        bool BitcoinLikeScriptChunk::isOpCode() const {
            return _value.isRight();
        }
    }
}