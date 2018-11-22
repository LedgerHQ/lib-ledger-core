\/*
 *
 * networks
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/01/2017.
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
#pragma once

#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER) && _MSC_VER <= 1900
        #include <libcore_export.h>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

#include "../../api/BitcoinLikeNetworkParameters.hpp"

namespace ledger {
    namespace core {

        namespace networks {
            extern LIBCORE_EXPORT const api::BitcoinLikeNetworkParameters getNetworkParameters(const std::string &networkName);
            extern LIBCORE_EXPORT const std::vector<api::BitcoinLikeNetworkParameters> ALL;

            //BIP115 (ex: Zencash)
            //extern LIBCORE_EXPORT const std::string BIP115;
            struct BIP115Parameters {
                std::string blockHash;
                std::vector<uint8_t> blockHeight;
            };
            extern LIBCORE_EXPORT const BIP115Parameters BIP115_PARAMETERS;

            //ZIP: Zcash improvements/updates (ex: Zcash overwinter, Sapling ...)
            struct ZIPParameters {
                uint32_t version;
                std::vector<uint8_t> overwinterFlag;
                std::vector<uint8_t> versionGroupId;
                uint64_t blockHeight; //block height at which ZIP will be effective
            };
            extern LIBCORE_EXPORT const ZIPParameters ZIP143_PARAMETERS;
            extern LIBCORE_EXPORT const ZIPParameters ZIP_SAPLING_PARAMETERS;
            template<class Archive>
            void serialize(Archive & archive,
                           api::BitcoinLikeNetworkParameters & p)
            {
                archive(
                    p.Identifier,
                    p.P2PKHVersion,
                    p.P2SHVersion,
                    p.XPUBVersion,
                    p.FeePolicy,
                    p.DustAmount,
                    p.MessagePrefix,
                    p.UsesTimestampedTransaction,
                    p.TimestampDelay,
                    p.SigHash,
                    p.AdditionalBIPs
                );
            }

        }
    }
}
