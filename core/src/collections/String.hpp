/*
 *
 * String
 * ledger-core
 *
 * Created by Pierre Pollastri on 09/03/2017.
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
#ifndef LEDGER_CORE_STRING_HPP
#define LEDGER_CORE_STRING_HPP

#include <string>
#include <sstream>

namespace ledger {
    namespace core {
        template <typename C, class Backend>
        class CharSequence {
        public:
            CharSequence() {

            }

            CharSequence(const C *base) : _backend(base) {

            }

            CharSequence(const C *base, size_t size) : _backend(base, size) {

            }

            CharSequence(const Backend base) : _backend(base) {

            }

            CharSequence<C, Backend> operator*(int number) {
                std::stringstream ss;
                for (auto i = 0; i < number; i++) {
                    ss << _backend;
                }
                return CharSequence<C, Backend>(ss.str());
            }

            CharSequence<C, Backend> operator+(const CharSequence<C, Backend>& str) {
                std::stringstream ss;
                ss << _backend << str._backend;
                return CharSequence<C, Backend>(ss.str());
            }

            std::stringstream& operator<<(std::stringstream& ss) {
                return ss << _backend;
            }

            std::string& str() {
                return _backend;
            }

            const std::string& str() const {
                return _backend;
            }

            const C* toCString() const {
                return _backend.c_str();
            };

            operator std::string() const {
                return _backend;
            };

            //friend std::ostream &operator<<(std::ostream &os, const CharSequence<C, Backend> &d);

        private:
            Backend _backend;
        };

        using String = CharSequence<char, std::string>;

        String operator "" _S(const char* str, size_t size);
    }
}

#endif //LEDGER_CORE_STRING_HPP
