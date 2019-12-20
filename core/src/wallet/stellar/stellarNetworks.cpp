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


#include "stellarNetworks.h"
#include <utils/Exception.hpp>

namespace ledger {
    namespace core {
        namespace networks {
            const api::StellarLikeNetworkParameters getStellarLikeNetworkParameters(const std::string &networkName) {
                if (networkName == "stellar") {
                    static const api::StellarLikeNetworkParameters STELLAR(
                        "xlm", {6 << 3}, 5000000, 100, {}, "Public Global Stellar Network ; September 2015"
                    );
                    return STELLAR;
                }
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "No network parameters set for {}", networkName);
            }

            const std::vector<api::StellarLikeNetworkParameters> ALL_STELLAR
                    ({
                             getStellarLikeNetworkParameters("stellar")
                     });
        }
    }
}