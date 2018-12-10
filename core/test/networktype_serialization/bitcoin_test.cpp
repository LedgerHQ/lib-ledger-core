#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <wallet/NetworkTypes.hpp>
#include <vector>
#include <cereal/cereal/archives/binary.hpp>

using namespace testing;

namespace ledger {
    namespace core {
        namespace bitcoin {
            bool operator==(const bitcoin::Block& a, const bitcoin::Block& b) {
                return (a.hash == b.hash) && (a.height == b.height) && (a.createdAt == b.createdAt);
            }

            bool operator==(const bitcoin::Input& a, const bitcoin::Input& b) {
                return
                    (a.address == b.address)
                    && (a.coinbase == b.coinbase)
                    && (a.index == b.index)
                    && (a.previousTxHash == b.previousTxHash)
                    && (a.previousTxOutputIndex == b.previousTxOutputIndex)
                    && (a.sequence == b.sequence)
                    && (a.signatureScript == b.signatureScript)
                    && (a.value == b.value);
            }

            bool operator==(const bitcoin::Output& a, const bitcoin::Output& b) {
                return
                    (a.address == b.address)
                    && (a.index == b.index)
                    && (a.script == b.script)
                    && (a.time == b.time)
                    && (a.transactionHash == b.transactionHash)
                    && (a.value == b.value);
            }

            bool operator==(const bitcoin::Transaction& a, const bitcoin::Transaction& b) {
                return
                    (a.block == b.block)
                    && (a.confirmations == b.confirmations)
                    && (a.fees == b.fees)
                    && (a.hash == b.hash)
                    && (a.inputs == b.inputs)
                    && (a.outputs == b.outputs)
                    && (a.receivedAt == b.receivedAt)
                    && (a.version == b.version);
            }
        };
        namespace tests {

            bitcoin::Block createBlock() {
                bitcoin::Block b;
                b.hash = "SOME HASH HERE";
                b.height = 123456;
                b.createdAt = std::chrono::system_clock::now();
                return b;
            }

            bitcoin::Input createInput() {
                bitcoin::Input inp;
                inp.address = "1 rue du mail :)";
                inp.coinbase = Option<std::string>();
                inp.index = 1;
                inp.previousTxHash = "SOME HASH HERE";
                inp.previousTxOutputIndex = 2;
                inp.sequence = 1231;
                inp.signatureScript = Option<std::string>::NONE;
                inp.value = BigInt(1231123);
                return inp;
            }

            bitcoin::Output createOutput() {
                bitcoin::Output out;
                out.address = "3akfjskdfskljf";
                out.index = 0;
                out.script = "sfsdfsflskdflk";
                out.time = "12:45";
                out.transactionHash = "SOME HASH";
                out.value = BigInt(1000);
                return out;
            }

            bitcoin::Transaction createTransaction() {
                bitcoin::Transaction tran;
                tran.block = Option<bitcoin::Block>::NONE;
                tran.confirmations = 199;
                tran.fees = BigInt(22);
                tran.hash = "OSDKALKFJKSJF";
                tran.inputs = { createInput(), createInput() };
                tran.outputs = { createOutput(), createOutput() };
                tran.receivedAt = std::chrono::system_clock::now();
                tran.version = 1;
                return tran;
            }

            template<typename T>
            T twoWay(const T& someValue) {
                std::stringstream ss;
                {
                    cereal::BinaryOutputArchive oarchive(ss);
                    oarchive(someValue);
                }
                T result;
                {
                    cereal::BinaryInputArchive iarchive(ss);
                    iarchive(result);
                }
                return result;
            }

            template<typename T>
            void check(const T& v) {
                EXPECT_EQ(v, twoWay(v));
            }

            TEST(BitcoinSerialization, Block) {
                check(createBlock());
            }

            TEST(BitcoinSerialization, Input) {
                check(createInput());
            }

            TEST(BitcoinSerialization, Output) {
                check(createOutput());
            }

            TEST(BitcoinSerialization, Transaction) {
                check(createTransaction());
            }
        }
    }
}