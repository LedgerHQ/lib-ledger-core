/*
 * AlgorandWallet
 *
 * Created by Hakim Aammar on 20/04/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "AlgorandWallet.hpp"
#include "AlgorandAccount.hpp"
#include "model/AlgorandAccount.hpp"
#include "database/AlgorandAccountDatabaseHelper.hpp"

#include <core/Services.hpp>
#include <api/ErrorCode.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>


namespace ledger {
namespace core {
namespace algorand {

    Wallet::Wallet(const std::string &name,
                   const api::Currency &currency,
                   const std::shared_ptr<Services> &services,
                   const std::shared_ptr<DynamicObject> &configuration,
                   const DerivationScheme &scheme,
                   const std::shared_ptr<BlockchainExplorer> &explorer,
                   const std::shared_ptr<BlockchainObserver> &observer,
                   const AccountSynchronizerFactory &synchronizerFactory)
        : AbstractWallet(name, currency, services, configuration, scheme)
    {
        _explorer = explorer;
        _observer = observer;
        _synchronizerFactory = synchronizerFactory;
    }

    bool Wallet::isSynchronizing() {
        return false;
    }

    std::shared_ptr<api::EventBus> Wallet::synchronize() {
        return nullptr;
    }

    FuturePtr<ledger::core::api::Account>
    Wallet::newAccountWithInfo(const api::AccountCreationInfo &info) {

        auto self = getSelf();
        return async<std::shared_ptr<api::Account>>([=] () -> std::shared_ptr<api::Account> {
            DerivationPath path(info.derivations[0]);
            if (info.publicKeys.size() < 1) {
                throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Missing pubkey in account creation info.");
            }
            soci::session sql(getDatabase()->getPool());
            {
                if (ledger::core::AccountDatabaseHelper::accountExists(sql, getWalletUid(), info.index)) {
                    throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "Account {} already exists", info.index);
                }

                soci::transaction tr(sql);

                if (ledger::core::AccountDatabaseHelper::accountExists(sql, self->getWalletUid(), info.index)) {
                    throw make_exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "Account {}, for wallet '{}', already exists", info.index, self->getWalletUid());
                }

                // FIXME Why separate calls? Can't be done in 1 call?
                ledger::core::AccountDatabaseHelper::createAccount(sql, self->getWalletUid(), info.index);
                algorand::AccountDatabaseHelper::createAccount(sql, self->getWalletUid(), info.index, hex::toString(info.publicKeys[0]));

                tr.commit();
            }
            // FIXME Should find a way to avoid calling createAccountUid() here
            return self->createAccountInstance(sql, ledger::core::AccountDatabaseHelper::createAccountUid(self->getWalletUid(), info.index));
        });
    }

    FuturePtr<ledger::core::api::Account>
    Wallet::newAccountWithExtendedKeyInfo(const api::ExtendedKeyAccountCreationInfo &info) {
        throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Algorand does not support account creation with extended key infos.");
    }

    Future<api::ExtendedKeyAccountCreationInfo>
    Wallet::getExtendedKeyAccountCreationInfo(int32_t accountIndex) {
        throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Algorand does not support account creation with extended key infos.");
    }

    Future<api::AccountCreationInfo> Wallet::getAccountCreationInfo(int32_t accountIndex) {
        auto scheme = getDerivationScheme();
        auto path = scheme.setCoinType(getCurrency().bip44CoinType).setAccountIndex(accountIndex).getPath();
        return Future<api::AccountCreationInfo>::successful(api::AccountCreationInfo{accountIndex, {"main"}, {path.toString()}, {}, {}});
    }

    std::shared_ptr<Wallet> Wallet::getSelf() {
        return std::dynamic_pointer_cast<Wallet>(shared_from_this());
    }

    std::shared_ptr<AbstractAccount>
    Wallet::createAccountInstance(soci::session &sql, const std::string &accountUid) {
        model::Account accountData;
        algorand::AccountDatabaseHelper::queryAccount(sql, accountUid, accountData);
        return std::make_shared<Account>(shared_from_this(),
                                         accountData.index,
                                         getCurrency(),
                                         hex::toByteArray(accountData.pubKeyHex));
    }

    bool Wallet::hasMultipleAddresses() const {
        return false;
    }

}
}
}
