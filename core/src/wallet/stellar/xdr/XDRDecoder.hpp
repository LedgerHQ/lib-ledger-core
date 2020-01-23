/*
 *
 * XDRDecoder.hpp
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

#ifndef LEDGER_CORE_XDRDECODER_HPP
#define LEDGER_CORE_XDRDECODER_HPP

#include <bytes/BytesReader.h>
#include <list>
#include <boost/variant/variant.hpp>
#include <array>
#include <utils/Option.hpp>
#include <typeinfo>

namespace ledger {
    namespace core {
        namespace stellar {
            namespace xdr {
                class Decoder;

                using ObjectDecoder = std::function<void(Decoder &)>;

                /**
                * Create a decoder for the given object. This function must have a
                * specialization for each types to decode.
                * @tparam T The class of the object to decode
                * @param object The object to decode
                * @return A function able to decode the object
                */
                template <class T>
                ObjectDecoder make_decoder(T& object);

                class Decoder {

                public:

                    Decoder(const std::vector<uint8_t> &data);

                    template<class Object>
                    Decoder& operator>>(std::list<Object> &list) {
                        list.clear();
                        auto listLength = _reader.readNextBeInt();
                        for (int i=0 ; i<listLength ; i++) {
                            Object item;
                            *this >> item;
                            list.push_back(item);
                        }
                        return *this;
                    }

                    template<class Object>
                    Decoder& operator>>(std::vector<Object> &list) {
                        list.clear();
                        auto listLength = _reader.readNextBeInt();
                        list.reserve(listLength);
                        for (int i=0 ; i<listLength ; i++) {
                            Object item;
                            *this >> item;
                            list[i] = item;
                        }
                        return *this;
                    }

                    template<class Object, std::size_t N>
                    Decoder& operator>>(std::array<Object, N> &list) {
                        for (int i=0 ; i<N ; i++) {
                            Object item;
                            *this >> item;
                            list[i] = item;
                        }
                        return *this;
                    };

                    template<class Object>
                    Decoder& operator>>(Option<Object> &option) {
                        auto optionNonEmpty = false;
                        *this >> optionNonEmpty;
                        if (optionNonEmpty) {
                            Object value;
                            *this >> value;
                            option = value;
                        }
                        return *this;
                    };

                    template<class Object>
                    Decoder& operator>>(Object &object) {
                        make_decoder(object)(*this);
                        return *this;
                    }

                    Decoder& operator>>(ObjectDecoder &d);

                    Decoder& operator>>(int32_t &i);

                    Decoder& operator>>(uint32_t &i);

                    Decoder& operator>>(int64_t &i);

                    Decoder& operator>>(uint64_t &i);

                    Decoder& operator>>(bool &b);

                    Decoder& operator>>(uint8_t &byte);

                    Decoder& operator>>(std::string &str);


                private:

                    BytesReader _reader;
                };

            }
        }
    }
}


#endif //LEDGER_CORE_XDRDECODER_HPP
