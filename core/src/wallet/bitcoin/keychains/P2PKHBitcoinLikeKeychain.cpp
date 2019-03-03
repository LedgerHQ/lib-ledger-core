/*
 *
 * P2PKHBitcoinLikeKeychain
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/01/2017.
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
#include "P2PKHBitcoinLikeKeychain.hpp"
#include <api/KeychainEngines.hpp>
namespace ledger {
    namespace core {

        P2PKHBitcoinLikeKeychain::P2PKHBitcoinLikeKeychain(const std::shared_ptr<api::DynamicObject> &configuration,
                                                           const api::Currency &params,
                                                           int account,
                                                           const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
                                                           const std::shared_ptr<Preferences> &preferences)
                : CommonBitcoinLikeKeychains(configuration, params, account, xpub, preferences)
        {
            _keychainEngine = api::KeychainEngines::BIP32_P2PKH;
            getAllObservableAddresses(0, _observableRange);
        }

        int32_t P2PKHBitcoinLikeKeychain::getOutputSizeAsSignedTxInput() const {
            int32_t result = 0;
            //32 bytes of previous transaction hash
            result += 32;
            //4 bytes for output index (e.g. 0x000000)
            result += 4;
            //1 byte for scriptSig size (e.g. 0x8c)
            result += 1;
            //1 byte for length of (DER-encoded signature + hash code type) (e.g. 0x49)
            result += 1;
            //72 bytes of DER-signature
            result += 72;
            //1 byte for hash core type
            result += 1;
            //1 byte length of public key
            result += 1;
            //65 bytes for public key
            result += 65;
            //4 bytes for the sequence
            result +=4;
            return result;
        }
    }
}