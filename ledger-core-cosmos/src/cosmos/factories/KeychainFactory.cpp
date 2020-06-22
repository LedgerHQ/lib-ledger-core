/*
 *
 * CosmosLikeKeychainFactory
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <cosmos/ExtendedPublicKey.hpp>
#include <cosmos/factories/KeychainFactory.hpp>

namespace ledger {
namespace core {
namespace cosmos {

std::shared_ptr<CosmosLikeKeychain> CosmosLikeKeychainFactory::build(
    const DerivationPath &path,
    const std::shared_ptr<DynamicObject> &configuration,
    const api::AccountCreationInfo &info,
    const std::shared_ptr<Preferences> &accountPreferences,
    const api::Currency &currency) {
  // NOTE : The keychain is supposed to be linked to an account, but this check
  // makes the very strong assumption that the keychain is supposed have the
  // pubkey / path associated with one exact address (as the CosmosLikeKeychain
  // constructor later stores directly the hash160 of the pubkey as an _address
  // attribute) This is problematic because there is no way to have a proper [
  // scheme.toLevel(ACCOUNT_INDEX) || node/index ] separation in the Keychain ||
  // Address logic, which makes handling the paths very complicated.
  //
  // The decision here is to have the Keychain hold the complete derivation
  // path, and that the addresses it creates should have no "derivationPath".
  //
  // From ledger-live-common point of view, this means (in
  // libcore/buildAccount/index.js ) :
  //  - accountPath is "44'/118'/<account>'/0/0"
  //  - "path" component is None
  if (!info.publicKeys.empty() && !info.derivations.empty() &&
      info.derivations.front() == path.toString()) {
    const auto &pubKey = info.publicKeys.front();
    if (pubKey.size() != 33) {
      throw make_exception(api::ErrorCode::RUNTIME_ERROR,
                           "Public key should be compressed (33 "
                           "bytes) here {} bytes",
                           pubKey.size());
    }
    return std::make_shared<CosmosLikeKeychain>(pubKey, path, currency);
  } else {
    throw make_exception(api::ErrorCode::MISSING_DERIVATION,
                         "Cannot find derivation {}", path.toString());
  }
}

std::shared_ptr<CosmosLikeKeychain> CosmosLikeKeychainFactory::restore(
    const DerivationPath &path,
    const std::shared_ptr<DynamicObject> &configuration,
    const std::string &restoreKey,
    const std::shared_ptr<Preferences> &accountPreferences,
    const api::Currency &currency) {
  return CosmosLikeKeychain::restore(path, currency, restoreKey);
}

} // namespace cosmos
} // namespace core
} // namespace ledger
