/*
 *
 * StellarLikeAssetDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/07/2019.
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

#include "StellarLikeAssetDatabaseHelper.hpp"
#include <utils/hex.h>
#include <crypto/SHA256.hpp>
#include <fmt/format.h>
#include <database/soci-option.h>

using namespace soci;

namespace ledger {
    namespace core {


        std::string
        StellarLikeAssetDatabaseHelper::createAssetUid(const std::string &type, const Option<std::string> &code,
                                                       const Option<std::string> &issuer) {
            stellar::Asset asset;
            asset.type = type;
            asset.issuer = issuer.getValueOr("");
            asset.code = code.getValueOr("");
            return createAssetUid(asset);
        }

        bool StellarLikeAssetDatabaseHelper::putAsset(soci::session &sql, const std::string &type,
                                                      const Option<std::string> &code,
                                                      const Option<std::string> &issuer) {
            int count = 0;
            auto uid = createAssetUid(type, code, issuer);
            sql << "SELECT COUNT(*) FROM stellar_assets WHERE uid = :uid", use(uid), into(count);

            if (count == 0)  {
                sql << "INSERT INTO stellar_assets VALUES (:uid, :type, :code, :issuer)", use(uid), use(type),
                use(code), use(issuer);
            }

            return count == 0;
        }

        std::string StellarLikeAssetDatabaseHelper::createAssetUid(const stellar::Asset &asset) {
            return SHA256::stringToHexHash(fmt::format("{}::{}::{}", asset.type, asset.code, asset.issuer));
        }

        bool StellarLikeAssetDatabaseHelper::putAsset(soci::session &sql, const stellar::Asset &asset) {
            Option<std::string> code;
            Option<std::string> issuer;

            if (!asset.code.empty())
                code = asset.code;
            if (!asset.issuer.empty())
                issuer = asset.issuer;
            return putAsset(sql, asset.type, code, issuer);
        }


    }
}
