/*
 *
 * BaseConverter.cpp
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

#include <core/math/BaseConverter.hpp>

namespace ledger {
    namespace core {

        static BaseConverter::CharNormalizer NO_OP_NORMALIZER = [] (char c) {
            return c;
        };

        static BaseConverter::CharNormalizer BASE32_RFC4648_NORMALIZER = [] (char c) {
            if (c == '0')
                return 'O';
            else if (c == '1')
                return 'I';
            return c;
        };

        static BaseConverter::PaddingPolicy BASE32_RFC4648_PADDER = [] (int padding, std::stringstream& ss) {
            switch (padding) {
                case 4: // 8bits
                    ss << "======";
                    break;
                case 3: // 16bits
                    ss << "====";
                    break;
                case 2: // 24bits
                    ss << "===";
                    break;
                case 1: // 32bits
                    ss << "=";
                    break;
            }
        };

        static BaseConverter::PaddingPolicy BASE64_RFC4648_PADDER = [] (int padding, std::stringstream& ss) {
            switch (padding) {
                case 2: // 8bits
                    ss << "==";
                    break;
                case 1: // 16bits
                    ss << "=";
                    break;
            }
        };

        static BaseConverter::PaddingPolicy NO_PADDING = [] (int padding, std::stringstream& ss) {};


        BaseConverter::Base32Params BaseConverter::BASE32_RFC4648 {
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567",
            BASE32_RFC4648_NORMALIZER,
            BASE32_RFC4648_PADDER
        };

        BaseConverter::Base32Params BaseConverter::BASE32_RFC4648_NO_PADDING {
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567",
                BASE32_RFC4648_NORMALIZER,
                NO_PADDING
        };

        BaseConverter::Base64Params BaseConverter::BASE64_RFC4648 {
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
                NO_OP_NORMALIZER,
                BASE64_RFC4648_PADDER
        };

    }
}
