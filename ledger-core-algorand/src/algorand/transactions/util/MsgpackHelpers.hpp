/*
 * MsgpackHelpers
 *
 * Created by Rémi Barjon on 04/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include <core/utils/Option.hpp>

#include <msgpack.hpp>

#include <cstdint>
#include <string>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

    using ledger::core::Option;

    template<typename T>
    struct KeyValue
    {
        KeyValue(std::string key, T value)
            : key(std::move(key))
            , value(std::forward<T>(value))
        {}

        std::string key;
        T value;
    };

    template<typename T>
    KeyValue<T> makeKeyValue(std::string key, T value)
    {
        return KeyValue<T>(std::move(key), value);
    }

    namespace impl {

        template<typename T>
        using IsBool =
            std::enable_if_t<std::is_same<std::decay_t<T>, bool>::value, bool>;

        template<typename T>
        using IsNotBool =
            std::enable_if_t<!std::is_same<std::decay_t<T>, bool>::value, bool>;

    } // namespace details

    /// The enable_if are there because algorand does not serialize a boolean if its
    /// value is false, making the implementation of this function different for bool...
    template<typename T, impl::IsNotBool<T> = true>
    uint32_t isValueValid(const T&)
    {
        return 1;
    }

    /// Specific implementation for boolean
    template<typename T, impl::IsBool<T> = true>
    uint32_t isValueValid(const T& value)
    {
        return value ? 1 : 0;
    }

    template<typename T>
    uint32_t isValueValid(const Option<T>& value)
    {
        if (value.hasValue()) {
            return isValueValid<T>(*value);
        }
        return 0;
    }

    template<typename Stream, typename T>
    void packKeyValue(packer<Stream>& o, KeyValue<T> keyvalue)
    {
        o.pack(keyvalue.key);
        o.pack(keyvalue.value);
    }

    template<typename Stream>
    void packKeyValue(packer<Stream>& o, KeyValue<bool> keyvalue)
    {
        if (keyvalue.value) {
            o.pack(keyvalue.key);
            o.pack(keyvalue.value);
        }
    }

    template<typename Stream, typename T>
    void packKeyValue(packer<Stream>& o, KeyValue<Option<T>>&& keyvalue)
    {
        if (keyvalue.value.hasValue()) {
            packKeyValue(o, makeKeyValue(keyvalue.key, *keyvalue.value));
        }
    }

    // FIXME(remibarjon): use fold expression (C++17)
    // template<typename... T>
    // uint32_t countValidValues(const T&... value)
    // {
    //     return (isValueValid(value) + ...);
    // }

    template<typename T>
    uint32_t countValidValues(T&& value)
    {
        return isValueValid(std::forward<T>(value));
    }

    /// Returns the number of valid elements.
    /// All values are valid except empty Options.
    template<typename T, typename... Ts>
    uint32_t countValidValues(T&& value, Ts&&... values)
    {
        return isValueValid(std::forward<T>(value)) + countValidValues(std::forward<Ts>(values)...);
    }

    // FIXME(remibarjon): use fold expression (C++17)
    // template<typename Stream, typename... T>
    // packer<Stream>& packKeyValues(packer<Stream>& o, T&&... keyvalue)
    // {
    //     (packKeyValue(o, std::forward<T>(keyvalue)), ...);
    //
    //     return o;
    // }

    /// Pack all the keyvalues filtering out the invalid ones
    template<typename Stream, typename... T>
    packer<Stream>& packKeyValues(packer<Stream>& o, T&&... keyvalue)
    {
        using _t = std::array<int, sizeof...(T)>;
        (void)_t { (packKeyValue(o, std::forward<T>(keyvalue)), 0)... };

        return o;
    }

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

