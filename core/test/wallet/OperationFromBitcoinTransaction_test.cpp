#include <gtest/gtest.h>
#include <Mocks.hpp>
#include <wallet/Operation.h>
#include <wallet/bitcoin/Bitcoin.hpp>

using namespace ledger::core;
using namespace ledger::core::bitcoin;
using namespace testing;

void addInput(Transaction& tr, uint32_t amount, const std::string& address) {
    Input in;
    in.address = address;
    in.value = BigInt(amount);
    tr.inputs.push_back(in);
}

void addOutput(Transaction& tr, uint32_t amount, const std::string& address) {
    Output out;
    out.address = address;
    out.value = BigInt(amount);
    tr.outputs.push_back(out);
}

TEST(OperationFromTransaction, SimpleSend) {
    // my_address1 100 => not_my_address1 99
    auto addressRegistry = std::make_shared<NiceMock<tests::KeychainRegistryMock>>();
    Transaction tr;
    addInput(tr, 100, "my_address1");
    addOutput(tr, 99, "not_my_address1");
    EXPECT_CALL(*addressRegistry, containsAddress("my_address1")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("not_my_address1")).WillOnce(Return(false));
    auto op = createOperation(tr, addressRegistry);
    EXPECT_EQ(op.fees, BigInt(1));
    EXPECT_EQ(op.type , api::OperationType::SEND);
    EXPECT_EQ(op.amount.toInt(), 100);
}

TEST(OperationFromTransaction, SimpleReceive) {
    // not_my_address1 100 => my_address1 99
    auto addressRegistry = std::make_shared<NiceMock<tests::KeychainRegistryMock>>();
    Transaction tr;
    addInput(tr, 100, "not_my_address1");
    addOutput(tr, 99, "my_address1");
    EXPECT_CALL(*addressRegistry, containsAddress("my_address1")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("not_my_address1")).WillOnce(Return(false));
    auto op = createOperation(tr, addressRegistry);
    EXPECT_EQ(op.fees, BigInt(1));
    EXPECT_EQ(op.type, api::OperationType::RECEIVE);
    EXPECT_EQ(op.amount.toInt(), 99);
}

TEST(OperationFromTransaction, SendWithChange) {
    // my_address1 100 => not_my_address1 79
    //                    my_change_address 20  
    auto addressRegistry = std::make_shared<NiceMock<tests::KeychainRegistryMock>>();
    Transaction tr;
    addInput(tr, 100, "my_address1");
    addOutput(tr, 79, "not_my_address1");
    addOutput(tr, 20, "my_change_address");
    EXPECT_CALL(*addressRegistry, containsAddress("my_address1")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my_change_address")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("not_my_address1")).WillOnce(Return(false));
    auto op = createOperation(tr, addressRegistry);
    EXPECT_EQ(op.fees, BigInt(1));
    EXPECT_EQ(op.type, api::OperationType::SEND);
    EXPECT_EQ(op.amount.toInt(), 80);
}

TEST(OperationFromTransaction, SendFromMultipleUTXO) {
    // my1     100 => not_my1 500
    // my2     100 
    // my3     100 
    // my4     100 
    // my5     100 
    // my6     100 
    auto addressRegistry = std::make_shared<NiceMock<tests::KeychainRegistryMock>>();
    Transaction tr;
    addInput(tr, 100, "my1");
    addInput(tr, 100, "my2");
    addInput(tr, 100, "my3");
    addInput(tr, 100, "my4");
    addInput(tr, 100, "my5");
    addInput(tr, 100, "my6");
    addOutput(tr, 500, "not_my1");
    EXPECT_CALL(*addressRegistry, containsAddress("my1")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my2")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my3")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my4")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my5")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my6")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("not_my1")).WillOnce(Return(false));
    auto op = createOperation(tr, addressRegistry);
    EXPECT_EQ(op.fees, BigInt(100));
    EXPECT_EQ(op.type, api::OperationType::SEND);
    EXPECT_EQ(op.amount.toInt(), 600);
}

TEST(OperationFromTransaction, CombineMyUTXOs) {
    // my1     100 => my_new 599
    // my2     100 
    // my3     100 
    // my4     100 
    // my5     100 
    // my6     100 
    auto addressRegistry = std::make_shared<NiceMock<tests::KeychainRegistryMock>>();
    Transaction tr;
    addInput(tr, 100, "my1");
    addInput(tr, 100, "my2");
    addInput(tr, 100, "my3");
    addInput(tr, 100, "my4");
    addInput(tr, 100, "my5");
    addInput(tr, 100, "my6");
    addOutput(tr, 599, "my_new");
    EXPECT_CALL(*addressRegistry, containsAddress("my1")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my2")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my3")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my4")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my5")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my6")).WillOnce(Return(true));
    EXPECT_CALL(*addressRegistry, containsAddress("my_new")).WillOnce(Return(true));
    auto op = createOperation(tr, addressRegistry);
    EXPECT_EQ(op.fees, BigInt(1));
    EXPECT_EQ(op.type, api::OperationType::SEND);
    EXPECT_EQ(op.amount.toInt(), 1); // send 1 to fees
}