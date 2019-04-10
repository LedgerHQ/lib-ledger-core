/*
 *
 * BitcoinLikeCommonKeychainFactory
 *
 * Created by El Khalil Bellakrid on 24/02/2019.
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


#ifndef LEDGER_CORE_BITCOINLIKECOMMONKEYCHAINFACTORY_H
#define LEDGER_CORE_BITCOINLIKECOMMONKEYCHAINFACTORY_H
namespace ledger {
    namespace core {
        template <class Keychain>
        class BitcoinLikeCommonKeychainFactory : public BitcoinLikeKeychainFactory {
        public:
            std::shared_ptr<ledger::core::BitcoinLikeKeychain>
            build(int32_t index,
                  const DerivationPath &path,
                  const std::shared_ptr<DynamicObject> &configuration,
                  const api::ExtendedKeyAccountCreationInfo& info,
                  const std::shared_ptr<Preferences> &accountPreferences, const api::Currency &currency) override {
                if (!info.extendedKeys.empty()) {
                    auto xpub = make_try<std::shared_ptr<BitcoinLikeExtendedPublicKey>>([&] () -> std::shared_ptr<BitcoinLikeExtendedPublicKey> {
                        return BitcoinLikeExtendedPublicKey::fromBase58(
                                currency,
                                info.extendedKeys[0],
                                Option<std::string>(path.toString()),
                                configuration
                        );
                    });
                    if (xpub.isFailure()) {
                        throw xpub.getFailure();
                    } else {
                        auto keychain = std::make_shared<Keychain>(
                                configuration, currency, index, xpub.getValue(), accountPreferences
                        );
                        return keychain;
                    }
                } else {
                    throw make_exception(api::ErrorCode::MISSING_DERIVATION, "Cannot find derivation {}", path.toString());
                }
            };

            std::shared_ptr<ledger::core::BitcoinLikeKeychain>
            restore(int32_t index,
                    const DerivationPath &path,
                    const std::shared_ptr<DynamicObject> &configuration, const std::string &databaseXpubEntry,
                    const std::shared_ptr<Preferences> &accountPreferences, const api::Currency &currency) override {
                auto keychain = std::make_shared<Keychain>(configuration,
                                                           currency,
                                                           index,
                                                           BitcoinLikeExtendedPublicKey::fromBase58(currency,
                                                                                                    databaseXpubEntry,
                                                                                                    Option<std::string>(path.toString()),
                                                                                                    configuration),
                                                           accountPreferences);
                return keychain;
            };
        };
    }
}
#endif //LEDGER_CORE_BITCOINLIKECOMMONKEYCHAINFACTORY_H
