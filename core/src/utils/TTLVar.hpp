/*
 *
 * TTLVar
 *
 * Created by Rémi Barjon on 05/02/2021
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Ledger
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

#include "Option.hpp"

#include <chrono>
#include <mutex>

namespace ledger {
namespace core {

template<typename T, typename Duration = std::chrono::seconds>
class TTLVar {
public:
    explicit TTLVar(T var, Duration ttl)
        : _m()
        , _var(std::move(var))
        , _updateTimeSinceEpoch(getNowDurationSinceEpoch())
        , _ttl(std::move(ttl))
    {}

    explicit TTLVar(Duration ttl)
        : _m()
        , _var()
        , _updateTimeSinceEpoch(Duration::zero())
        , _ttl(std::move(ttl))
    {}

    Option<T> get() const {
        std::lock_guard<std::mutex> lock(_m);
        if (getNowDurationSinceEpoch() - _updateTimeSinceEpoch >= _ttl) {
            _var = Option<T>();
        }
        return _var;
    }

    void update(T var) const {
        std::lock_guard<std::mutex> lock(_m);
        _var = Option<T>(var);
        _updateTimeSinceEpoch = getNowDurationSinceEpoch();
    }

private:
    static Duration getNowDurationSinceEpoch() {
        return std::chrono::duration_cast<Duration>(
            std::chrono::steady_clock::now().time_since_epoch());
    }

private:
    mutable std::mutex _m;
    mutable Option<T> _var;
    mutable Duration _updateTimeSinceEpoch;
    Duration _ttl;
};

} // namespace core
} // namespace ledger
