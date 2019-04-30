/*
 *
 * serialization
 * ledger-core
 *
 * Created by Pierre Pollastri on 01/02/2017.
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
#ifndef LEDGER_CORE_SERIALIZATION_HPP
#define LEDGER_CORE_SERIALIZATION_HPP

#include <vector>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/external/base64.hpp>
#include <sstream>

namespace ledger {
    namespace core {
        namespace serialization {

            template <typename T>
            void load(const std::vector<uint8_t>& src, T& dest) {
                boost::iostreams::array_source my_vec_source(reinterpret_cast<const char*>(&src[0]), src.size());
                boost::iostreams::stream<boost::iostreams::array_source> is(my_vec_source);
                ::cereal::BinaryInputArchive archive(is);
                archive(dest);
            }

            template <typename T>
            void save(T& src, std::vector<uint8_t>& dest) {
                std::stringstream is;
                ::cereal::BinaryOutputArchive archive(is);
                archive(src);
                auto raw = is.str();
                dest.assign((const uint8_t *)raw.data(), (const uint8_t *)raw.data() + raw.size());
            };

            template <typename T>
            void loadBase64(const std::string& str, T& dest) {
                auto decoded = cereal::base64::decode(str);
                std::vector<uint8_t> data;
                data.reserve(decoded.size());
                for (auto& c : decoded) {
                    data.push_back((uint8_t) c);
                }
                load(data, dest);
            }

            template <typename T>
            void saveBase64(T& src, std::string& dest) {
                std::vector<uint8_t> data;
                save(src, data);
                dest = cereal::base64::encode((const unsigned char *)data.data(), data.size());
            }

        }
    }
}


#endif //LEDGER_CORE_SERIALIZATION_HPP
