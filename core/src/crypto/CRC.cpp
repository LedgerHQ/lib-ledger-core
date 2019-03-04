/*
 *
 * CRC.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/02/2019.
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

#include "CRC.hpp"
#include <CRC.h>
#include <mutex>

namespace ledger {
    namespace core {

        struct CRC::CRC16Profile {
            CRC16Profile(const ::CRC::Parameters<uint16_t, 16>& params) : _params(params) {}
            ::CRC::Table<uint16_t, 16>& getTable() {
                std::lock_guard<std::mutex> lock(_mutex);
                if (_table.isEmpty())
                    _table = _params.MakeTable();
                return _table.getValue();
            };

        private:
            ::CRC::Parameters<uint16_t, 16> _params;
            Option<::CRC::Table<uint16_t, 16>> _table;
            std::mutex _mutex;
        };

        CRC::CRC16Profile CRC::XMODEM {::CRC::CRC_16_XMODEM()};

        uint16_t CRC::calculate(const std::vector<uint8_t> &bytes, CRC16Profile& profile) {
            return ::CRC::Calculate(bytes.data(), bytes.size(), profile.getTable());
        }
    }
}