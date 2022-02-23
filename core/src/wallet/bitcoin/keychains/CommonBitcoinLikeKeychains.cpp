/*
 *
 * CommonBitcoinLikeKeychain
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 05/05/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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
#include "CommonBitcoinLikeKeychains.hpp"
#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/set.hpp"
#include "boost/iostreams/device/array.hpp"
#include "boost/iostreams/stream.hpp"
#include "fmt/format.h"
#include "api/DynamicObject.hpp"
#include "api/Configuration.hpp"
#include "utils/DerivationPath.hpp"
#include "collections/strings.hpp"
#include "bitcoin/BitcoinLikeAddress.hpp"

#include <iostream>
#include <api/KeychainEngines.hpp>
#include <api/ConfigurationDefaults.hpp>

using namespace std;

namespace ledger {
    namespace core {

        CommonBitcoinLikeKeychains::CommonBitcoinLikeKeychains(const std::shared_ptr<api::DynamicObject> &configuration,
                                                               const api::Currency &params,
                                                               int account,
                                                               const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
                                                               const std::shared_ptr<Preferences> &preferences)
                : BitcoinLikeKeychain(configuration, params, account, preferences) {
            _xpub = xpub;

            {
                auto localPath = getDerivationScheme().getSchemeTo(DerivationSchemeLevel::NODE)
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(getCurrency().bip44CoinType)
                        .setNode(RECEIVE).getPath();
                _publicNodeXpub = std::static_pointer_cast<BitcoinLikeExtendedPublicKey>(_xpub)->derive(localPath);
            }
            {
                auto localPath = getDerivationScheme().getSchemeTo(DerivationSchemeLevel::NODE)
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(getCurrency().bip44CoinType)
                        .setNode(CHANGE).getPath();
                _internalNodeXpub = std::static_pointer_cast<BitcoinLikeExtendedPublicKey>(_xpub)->derive(localPath);
            }
            _observableRange = (uint32_t) configuration->getInt(api::Configuration::KEYCHAIN_OBSERVABLE_RANGE)
                    .value_or(api::ConfigurationDefaults::KEYCHAIN_DEFAULT_OBSERVABLE_RANGE);
        }

        KeychainPersistentState CommonBitcoinLikeKeychains::getState() const {
            // Try to get the state from preferences
            auto preferences = getPreferences();
            if(preferences == nullptr) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Preferences is null.");
            }
            KeychainPersistentState state;
            auto rawState = preferences->getData("state", {});
            if (!rawState.empty()) {
                
                boost::iostreams::array_source my_vec_source(reinterpret_cast<char*>(&rawState[0]), rawState.size());
                boost::iostreams::stream<boost::iostreams::array_source> is(my_vec_source);
                ::cereal::BinaryInputArchive archive(is);
                archive(state);
            } 
            return state;
        }

        std::string CommonBitcoinLikeKeychains::getKeychainEngine() const {
            return _keychainEngine;
        }

        bool CommonBitcoinLikeKeychains::markPathAsUsed(const DerivationPath &p, bool needExtendKeychain) {
            KeychainPersistentState state = getState();
            DerivationPath path(p);

            auto isToUpdate = [&path](const uint32_t& maxConsecutiveIndex, const std::set<uint32_t>& nonConsecutiveIndexes) {
                return path.getLastChildNum() >= maxConsecutiveIndex && nonConsecutiveIndexes.find(path.getLastChildNum()) == nonConsecutiveIndexes.end();
            };

            auto updateState = [&path](uint32_t& maxConsecutiveIndex, std::set<uint32_t>& nonConsecutiveIndexes, bool& empty) {
                if (path.getLastChildNum() == maxConsecutiveIndex)
                    maxConsecutiveIndex += 1;
                else
                    nonConsecutiveIndexes.insert(path.getLastChildNum());
                empty = false;
            };


            if (path.getParent().getLastChildNum() == 0) {

                if (!isToUpdate(state.maxConsecutiveReceiveIndex, state.nonConsecutiveReceiveIndexes)) {
                    return false;
                }

                updateState(state.maxConsecutiveReceiveIndex, state.nonConsecutiveReceiveIndexes, state.empty);
                saveState(std::move(state));
                if (needExtendKeychain) {
                    getAllObservableAddresses(path.getLastChildNum(), path.getLastChildNum() + _observableRange);
                }

            } else {

                if (!isToUpdate(state.maxConsecutiveChangeIndex, state.nonConsecutiveChangeIndexes)) {
                    return false;
                }

                updateState(state.maxConsecutiveChangeIndex, state.nonConsecutiveChangeIndexes, state.empty);
                saveState(std::move(state));
                if (needExtendKeychain) {
                    getAllObservableAddresses(path.getLastChildNum(), path.getLastChildNum() + _observableRange);
                }
            }

            return true;
        }

        //TODO the two following methods are similar. Merge them if possible
        std::vector<BitcoinLikeKeychain::Address> CommonBitcoinLikeKeychains::getAllObservableAddresses(uint32_t from, uint32_t to) {
            auto currency = getCurrency();
            std::vector<BitcoinLikeKeychain::Address> res;
            res.reserve((to - from + 1) * 2);
            auto preferencesEdit = getPreferences()->edit();
            bool hasDBChange = false;
            for (int iPurpose : {KeyPurpose::RECEIVE, KeyPurpose::CHANGE})
            {
                for (int index = from; index <= to; index++)
                {
                    auto localPath = getDerivationScheme()
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(currency.bip44CoinType)
                        .setNode(iPurpose).setAddressIndex(index).getPath().toString();
                    auto cacheKey = fmt::format("path:{}", localPath);
                    auto address = getPreferences()->getString(cacheKey, "");

                    if (address.empty()) {
                        auto xpub = iPurpose == KeyPurpose::RECEIVE ? _publicNodeXpub : _internalNodeXpub;
                        auto p = getDerivationScheme().getSchemeFrom(DerivationSchemeLevel::NODE).shift(1)
                            .setAccountIndex(getAccountIndex())
                            .setCoinType(currency.bip44CoinType)
                            .setNode(iPurpose).setAddressIndex(index).getPath().toString();
                        address = BitcoinLikeAddress::fromPublicKey(xpub, currency, p, _keychainEngine);
                        preferencesEdit = preferencesEdit->putString(cacheKey, address)->putString(fmt::format("address:{}", address), localPath);
                        hasDBChange = true;
                    }
                    res.emplace_back(std::dynamic_pointer_cast<BitcoinLikeAddress>(BitcoinLikeAddress::parse(address, getCurrency(), Option<std::string>(localPath))));
                }
            }
            if (hasDBChange) {
                preferencesEdit->commit();
            }
            return res;
        }

        std::vector<std::string> CommonBitcoinLikeKeychains::getAllObservableAddressString(uint32_t from, uint32_t to) {
            auto currency = getCurrency();
            std::vector<std::string> res;
            res.reserve((to - from + 1) * 2);
            auto preferencesEdit = getPreferences()->edit();
            bool hasDBChange = false;
            for (int index = from; index <= to; index++)
            {
                for (int iPurpose : {KeyPurpose::RECEIVE, KeyPurpose::CHANGE})
                {
                    auto localPath = getDerivationScheme()
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(currency.bip44CoinType)
                        .setNode(iPurpose).setAddressIndex(index).getPath().toString();
                    auto p = getDerivationScheme().getSchemeFrom(DerivationSchemeLevel::NODE).shift(1)
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(currency.bip44CoinType)
                        .setNode(iPurpose).setAddressIndex(index).getPath().toString();
                    auto cacheKey = fmt::format("path:{}", localPath);
                    auto address = getPreferences()->getString(cacheKey, "");

                    if (address.empty()) {
                        auto xpub = iPurpose == KeyPurpose::RECEIVE ? _publicNodeXpub : _internalNodeXpub;
                        address = BitcoinLikeAddress::fromPublicKey(xpub, currency, p, _keychainEngine);
                        preferencesEdit = preferencesEdit->putString(cacheKey, address)->putString(fmt::format("address:{}", address), localPath);
                        hasDBChange = true;
                    }
                    res.emplace_back(address);
                }
            }
            if (hasDBChange) {
                preferencesEdit->commit();
            }
            return res;
        }

        BitcoinLikeKeychain::Address CommonBitcoinLikeKeychains::getFreshAddress(BitcoinLikeKeychain::KeyPurpose purpose) {
            KeychainPersistentState state = getState();
            return derive(purpose, (purpose == KeyPurpose::RECEIVE ? state.maxConsecutiveReceiveIndex : state.maxConsecutiveChangeIndex));
        }

        bool CommonBitcoinLikeKeychains::isEmpty() const {
            return getState().empty;
        }

        std::vector<BitcoinLikeKeychain::Address>
        CommonBitcoinLikeKeychains::getFreshAddresses(BitcoinLikeKeychain::KeyPurpose purpose, size_t n) {
            KeychainPersistentState state = getState();
            auto startOffset = (purpose == KeyPurpose::RECEIVE) ? state.maxConsecutiveReceiveIndex : state.maxConsecutiveChangeIndex;
            std::vector<BitcoinLikeKeychain::Address> result(n);
            for (auto i = 0; i < n; i++) {
                result[i] = derive(purpose, startOffset + i);
            }
            return result;
        }

        Option<BitcoinLikeKeychain::KeyPurpose>
        CommonBitcoinLikeKeychains::getAddressPurpose(const std::string &address) const {
            return getAddressDerivationPath(address).flatMap<KeyPurpose>([] (const std::string& p) {
                DerivationPath derivation(p);
                if (derivation.getDepth() > 1) {
                    return Option<KeyPurpose>(derivation.getParent().getLastChildNum() == 0 ? KeyPurpose::RECEIVE : KeyPurpose::CHANGE);
                } else {
                    return Option<KeyPurpose>();
                }
            });
        }

        Option<std::string> CommonBitcoinLikeKeychains::getAddressDerivationPath(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                return Option<std::string>();
            } else {
                auto derivation = DerivationPath(getExtendedPublicKey()->getRootPath()) + DerivationPath(path);
                return Option<std::string>(derivation.toString());
            }
        }

        std::vector<BitcoinLikeKeychain::Address>
        CommonBitcoinLikeKeychains::getAllObservableAddresses(BitcoinLikeKeychain::KeyPurpose purpose, uint32_t from,
                                                            uint32_t to) {
            KeychainPersistentState state = getState();                                         
            auto maxObservableIndex = (purpose == KeyPurpose::CHANGE ? state.maxConsecutiveChangeIndex + state.nonConsecutiveChangeIndexes.size() : state.maxConsecutiveReceiveIndex + state.nonConsecutiveReceiveIndexes.size()) + _observableRange;
            auto length = std::min<size_t >(to - from, maxObservableIndex - from);
            std::vector<BitcoinLikeKeychain::Address> result;
            result.reserve(length + 1);
            for (auto i = 0; i <= length; i++) {
                if (purpose == KeyPurpose::RECEIVE) {
                    result.push_back(derive(KeyPurpose::RECEIVE, from + i));
                } else {
                    result.push_back(derive(KeyPurpose::CHANGE, from + i));
                }
            }
            return result;
        }

        void CommonBitcoinLikeKeychains::saveState(KeychainPersistentState state) const {
            while (state.nonConsecutiveReceiveIndexes.find(state.maxConsecutiveReceiveIndex) != state.nonConsecutiveReceiveIndexes.end()) {
                state.nonConsecutiveReceiveIndexes.erase(state.maxConsecutiveReceiveIndex);
                state.maxConsecutiveReceiveIndex += 1;
            }
            while (state.nonConsecutiveChangeIndexes.find(state.maxConsecutiveChangeIndex) != state.nonConsecutiveChangeIndexes.end()) {
                state.nonConsecutiveChangeIndexes.erase(state.maxConsecutiveChangeIndex);
                state.maxConsecutiveChangeIndex += 1;
            }
            std::stringstream is;
            ::cereal::BinaryOutputArchive archive(is);
            archive(state);
            auto savedState = is.str();
            getPreferences()->edit()->putData("state", std::vector<uint8_t>((const uint8_t *)savedState.data(),(const uint8_t *)savedState.data() + savedState.size()))->commit();
        }

        std::shared_ptr<api::BitcoinLikeExtendedPublicKey> CommonBitcoinLikeKeychains::getExtendedPublicKey() const {
            return _xpub;
        }

        std::string CommonBitcoinLikeKeychains::getRestoreKey() const {
            return _xpub->toBase58();
        }

        int32_t CommonBitcoinLikeKeychains::getObservableRangeSize() const {
            return _observableRange;
        }

        bool CommonBitcoinLikeKeychains::contains(const std::string &address) const {
            return getAddressDerivationPath(address).nonEmpty();
        }

        std::vector<BitcoinLikeKeychain::Address> CommonBitcoinLikeKeychains::getAllAddresses() {
            KeychainPersistentState state = getState();
            std::vector<BitcoinLikeKeychain::Address> addresses;
            addresses.reserve(state.maxConsecutiveChangeIndex + 1 + state.maxConsecutiveReceiveIndex + 1);

            auto fetchAddressesFrom = [&](auto const keyPurpose, auto const maxIndex) {
                  for (auto i = 0; i <= maxIndex; ++i) {
                      addresses.push_back(derive(keyPurpose, i));
                  }
            };

            fetchAddressesFrom(KeyPurpose::CHANGE, state.maxConsecutiveChangeIndex);
            fetchAddressesFrom(KeyPurpose::RECEIVE, state.maxConsecutiveReceiveIndex);

            return addresses;
        }

        Option<std::vector<uint8_t>> CommonBitcoinLikeKeychains::getPublicKey(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                Option<std::vector<uint8_t>>();
            }
            return Option<std::vector<uint8_t>>(_xpub->derivePublicKey(path));
        }

        BitcoinLikeKeychain::Address CommonBitcoinLikeKeychains::derive(KeyPurpose purpose, off_t index) {
            auto currency = getCurrency();
            auto iPurpose = (purpose == KeyPurpose::RECEIVE) ? 0 : 1;
            auto localPath = getDerivationScheme()
                    .setAccountIndex(getAccountIndex())
                    .setCoinType(currency.bip44CoinType)
                    .setNode(iPurpose)
                    .setAddressIndex((int) index).getPath().toString();

            auto cacheKey = fmt::format("path:{}", localPath);
            auto address = getPreferences()->getString(cacheKey, "");

            if (address.empty()) {

                auto p = getDerivationScheme().getSchemeFrom(DerivationSchemeLevel::NODE).shift(1)
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(currency.bip44CoinType)
                        .setNode(iPurpose)
                        .setAddressIndex((int) index).getPath().toString();
                auto xpub = iPurpose == KeyPurpose::RECEIVE ? _publicNodeXpub : _internalNodeXpub;
                address = BitcoinLikeAddress::fromPublicKey(xpub, currency, p, _keychainEngine);
                // Feed path -> address cache
                // Feed address -> path cache
                    getPreferences()
                        ->edit()
                        ->putString(cacheKey, address)
                        ->putString(fmt::format("address:{}", address), localPath)
                        ->commit();
            }
            return std::dynamic_pointer_cast<BitcoinLikeAddress>(BitcoinLikeAddress::parse(address, getCurrency(), Option<std::string>(localPath)));
        }
    }
}

CEREAL_CLASS_VERSION(ledger::core::KeychainPersistentState, 0);
