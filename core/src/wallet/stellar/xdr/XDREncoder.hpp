/*
 *
 * XDREncoder.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 23/07/2019.
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

#ifndef LEDGER_CORE_XDRENCODER_HPP
#define LEDGER_CORE_XDRENCODER_HPP

#include <bytes/BytesWriter.h>
#include <list>
#include <boost/variant/variant.hpp>
#include <array>
#include <utils/Option.hpp>

namespace ledger {
    namespace core {
        namespace stellar {
            namespace xdr {
                class Encoder;

                using ObjectEncoder = std::function<void(Encoder &)>;

                /**
                * Create an encoder for the given object. This function must have a
                * specialization for each types to encode.
                * @tparam T The class of the object to encode
                * @param object The object to encode
                * @return A function able to encode the object
                */
                template <class T>
                ObjectEncoder make_encoder(const T& object);

                class Encoder {
                public:

                    template<class Object>
                    Encoder& operator<<(const std::list<Object> &list) {
                        _writer.writeBeValue<int32_t>(list.size());
                        for (const auto &item : list) {
                            *this << item;
                        }
                        return *this;
                    };

                    template<class Object>
                    Encoder& operator<<(const std::vector<Object> &list) {
                        _writer.writeBeValue<int32_t>(list.size());
                        for (const auto &item : list) {
                            *this << item;
                        }
                        return *this;
                    };

                    template<class Object, std::size_t N>
                    Encoder& operator<<(const std::array<Object, N> &list) {
                        for (const auto &item : list) {
                            *this << item;
                        }
                        return *this;
                    };

                    template<class Object>
                    Encoder& operator<<(const Object& object) {
                        make_encoder(object)(*this);
                        return *this;
                    }

                    template<class Object>
                    Encoder& operator<<(const Option<Object> &option) {
                        *this << option.nonEmpty();
                        if (option.nonEmpty()) {
                            *this << option.getValue();
                        }
                        return *this;
                    };

                    Encoder& operator<<(const ObjectEncoder &w);

                    Encoder& operator<<(int32_t i);

                    Encoder& operator<<(uint32_t i);

                    Encoder& operator<<(int64_t i);

                    Encoder& operator<<(uint64_t i);

                    Encoder& operator<<(bool b);
                    Encoder& operator<<(uint8_t byte);

                    Encoder& operator<<(const std::string &str);

                    Encoder& operator<<(const std::vector<uint8_t> &bytes);

                    std::vector<uint8_t> toByteArray() const;

                private:
                    BytesWriter _writer;
                };

            }
        }
    }
}


#endif //LEDGER_CORE_XDRENCODER_HPP
