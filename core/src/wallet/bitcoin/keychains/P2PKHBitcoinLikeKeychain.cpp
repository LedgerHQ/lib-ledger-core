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
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/set.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <fmt/format.h>
#include <src/api/DynamicObject.hpp>
#include "../../../api/BitcoinLikeAddress.hpp"
#include "../../../api/Configuration.hpp"
#include "../../../utils/DerivationPath.hpp"
#include "../../../collections/strings.hpp"
#include <debug/Benchmarker.h>

namespace ledger {
    namespace core {

        P2PKHBitcoinLikeKeychain::P2PKHBitcoinLikeKeychain(const std::shared_ptr<api::DynamicObject> &configuration,
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

            // Try to restore the state from preferences
            auto state = preferences->getData("state", {});
            if (!state.empty()) {
                boost::iostreams::array_source my_vec_source(reinterpret_cast<char*>(&state[0]), state.size());
                boost::iostreams::stream<boost::iostreams::array_source> is(my_vec_source);
                ::cereal::BinaryInputArchive archive(is);
                archive(_state);
            } else {
                _state.maxConsecutiveReceiveIndex = 0;
                _state.maxConsecutiveChangeIndex = 0;
                _state.empty = true;
            }
            _observableRange = (uint32_t) configuration->getInt(api::Configuration::KEYCHAIN_OBSERVABLE_RANGE).value_or(20);
            getAllObservableAddresses(0, _observableRange);
        }

        bool P2PKHBitcoinLikeKeychain::markPathAsUsed(const DerivationPath &p) {
            DerivationPath path(p);
            if (path.getParent().getLastChildNum() == 0) {
                if (path.getLastChildNum() < _state.maxConsecutiveReceiveIndex ||
                    _state.nonConsecutiveReceiveIndexes.find(path.getLastChildNum()) != _state.nonConsecutiveReceiveIndexes.end()) {
                    return false;
                } else {
                    if (path.getLastChildNum() == _state.maxConsecutiveReceiveIndex)
                        _state.maxConsecutiveReceiveIndex += 1;
                    else
                        _state.nonConsecutiveReceiveIndexes.insert(path.getLastChildNum());
                    _state.empty = false;
                    saveState();
                    return true;
                }
            } else {
                if (path.getLastChildNum() < _state.maxConsecutiveChangeIndex ||
                    _state.nonConsecutiveChangeIndexes.find(path.getLastChildNum()) != _state.nonConsecutiveChangeIndexes.end()) {
                    return false;
                } else {
                    if (path.getLastChildNum() == _state.maxConsecutiveChangeIndex)
                        _state.maxConsecutiveChangeIndex += 1;
                    else
                        _state.nonConsecutiveChangeIndexes.insert(path.getLastChildNum());
                    _state.empty = false;
                    saveState();
                    return true;
                }
            }
        }

        std::string P2PKHBitcoinLikeKeychain::getFreshAddress(BitcoinLikeKeychain::KeyPurpose purpose) {
            return derive(purpose, (purpose == KeyPurpose::RECEIVE ? _state.maxConsecutiveReceiveIndex : _state.maxConsecutiveChangeIndex));
        }

        bool P2PKHBitcoinLikeKeychain::isEmpty() const {
            return _state.empty;
        }

        std::vector<std::string> P2PKHBitcoinLikeKeychain::getAllObservableAddresses(uint32_t from, uint32_t to) {
            auto length = to - from;
            std::vector<std::string> result;
            result.reserve((length + 1) * 2);
            for (auto i = 0; i <= length; i++) {
                result.push_back(derive(KeyPurpose::RECEIVE, from + i));
                result.push_back(derive(KeyPurpose::CHANGE, from + i));
            }
            return result;
        }

        std::vector<std::string>
        P2PKHBitcoinLikeKeychain::getFreshAddresses(BitcoinLikeKeychain::KeyPurpose purpose, size_t n) {
            auto startOffset = (purpose == KeyPurpose::RECEIVE) ? _state.maxConsecutiveReceiveIndex : _state.maxConsecutiveChangeIndex;
            std::vector<std::string> result(n);
            for (auto i = 0; i < n; i++) {
                result.push_back(derive(purpose, startOffset + i));
            }
            return result;
        }

        Option<BitcoinLikeKeychain::KeyPurpose>
        P2PKHBitcoinLikeKeychain::getAddressPurpose(const std::string &address) const {
            return getAddressDerivationPath(address).flatMap<KeyPurpose>([] (const std::string& p) {
                DerivationPath derivation(p);
                if (derivation.getDepth() > 1) {
                    return Option<KeyPurpose>(derivation.getParent().getLastChildNum() == 0 ? KeyPurpose::RECEIVE : KeyPurpose::CHANGE);
                } else {
                    return Option<KeyPurpose>();
                }
            });
        }

        Option<std::string> P2PKHBitcoinLikeKeychain::getAddressDerivationPath(const std::string &address) const {
            auto path = getPreferences()->getString(fmt::format("address:{}", address), "");
            if (path.empty()) {
                return Option<std::string>();
            } else {
                auto derivation = DerivationPath(getExtendedPublicKey()->getRootPath()) + DerivationPath(path);
                return Option<std::string>(derivation.toString());
            }
        }

        std::vector<std::string>
        P2PKHBitcoinLikeKeychain::getAllObservableAddresses(BitcoinLikeKeychain::KeyPurpose purpose, uint32_t from,
                                                            uint32_t to) {
            auto maxObservableIndex = (purpose == KeyPurpose::CHANGE ? _state.maxConsecutiveChangeIndex + _state.nonConsecutiveChangeIndexes.size() : _state.maxConsecutiveReceiveIndex + _state.nonConsecutiveReceiveIndexes.size()) + _observableRange;
            auto length = std::min<size_t >(to - from, maxObservableIndex - from);
            std::vector<std::string> result(length +1);
            for (auto i = 0; i <= length; i++) {
                if (purpose == KeyPurpose::RECEIVE) {
                    result.push_back(derive(KeyPurpose::RECEIVE, from + i));
                } else {
                    result.push_back(derive(KeyPurpose::CHANGE, from + i));
                }
            }
            return result;
        }

        std::string P2PKHBitcoinLikeKeychain::derive(KeyPurpose purpose, off_t index) {
            auto iPurpose = (purpose == KeyPurpose::RECEIVE) ? 0 : 1;
            auto localPath = getDerivationScheme()
                                .setAccountIndex(getAccountIndex())
                                .setCoinType(getCurrency().bip44CoinType)
                                .setNode(iPurpose)
                                .setAddressIndex((int) index).getPath().toString();
            auto cacheKey = fmt::format("path:{}", localPath);
            auto address = getPreferences()->getString(cacheKey, "");
            if (address.empty()) {
                auto p = getDerivationScheme().getSchemeFrom(DerivationSchemeLevel::NODE).shift(1)
                        .setAccountIndex(getAccountIndex())
                        .setCoinType(getCurrency().bip44CoinType)
                        .setNode(iPurpose)
                        .setAddressIndex((int) index).getPath().toString();
                auto xpub = iPurpose == KeyPurpose::RECEIVE ? _publicNodeXpub : _internalNodeXpub;
                address = xpub->derive(p)->toBase58();
                // Feed path -> address cache
                // Feed address -> path cache
                getPreferences()
                        ->edit()
                        ->putString(cacheKey, address)
                        ->putString(fmt::format("address:{}", address), localPath)
                        ->commit();
            }
            return address;
        }

        void P2PKHBitcoinLikeKeychain::saveState() {
            while (_state.nonConsecutiveReceiveIndexes.find(_state.maxConsecutiveReceiveIndex) != _state.nonConsecutiveReceiveIndexes.end()) {
                _state.nonConsecutiveReceiveIndexes.erase(_state.maxConsecutiveReceiveIndex);
                _state.maxConsecutiveReceiveIndex += 1;
            }
            while (_state.nonConsecutiveChangeIndexes.find(_state.maxConsecutiveChangeIndex) != _state.nonConsecutiveChangeIndexes.end()) {
                _state.nonConsecutiveChangeIndexes.erase(_state.maxConsecutiveChangeIndex);
                _state.maxConsecutiveChangeIndex += 1;
            }
            std::stringstream is;
            ::cereal::BinaryOutputArchive archive(is);
            archive(_state);
            auto savedState = is.str();
            getPreferences()->edit()->putData("state", std::vector<uint8_t>((const uint8_t *)savedState.data(),(const uint8_t *)savedState.data() + savedState.size()))->commit();
        }

        std::shared_ptr<api::BitcoinLikeExtendedPublicKey> P2PKHBitcoinLikeKeychain::getExtendedPublicKey() const {
            return _xpub;
        }

        std::string P2PKHBitcoinLikeKeychain::getRestoreKey() const {
            return _xpub->toBase58();
        }

    }
}

CEREAL_CLASS_VERSION(ledger::core::P2PKHKeychainPersistentState, 0);