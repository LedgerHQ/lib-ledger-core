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

#include <map>
#include <memory>
#include <type_traits>

namespace ledger
{
namespace core
{

template <typename Key, typename T>
class AbstractFactoryGenerator 
{
public:
    static_assert(
        std::is_base_of<std::enable_shared_from_this<T>, T>::value,
        "AbstractFactoryGenerator<Key, T> requires 'T' to be sharable."
    );

    using key_type = std::decay_t<Key>;
    using value_type = std::shared_ptr<T>;

    AbstractFactoryGenerator() = default;
    ~AbstractFactoryGenerator() = default;

    value_type add(key_type const& key, value_type const& factory);
    void remove(key_type const& key);
    value_type make(key_type const& key) const;

private:
    std::map<key_type, value_type> factories_;
};

} // namespace core
} // namespace ledger

#include <core/utils/AbstractFactoryGenerator.inl>
