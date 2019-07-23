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

namespace ledger {
    namespace core {
        namespace stellar {

            class XDREncoder;
            using WriteInstance = void (*)(void*, XDREncoder& encoder);

            struct XDRUnionInstance {
                int32_t discriminant;
                void *instance;
                WriteInstance write;

                XDRUnionInstance(int32_t d, void* instance, WriteInstance write);
            };

            class XDREncoder {
            public:

                template <class Object>
                void write(std::list<Object> list) {
                    _writer.writeBeValue<int32_t>(list.size());
                };

                void write(const XDRUnionInstance& instance);

                void write(int32_t i);
                void write(uint32_t i);

                void write(int64_t i);
                void write(uint64_t i);

                void writeString();

            private:
                BytesWriter _writer;
            };
        }
    }
}


#endif //LEDGER_CORE_XDRENCODER_HPP
