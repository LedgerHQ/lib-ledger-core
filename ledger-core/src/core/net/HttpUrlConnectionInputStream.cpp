/*
 *
 * HttpUrlConnectionInputStream
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/02/2017.
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

#include <core/api/HttpReadBodyResult.hpp>
#include <core/net/HttpUrlConnectionInputStream.hpp>
#include <core/utils/Exception.hpp>

namespace ledger {
    namespace core {
        HttpUrlConnectionInputStream::HttpUrlConnectionInputStream(
                const std::shared_ptr<api::HttpUrlConnection> &connection) {
            _connection = connection;
            _index = 0;
            _offset = 0;
            refill();
        }

        HttpUrlConnectionInputStream::Ch HttpUrlConnectionInputStream::Peek() {
            if (_index >= _buffer.size())
                return '\0';
            else
                return (Ch)_buffer[_index];
        }

        HttpUrlConnectionInputStream::Ch HttpUrlConnectionInputStream::Take() {
            if (_index >= _buffer.size())
                return '\0';
            else {
                auto c = (Ch) _buffer[_index];
                _index += 1;
                return c;
            }
        }

        size_t HttpUrlConnectionInputStream::Tell() const {
            return (size_t)(_index + _offset);
        }

        HttpUrlConnectionInputStream::Ch *HttpUrlConnectionInputStream::PutBegin() {
            throw Exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Cannot use this stream to write data");
            return nullptr;
        }

        void HttpUrlConnectionInputStream::Put(HttpUrlConnectionInputStream::Ch) {
            throw Exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Cannot use this stream to write data");
        }

        void HttpUrlConnectionInputStream::Flush() {
            throw Exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Cannot use this stream to write data");
        }

        size_t HttpUrlConnectionInputStream::PutEnd(HttpUrlConnectionInputStream::Ch *) {
            throw Exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Cannot use this stream to write data");
            return 0;
        }

        void HttpUrlConnectionInputStream::refill() {
            if (_index >= _buffer.size()) {
                auto result = _connection->readBody();
                if (result.error) {
                    throw Exception(result.error.value().code,
                                    result.error.value().message,
                                    std::static_pointer_cast<void>(_connection)
                    );
                }
                _index = 0;
                _offset += _buffer.size();
               _buffer = result.data.value();
            }
        }
    }
}
