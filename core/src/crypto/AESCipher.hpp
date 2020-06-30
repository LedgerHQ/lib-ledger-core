/*
 *
 * AESCipher
 * ledger-core
 *
 * Created by Pierre Pollastri on 08/12/2016.
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

#pragma once

#include "../api/RandomNumberGenerator.hpp"
#include <memory>
#include <string>
#include <ostream>
#include <istream>
#include "../bytes/BytesReader.h"
#include "../bytes/BytesWriter.h"

namespace ledger {
 namespace core {
     class AESCipher {
     public:
         AESCipher(const std::shared_ptr<api::RandomNumberGenerator>& rng, const std::string& password, const std::string &salt, uint32_t iter);

         void encrypt(std::istream *input, std::ostream *output);
         void decrypt(std::istream *input, std::ostream *output) const ;

         void encrypt(BytesReader& input, BytesWriter& output);
         void decrypt(BytesReader& input, BytesWriter& output) const;

     private:
         std::shared_ptr<api::RandomNumberGenerator> _rng;
         std::vector<uint8_t> _key;
     };
 }
}
