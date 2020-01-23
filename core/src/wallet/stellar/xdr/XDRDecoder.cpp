/*
 *
 * XDRDecoder.cpp
 * ledger-core
 *
 * Created by Hakim Aammar on 13/01/2020.
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

#include "XDRDecoder.hpp"

using namespace ledger::core::stellar::xdr;

Decoder::Decoder(const std::vector<uint8_t> &data) :
_reader(data)
{}

Decoder& Decoder::operator>>(int32_t &i) {
    i = _reader.readNextBeInt();
    return *this;
}

Decoder& Decoder::operator>>(uint32_t &i) {
    i = _reader.readNextBeUint();
    return *this;
}

Decoder& Decoder::operator>>(int64_t &i) {
    i = _reader.readNextBeLong();
    return *this;
}

Decoder& Decoder::operator>>(uint64_t &i) {
    i = _reader.readNextBeUlong();
    return *this;
}

Decoder& Decoder::operator>>(std::string &str) {
    // FIXME Manage issue of https://github.com/pollastri-pierre/lib-ledger-core/commit/301f55beca4964e986fd3937650a62ef5e9c4a4d
    str = _reader.readNextVarString();
    return *this;
}

Decoder& Decoder::operator>>(ObjectDecoder &r) {
    r(*this);
    return *this;
}

Decoder& Decoder::operator>>(bool &b) {
    int32_t v;
    (*this) >> v;
    b = (v == 0) ? false : true;
    return *this;
}

Decoder& Decoder::operator>>(uint8_t &byte) {
    byte = _reader.readNextByte();
    return *this;
}


