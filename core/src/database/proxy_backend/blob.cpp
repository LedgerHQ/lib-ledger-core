/*
 *
 * blob.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/12/2018.
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

#include "soci-proxy.h"

using namespace ledger::core;
using namespace soci;


std::size_t proxy_blob_backend::get_len() {
    return static_cast<size_t>(_backend->size());
}

std::size_t proxy_blob_backend::read(std::size_t offset, char *buf, std::size_t toRead) {
    std::vector<uint8_t> buffer = _backend->read(static_cast<int64_t>(offset), static_cast<int64_t>(toRead));
    auto it = buffer.begin();
    for (auto i = 0; i < toRead && it != buffer.end(); i += 1, it++) {
        buf[i] = *it;
    }
    return buffer.size();
}

std::size_t proxy_blob_backend::write(std::size_t offset, char const *buf, std::size_t toWrite) {
    const std::vector<uint8_t> data((uint8_t *)buf, (uint8_t *)(buf + toWrite));
    return static_cast<size_t>(_backend->write(static_cast<int64_t>(offset), data));
}

std::size_t proxy_blob_backend::append(char const *buf, std::size_t toWrite) {
    const std::vector<uint8_t> data((uint8_t *)buf, (uint8_t *)(buf + toWrite));
    return static_cast<size_t>(_backend->append(data));
}

void proxy_blob_backend::trim(std::size_t newLen) {
    _backend->trim(static_cast<int64_t>(newLen));
}
