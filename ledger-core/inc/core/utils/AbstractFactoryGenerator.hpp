/*
 *
 * AbstractFactoryGenerator
 * ledger-core
 *
 * Created by Alexis Le Provost on 16/09/2019.
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

#pragma once 

#include <memory>
#include <type_traits>
#include <unordered_map>

namespace ledger
{
namespace core
{

/**
 * The purpose of this class is to hold at runtime a bunch of abstract factories
 * and it offers a way to make them on the fly thanks to unique keys.
 */
template <typename Key, typename T>
class AbstractFactoryGenerator 
{
public:
    static_assert(
        std::is_base_of<std::enable_shared_from_this<T>, T>::value,
        "AbstractFactoryGenerator<Key, T> requires 'T' to be sharable."
    );

    using KeyType = std::decay_t<Key>;
    using ValueType = std::shared_ptr<T>;

    // Add - or replace if factory already exist - a new factory identifies by the key
    ValueType learn(KeyType const& key, ValueType const& factory);

    // Remove the factory identifies by the key
    void forget(KeyType const& key);

    // Find the factory identifies by the key and create a sharable instance of it
    ValueType make(KeyType const& key) const;

private:
    std::unordered_map<KeyType, ValueType> _factories;
};

} // namespace core
} // namespace ledger

#include <core/utils/AbstractFactoryGenerator.inl>
