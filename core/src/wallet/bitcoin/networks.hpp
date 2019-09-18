/*
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
#ifndef LEDGER_CORE_NETWORKS_HPP
#define LEDGER_CORE_NETWORKS_HPP

#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER)
        #include <libcore_export.h>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

#include "../../api/BitcoinLikeNetworkParameters.hpp"

namespace ledger {
    namespace core {

        namespace networks {

            enum sigHashType : uint8_t {
                SIGHASH_ALL = 0x01,
                SIGHASH_NONE = 0x02,
                SIGHASH_SINGLE = 0x03,
                SIGHASH_FORKID = 0x40,
                SIGHASH_ANYONECANPAY = 0x80
            };
            
            // Since version for networks are starting at 1
            static const int HIGHTEST_PARAMETERS_VERSION = 2;

            extern LIBCORE_EXPORT const api::BitcoinLikeNetworkParameters getNetworkParameters(const std::string &networkName, int version = HIGHTEST_PARAMETERS_VERSION);

            // Since we are not (supposed) to have too many versions, we migrate from the beginning ...
            // TODO: could be optimized by saving last HIGHTEST_PARAMETERS_VERSION and start from it
            // or better add a field version to BitcoinLikeNetworkParameters
            template <int migration>
            void migrateParameters(api::BitcoinLikeNetworkParameters &params) {
                // Migration not implemented
                return;
            }
            void migrateParameters(api::BitcoinLikeNetworkParameters &params, int migration);

            static bool migrateParameters(api::BitcoinLikeNetworkParameters &params,
                                          int currentVersion,
                                          int targetVersion) {
                if (targetVersion < 2 || currentVersion < 1) {
                    return false;
                }
                auto previous = migrateParameters(params, currentVersion,  targetVersion - 1);
                if (currentVersion < targetVersion) {
                    migrateParameters(params, targetVersion);
                    return true;
                }
                return previous;
            };

            extern LIBCORE_EXPORT const std::vector<api::BitcoinLikeNetworkParameters> ALL;

            //BIP115 (ex: Zencash)
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


#endif //LEDGER_CORE_NETWORKS_HPP
