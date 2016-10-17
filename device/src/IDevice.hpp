/*
 *
 * IDevice
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/09/2016.
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
#ifndef LEDGER_CORE_IDEVICE_H
#define LEDGER_CORE_IDEVICE_H

#include <vector>
#include <ledger/core/async/Callback.hpp>
#include <ledger/core/async/EventEmitter.hpp>
#include <ledger/core/utils/json.hpp>

using json = nlohmann::json;

namespace ledger {
    namespace device {

        enum TransportType {
            HID = 1,
            BLUETOOTH = 2,
            TEE = 3,
            LEGACY_USB = 4,
            UNDEFINED
        };

        class IDevice {
            public:
            virtual void exchange(std::vector<uint8_t> data, const ledger::core::Callback<void>& callback) = 0;
            virtual void connect(const ledger::core::Callback<void>& callback) = 0;
            virtual void disconnect(const ledger::core::Callback<void>& callback) = 0;
            virtual long getIdentifier() const = 0;
            virtual json getDeviceInformation() const = 0;
            virtual TransportType getTransportType() const = 0;
            virtual ledger::core::EventEmitter* getEventEmitter() = 0;
        };

    }
}


#endif //LEDGER_CORE_IDEVICE_H
