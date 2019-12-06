/*
 *
 * tezosNetworks
 *
 * Created by El Khalil Bellakrid on 26/04/2019.
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


#include "tezosNetworks.h"
#include <utils/Exception.hpp>

namespace ledger {
    namespace core {
        namespace networks {
            const api::TezosLikeNetworkParameters getTezosLikeNetworkParameters(const std::string &networkName) {
                if (networkName == "tezos") {
                    static const api::TezosLikeNetworkParameters TEZOS(
                            "xtz",
                            "XTZ signed message:\n",
                            {0x04, 0x88, 0xB2, 0x1E},
                            {0x06, 0xA1, 0x9F},
                            {0x02, 0x5A, 0x79},
                            {},
                            0
                    );
                    return TEZOS;
                }
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "No network parameters set for {}", networkName);
            }

            const std::vector<api::TezosLikeNetworkParameters> ALL_TEZOS
                    ({
                             getTezosLikeNetworkParameters("tezos")
                     });
        }
    }
}