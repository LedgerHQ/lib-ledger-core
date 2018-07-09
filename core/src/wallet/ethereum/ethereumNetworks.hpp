/*
 *
 * networks
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 08/07/2018.
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
#ifndef LEDGER_CORE_ETH_NETWORKS_HPP
#define LEDGER_CORE_ETH_NETWORKS_HPP

#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER) && _MSC_VER <= 1900
        #include <libcore_export.h>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

#include "../../api/EthereumLikeNetworkParameters.hpp"

namespace ledger {
    namespace core {

        namespace networks {
            extern LIBCORE_EXPORT const api::EthereumLikeNetworkParameters getEthLikeNetworkParameters(const std::string &networkName);
            extern LIBCORE_EXPORT const std::vector<api::EthereumLikeNetworkParameters> ALL_ETH;

            template<class Archive>
            void serialize(Archive & archive,
                           api::EthereumLikeNetworkParameters & p)
            {
                archive(
                    p.Identifier,
                    p.MessagePrefix,
                    p.AdditionalEIPs
                );
            }

        }
    }
}


#endif //LEDGER_CORE_ETH_NETWORKS_HPP
