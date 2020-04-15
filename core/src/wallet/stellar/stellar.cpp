/*
 *
 * stellar.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 06/02/2020.
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

#include "stellar.hpp"
#include "StellarLikeAddress.hpp"

namespace ledger {
    namespace core {
        namespace stellar {

            static const std::string kAssetTypeNative("native");
            static const std::string kAssetTypeCreditAlphanum4("credit_alphanum4");
            static const std::string kAssetTypeCreditAlphanum12("credit_alphanum12");

            void xdrAssetToAsset(const xdr::Asset& asset, const api::StellarLikeNetworkParameters& params, stellar::Asset& out) {
                if (asset.type == xdr::AssetType::ASSET_TYPE_NATIVE) {
                    out.type = kAssetTypeNative;
                    return ;
                } else if (asset.type == xdr::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4){
                    const auto& code =  boost::get<xdr::AssetCode4>(asset.assetCode);
                    out.code = std::string(code.begin(), code.end());
                    out.type = kAssetTypeCreditAlphanum4;
                } else {
                    const auto& code =  boost::get<xdr::AssetCode4>(asset.assetCode);
                    out.code = std::string(code.begin(), code.end());
                    out.type = kAssetTypeCreditAlphanum4;
                }
                if (asset.issuer.nonEmpty()) {
                    out.issuer = StellarLikeAddress::convertXdrAccountToAddress(asset.issuer.getValue(), params);
                }
            }

        }
    }
}