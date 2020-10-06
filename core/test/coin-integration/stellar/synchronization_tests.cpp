/*
 *
 * synchronization_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 01/04/2019.
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

#include "StellarFixture.hpp"
#include <wallet/common/OperationQuery.h>
#include <math/BigInt.h>
#include <api/StellarLikeMemo.hpp>
#include <api/StellarLikeMemoType.hpp>

TEST_F(StellarFixture, SynchronizeStellarAccount) {
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet", "stellar", api::DynamicObject::newInstance());
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = newAccount(wallet, 0, defaultAccount());
    auto exists = uv::wait(account->exists());
    EXPECT_TRUE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       getTestExecutionContext()->stop();
                   }));
    EXPECT_EQ(bus, account->synchronize());
    getTestExecutionContext()->waitUntilStopped();
    auto balance = uv::wait(account->getBalance());
    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->addOrder(api::OperationOrderKey::DATE, false)->complete())->execute());
    EXPECT_TRUE(balance->toBigInt()->compare(api::BigInt::fromLong(0)) > 0);
    EXPECT_TRUE(operations.size() >= 5);

    for (const auto& op : operations) {
        auto record = op->asStellarLikeOperation()->getRecord();
        fmt::print("{} {} {} {}\n",   api::to_string(op->getOperationType()), op->getAmount()->toString(), op->getFees()->toString(), api::to_string(record.operationType));
        if (op->getOperationType() == api::OperationType::SEND) {
            EXPECT_TRUE(op->getFees()->toLong() >= 100);
        }
    }

    const auto& first = operations.front();

    EXPECT_EQ(first->getAmount()->toString(), "1800038671");
    EXPECT_EQ(first->getFees()->toString(), "100");
    EXPECT_EQ(first->getDate(), DateUtils::fromJSON("2019-03-14T10:08:27Z"));
    EXPECT_EQ(first->getSenders().front(), "GBV4ZDEPNQ2FKSPKGJP2YKDAIZWQ2XKRQD4V4ACH3TCTFY6KPY3OAVS7");
    EXPECT_EQ(first->getRecipients().front(), "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(first->isComplete(), true);
    EXPECT_EQ(first->getOperationType(), api::OperationType::RECEIVE);


    const auto& second = operations[1];

    EXPECT_EQ(second->getAmount()->toString(), "50000000");
    EXPECT_EQ(second->getFees()->toString(), "100");
    EXPECT_EQ(second->getDate(), DateUtils::fromJSON("2019-03-14T10:24:15Z"));
    EXPECT_EQ(second->getSenders().front(), "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3");
    EXPECT_EQ(second->getRecipients().front(), "GB6TMMOCZSFFVXUXPV6FATTGQN6NKV74I2LTBB6LR7GEWLTN2IGZ6L6X");
    EXPECT_EQ(second->isComplete(), true);
    EXPECT_EQ(second->getOperationType(), api::OperationType::SEND);

    auto reserve = uv::wait(account->getBaseReserve());
    EXPECT_TRUE(reserve->toLong() >= (2 * 5000000));

    auto txFound = false;
    for (auto& op : operations) {
        if (op->asStellarLikeOperation()->getRecord().transactionHash == "6c084cdf56ff11b47baeab9a17fe8dc66d8009b7f4f86101758c9e99348af9a3") {
            auto memo = op->asStellarLikeOperation()->getTransaction()->getMemo();
            EXPECT_EQ(memo->getMemoType(), api::StellarLikeMemoType::MEMO_TEXT);
            EXPECT_EQ(memo->getMemoText(), "Salut charlotte");
            txFound = true;
        }
    }
    EXPECT_TRUE(txFound);
}

TEST_F(StellarFixture, SynchronizeEmptyStellarAccount) {
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet", "stellar", api::DynamicObject::newInstance());
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = newAccount(wallet, 0, emptyAccount());

    auto exists = uv::wait(account->exists());
    EXPECT_FALSE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       getTestExecutionContext()->stop();
                   }));
    getTestExecutionContext()->waitUntilStopped();
    auto address = uv::wait(account->getFreshPublicAddresses())[0];
    auto balance = uv::wait(account->getBalance());
    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    EXPECT_TRUE(balance->toBigInt()->compare(api::BigInt::fromLong(0)) == 0);
    EXPECT_TRUE(operations.size() == 0);

    // Fetch the first send operation
    for (const auto& op: operations) {
        if (op->getOperationType() == api::OperationType::SEND) {
            const auto& sop = op->asStellarLikeOperation();
            ASSERT_EQ(sop->getTransaction()->getSourceAccount()->toString(), address->toString());
            ASSERT_TRUE(sop->getTransaction()->getFee()->toLong() > 0);
            auto sequence = std::dynamic_pointer_cast<api::BigIntImpl>(sop->getTransaction()->getSourceAccountSequence())->backend();
            ASSERT_TRUE(sequence > BigInt::ZERO);
        }
    }
}

TEST_F(StellarFixture, SynchronizeStellarAccountWithSubEntry) {
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet", "stellar", api::DynamicObject::newInstance());
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    StellarLikeAddress addr("GAT4LBXYJGJJJRSNK74NPFLO55CDDXSYVMQODSEAAH3M6EY4S7LPH5GV", getCurrency(), Option<std::string>::NONE);
    auto account = newAccount(wallet, 0, accountInfo(hex::toString(addr.toPublicKey())));
    auto exists = uv::wait(account->exists());
    EXPECT_TRUE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       getTestExecutionContext()->stop();
                   }));
    EXPECT_EQ(bus, account->synchronize());
    getTestExecutionContext()->waitUntilStopped();
    auto reserve = uv::wait(account->getBaseReserve());
    EXPECT_TRUE(reserve->toLong() > 2 * 5000000);
}

TEST_F(StellarFixture, SynchronizeStellarAccountWithManageBuyOffer) {
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet_1", "stellar", api::DynamicObject::newInstance());
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = newAccount(wallet, 0, accountInfoFromAddress("GDDU4HHNCSZ2BI6ELSSFKPSOBL2TEB4A3ZJWOCT2DILQKVJTZBNSOZA2"));
    auto exists = uv::wait(account->exists());
    EXPECT_TRUE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
            make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED);
        getTestExecutionContext()->stop();
    }));
    EXPECT_EQ(bus, account->synchronize());
    getTestExecutionContext()->waitUntilStopped();
    auto balance = uv::wait(account->getBalance());
    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->addOrder(api::OperationOrderKey::DATE, false)->complete())->execute());
    EXPECT_TRUE(balance->toBigInt()->compare(api::BigInt::fromLong(0)) > 0);
    EXPECT_TRUE(operations.size() >= 5);
}

TEST_F(StellarFixture, SynchronizeStellarAccountWithMultisig) {
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet_2", "stellar", api::DynamicObject::newInstance());
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = newAccount(wallet, 0, accountInfoFromAddress("GAJTWW4OGH5BWFTH24C7SGIDALKI2HUVC2LXHFD533A5FIMSXE5AB3TJ"));
    auto exists = uv::wait(account->exists());
    EXPECT_TRUE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       getTestExecutionContext()->stop();
                   }));
    EXPECT_EQ(bus, account->synchronize());
    getTestExecutionContext()->waitUntilStopped();
    auto signers = uv::wait(account->getSigners());
    EXPECT_EQ(signers.size(), 2);
    const auto& signer_1 = std::find_if(signers.begin(), signers.end(), [] (const stellar::AccountSigner& s) {
        return s.key == "GAJTWW4OGH5BWFTH24C7SGIDALKI2HUVC2LXHFD533A5FIMSXE5AB3TJ";
    });
    const auto& signer_2 = std::find_if(signers.begin(), signers.end(), [] (const stellar::AccountSigner& s) {
        return s.key == "GDDU4HHNCSZ2BI6ELSSFKPSOBL2TEB4A3ZJWOCT2DILQKVJTZBNSOZA2";
    });
    EXPECT_NE(signer_1, signers.end());
    EXPECT_NE(signer_2, signers.end());

    EXPECT_EQ(signer_1->type, "ed25519_public_key");
    EXPECT_EQ(signer_1->weight, 10);

    EXPECT_EQ(signer_2->type, "ed25519_public_key");
    EXPECT_EQ(signer_2->weight, 10);
}

// Synchronize an account with protocol 13 upgrade object
TEST_F(StellarFixture, SynchronizeProtocol13) {
    // GBV4NH4G5SWYM6OQJKZKG2PA2O2VQ2W6K5S43WLMLJRWU4XTG5EST5QP
    auto pool = newPool();
    auto wallet = newWallet(pool, "my_wallet_proto_13", "stellar", api::DynamicObject::newInstance());
    auto info = uv::wait(wallet->getNextAccountCreationInfo());
    auto account = newAccount(wallet, 0, accountInfoFromAddress("GBSEXVEU2WBBLIUCWIWFDPV5I4HLBSOWRNJVKLNXZFVNITFPKQIVO3YI"));
    auto exists = uv::wait(account->exists());
    EXPECT_TRUE(exists);
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(),
                   make_receiver([=](const std::shared_ptr<api::Event> &event) {
                       fmt::print("Received event {}\n", api::to_string(event->getCode()));
                       if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                           return;
                       EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                       EXPECT_EQ(event->getCode(),
                                 api::EventCode::SYNCHRONIZATION_SUCCEED);
                       getTestExecutionContext()->stop();
                   }));
    EXPECT_EQ(bus, account->synchronize());
    getTestExecutionContext()->waitUntilStopped();
    auto balance = uv::wait(account->getBalance());
    auto address = uv::wait(account->getFreshPublicAddresses()).front();
    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    EXPECT_GT(balance->toLong(), 0);
    EXPECT_TRUE(operations.size() >= 11);

    // Fetch the first send operation
    for (const auto& op: operations) {
        fmt::print("{} {} {}\n", api::to_string(op->getOperationType()), op->getAmount()->toString(), op->asStellarLikeOperation()->getRecord().transactionHash);
        if (op->getOperationType() == api::OperationType::SEND) {
            const auto& sop = op->asStellarLikeOperation();
            ASSERT_EQ(sop->getTransaction()->getSourceAccount()->toString(), address->toString());
            ASSERT_TRUE(sop->getTransaction()->getFee()->toLong() > 0);
            auto sequence = std::dynamic_pointer_cast<api::BigIntImpl>(sop->getTransaction()->getSourceAccountSequence())->backend();
            ASSERT_TRUE(sequence > BigInt::ZERO);
        }
    }
}