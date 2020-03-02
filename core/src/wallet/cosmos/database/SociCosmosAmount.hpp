/*
 *
 * SociCosmosAmount.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 02/12/2019.
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

#include <type-conversion-traits.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>

#include <wallet/cosmos/cosmos.hpp>

namespace soci {

    template <typename Node, typename Allocator>
    inline void cosmos_coin_to_json_tuple(const ledger::core::cosmos::Coin& coin, Node& node, Allocator& allocator) {
        using namespace rapidjson;
        node.PushBack(Value().SetString(coin.amount.data(), allocator).Move(), allocator);
        node.PushBack(Value().SetString(coin.denom.data(), allocator).Move(), allocator);
    }

    template <typename Node>
    void cosmos_coin_from_json_tuple(const Node& node, ledger::core::cosmos::Coin& out) {
        out.amount = node[0].GetString();
        out.denom = node[1].GetString();
    }

    template <>
    struct type_conversion<ledger::core::cosmos::Coin> {
        typedef std::string base_type;
        static void from_base(base_type const & in, indicator ind, ledger::core::cosmos::Coin & out) {
            using namespace rapidjson;

            Document d;
            d.Parse(in.data());
            const auto& tuple = d.GetArray();
            cosmos_coin_from_json_tuple(tuple, out);
        }

        static void to_base(ledger::core::cosmos::Coin const & in, base_type & out, indicator & ind) {
            using namespace rapidjson;
            Document d;
            auto& allocator = d.GetAllocator();

            auto& tuple = d.SetArray();
            cosmos_coin_to_json_tuple(in, tuple, allocator);

            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            d.Accept(writer);
            out = buffer.GetString();
        }

    };

    template <>
    struct type_conversion<ledger::core::cosmos::Fee> {
        typedef std::string base_type;
        static void from_base(base_type const& in, indicator ind, ledger::core::cosmos::Fee & out) {
            using namespace rapidjson;
            using namespace ledger::core;

            Document d;
            d.Parse(in.data());
            const auto& obj = d.GetObject();
            out.gas = BigInt::fromString(obj["gas"].GetString());
            const auto& array = obj["amount"].GetArray();
            out.amount.resize(array.Size());
            auto index = 0;
            for (const auto& item : array) {
                const auto& tuple = item.GetArray();
                cosmos_coin_from_json_tuple(tuple, out.amount[index]);
                index += 1;
            }
        }

        static void to_base(ledger::core::cosmos::Fee const & in, base_type & out, indicator & ind) {
            using namespace rapidjson;
            Document d;
            auto& allocator = d.GetAllocator();

            auto& obj = d.SetObject();
            auto gas = in.gas.toString();
            obj["gas"].SetString(gas.data(), gas.size());
            auto& array = obj["amount"].SetObject();
            for (const auto& item : in.amount) {
                Value tuple(kArrayType);
                cosmos_coin_to_json_tuple(item, tuple, allocator);
                array.PushBack(tuple, allocator);
            }
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            d.Accept(writer);
            out = buffer.GetString();
        }
    };

    std::string coinToString(const ledger::core::cosmos::Coin &coin);
    void stringToCoin(const std::string &str, ledger::core::cosmos::Coin &out);

    std::string coinsToString(const std::vector<ledger::core::cosmos::Coin> &coins);
    void stringToCoins(const std::string &str, std::vector<ledger::core::cosmos::Coin> &out);
}
