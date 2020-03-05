/*
 *
 * CosmosLikeBech32ParametersHelpers
 *
 * Created by Gerry Agbobada on 2020/02/10
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

#include <api/ErrorCode.hpp>
#include <collections/strings.hpp>
#include <math/BigInt.h>
#include <math/bech32/Bech32Parameters.h>
#include <utils/Exception.hpp>
#include <utils/hex.h>
#include <api/CosmosBech32Type.hpp>


namespace ledger {
namespace core {
namespace cosmos {
using namespace Bech32Parameters;
static const Bech32Parameters::Bech32Struct getBech32Params(api::CosmosBech32Type type) {
    switch (type) {
        case api::CosmosBech32Type::PUBLIC_KEY:

            static const Bech32Struct COSMOS_PUB = {
                "cosmospub",
                "cosmospub",
                "1",
                6,
                {0x3b6a57b2ULL, 0x26508e6dULL, 0x1ea119faULL, 0x3d4233ddULL, 0x2a1462b3ULL},
                {0x00},
                {0x00}};
            return COSMOS_PUB;
        case api::CosmosBech32Type::PUBLIC_KEY_VAL:
            static const Bech32Parameters::Bech32Struct COSMOS_PUB_VAL = {
                "cosmosvaloperpub",
                "cosmosvaloperpub",
                "1",
                6,
                {0x3b6a57b2ULL, 0x26508e6dULL, 0x1ea119faULL, 0x3d4233ddULL, 0x2a1462b3ULL},
                {0x00},
                {0x00}};
            return COSMOS_PUB_VAL;
        case api::CosmosBech32Type::ADDRESS:
            static const Bech32Parameters::Bech32Struct COSMOS = {
                "cosmos",
                "cosmos",
                "1",
                6,
                {0x3b6a57b2ULL, 0x26508e6dULL, 0x1ea119faULL, 0x3d4233ddULL, 0x2a1462b3ULL},
                {0x01},
                {0x01}};
            return COSMOS;
        case api::CosmosBech32Type::ADDRESS_VAL:
            static const Bech32Parameters::Bech32Struct COSMOS_VAL = {
                "cosmosvaloper",
                "cosmosvaloper",
                "1",
                6,
                {0x3b6a57b2ULL, 0x26508e6dULL, 0x1ea119faULL, 0x3d4233ddULL, 0x2a1462b3ULL},
                {0x01},
                {0x01}};
            return COSMOS_VAL;
        default:
            throw make_exception(
                api::ErrorCode::INVALID_ARGUMENT, "No Bech32 parameters set for this Bech32 type");
    }
}

static const std::vector<Bech32Struct> ALL_BECH32_PARAMS(
    {getBech32Params(api::CosmosBech32Type::ADDRESS),
     getBech32Params(api::CosmosBech32Type::ADDRESS_VAL),
     getBech32Params(api::CosmosBech32Type::PUBLIC_KEY),
     getBech32Params(api::CosmosBech32Type::PUBLIC_KEY_VAL)});

static bool insertBech32Parameters(soci::session& sql, const Bech32Struct& params) {
    auto count = 0;
    sql << "SELECT COUNT(*) FROM bech32_parameters WHERE name = :name", soci::use(params.name),
        soci::into(count);
    if (count == 0) {
        std::stringstream generator;
        std::vector<std::string> strGenerator;
        std::string separator(",");
        for (auto& g : params.generator) {
            BigInt bigIntG(g);
            strGenerator.push_back(bigIntG.toString());
        }
        strings::join(strGenerator, generator, separator);
        auto P2WPKHVersion = hex::toString(params.P2WPKHVersion);
        auto P2WSHVersion = hex::toString(params.P2WSHVersion);
        auto generatorStr = generator.str();
        sql << "INSERT INTO bech32_parameters VALUES(:name, :hrp, :separator, :generator, "
               ":p2wpkh_version, :p2wsh_version)",
            soci::use(params.name), soci::use(params.hrp), soci::use(params.separator),
            soci::use(generatorStr), soci::use(P2WPKHVersion), soci::use(P2WSHVersion);
        return true;
    }
    return false;
}
}  // namespace cosmos
}  // namespace core
}  // namespace ledger
