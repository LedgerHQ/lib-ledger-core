/*
 *
 * BaseFixture.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/09/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include <utils/FilesystemUtils.hpp>

#include <core/utils/Hex.hpp>
#include <core/wallet/WalletDatabaseEntry.hpp>
#include <core/wallet/WalletDatabaseHelper.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>

#include "BaseFixture.hpp"
#include "IntegrationEnvironment.hpp"

void BaseFixture::SetUp() {
    ::testing::Test::SetUp();

    ledger::qt::FilesystemUtils::clearFs(IntegrationEnvironment::getInstance()->getApplicationDirPath());
    dispatcher = std::make_shared<QtThreadDispatcher>();
    resolver = std::make_shared<NativePathResolver>(IntegrationEnvironment::getInstance()->getApplicationDirPath());
    backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    http = std::make_shared<QtHttpClient>(dispatcher->getMainExecutionContext());
    ws = std::make_shared<FakeWebSocketClient>();
    rng = std::make_shared<OpenSSLRandomNumberGenerator>();
}

void BaseFixture::TearDown() {
    ::testing::Test::TearDown();

    resolver->clean();
}

std::shared_ptr<Services> BaseFixture::newDefaultServices(
    const std::string &tenant,
    const std::string &password
) {
    return Services::newInstance(
        tenant,
        password,
        http,
        ws,
        resolver,
        printer,
        dispatcher,
        rng,
        backend,
        api::DynamicObject::newInstance()
    );
}

std::shared_ptr<WalletStore> BaseFixture::newWalletStore(
    const std::shared_ptr<Services>& services
) {
    return std::make_shared<WalletStore>(services);
}

void BaseFixture::createWalletInDatabase(
    const std::shared_ptr<Services> &services,
    const std::string &walletName,
    const std::string &currencyName,
    const std::shared_ptr<api::DynamicObject> &configuration
) {
    soci::session sql(services->getDatabaseSessionPool()->getPool());

    WalletDatabaseEntry entry;
    entry.configuration = std::static_pointer_cast<DynamicObject>(configuration);
    entry.name = walletName;
    entry.tenant = services->getTenant();
    entry.currencyName = currencyName;
    entry.updateUid();

    WalletDatabaseHelper::putWallet(sql, entry);
}

void BaseFixture::createAccountInDatabase(const std::shared_ptr<Services> &services,
    const std::string &walletName,
    int32_t index
) {
    soci::session sql(services->getDatabaseSessionPool()->getPool());

    auto walletUid = WalletDatabaseEntry::createWalletUid(services->getTenant(), walletName);

    if (!AccountDatabaseHelper::accountExists(sql, walletUid, index)) {
        AccountDatabaseHelper::createAccount(sql, walletUid, index);
    }
}
