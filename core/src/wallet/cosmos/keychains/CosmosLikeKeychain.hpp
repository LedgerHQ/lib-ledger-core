/*
 *
 * CosmosLikeKeychain
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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


#ifndef LEDGER_CORE_COSMOSLIKEKEYCHAIN_H
#define LEDGER_CORE_COSMOSLIKEKEYCHAIN_H

#include <string>
#include <vector>

#include <utils/DerivationPath.hpp>
#include <preferences/Preferences.hpp>
#include <api/Currency.hpp>

#include <cosmos/CosmosLikeAddress.hpp>

namespace ledger {
        namespace core {
                class CosmosLikeKeychain  {
                        public:
                                using Address = std::shared_ptr<CosmosLikeAddress>;

                                CosmosLikeKeychain(const std::vector<uint8_t>& pubKey,
                                                   const DerivationPath& path,
                                                   const api::Currency& currency);

                                Address getAddress() const;
                                bool contains(const std::string& address) const;
                                std::string getRestoreKey() const;
                                const std::vector<uint8_t>& getPublicKey() const;
                                std::vector<Address> getAllObservableAddresses(uint32_t from, uint32_t to);

                                static std::shared_ptr<CosmosLikeKeychain> restore(const DerivationPath& path, const api::Currency& currency, const std::string& restoreKey);
                        private:
                                std::vector<uint8_t> _pubKey;
                                Address _address;
                };
        }
}
#endif //LEDGER_CORE_COSMOSLIKEKEYCHAIN_H
