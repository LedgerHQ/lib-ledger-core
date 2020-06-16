#include <gtest/gtest.h>

#include <wallet/algorand/model/transactions/AlgorandSignedTransaction.hpp>
#include <wallet/algorand/AlgorandAddress.hpp>

#include <utils/hex.h>
#include <utils/Option.hpp>

#include <cstdint>
#include <string>

#include <fstream>

namespace ledger {
namespace core {
namespace algorand {
namespace model {
namespace test {

    namespace {

        constexpr auto SEND_ADDRESS = "F7ZFRLHWTOZJWHHBH2ROVW4DY6RXJJSPFJVFGE6DMM6JNKTLHFKKMKW4JU";
        constexpr auto RECV_ADDRESS = "O2PCLDMZQVZYUOC265KYCGGLWORPN4OUC4SJV5JJF4SHVTUSVMXKW32JJU";
        constexpr auto ASSET_NAME = "YRQTRENFFRG";
        constexpr auto ALGO_AMOUNT = uint64_t{ 10000 };
        constexpr auto ASSET_ID = uint64_t{ 9679401 };
        constexpr auto ASSET_AMOUNT = uint64_t{ 20 };

        Transaction::Header makeHeader(const std::string& type)
        {
            const auto note = std::string("frag ivn yrqtre");
            const auto header = Transaction::Header(
                    1000,
                    7334600,
                    std::string("testnet-v1.0"),
                    B64String("SGO1GKSzyE7IEPItTxCByw9x8FmnrCDexi9/cOUJOiI="),
                    {},
                    7335600,
                    {},
                    std::vector<uint8_t>(std::begin(note), std::end(note)),
                    Address(SEND_ADDRESS),
                    type);
            return header;
        }

    } // namespace

    TEST(TransactionSerialization, Payment)
    {
        const auto refBinary = std::string(
                "81a374786e8aa3616d74cd2710a3666565cd03e8a26676ce006feac8a"
                "367656eac746573746e65742d76312e30a26768c4204863b518a4b3c8"
                "4ec810f22d4f1081cb0f71f059a7ac20dec62f7f70e5093a22a26c76c"
                "e006feeb0a46e6f7465c40f667261672069766e20797271747265a372"
                "6376c420769e258d9985738a385af7558118cbb3a2f6f1d417249af52"
                "92f247ace92ab2ea3736e64c4202ff258acf69bb29b1ce13ea2eadb83"
                "c7a374a64f2a6a5313c3633c96aa6b3954a474797065a3706179");

        const auto header = makeHeader(constants::pay);
        const auto detailsAlgoTransfer = PaymentTxnFields(
                ALGO_AMOUNT,
                {},
                Address(RECV_ADDRESS));
        const auto tx = Transaction(header, detailsAlgoTransfer);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

    TEST(TransactionSerialization, AssetCreate)
    {
        const auto refBinary = std::string(
                "81a374786e89a46170617289a2616eab5952515452454e46465247a26"
                "175b775676763663a2f2f6a6a6a2e7972717472652e70627a2fa163c4"
                "202ff258acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313c3633c9"
                "6aa6b3954a2646302a166c4202ff258acf69bb29b1ce13ea2eadb83c7"
                "a374a64f2a6a5313c3633c96aa6b3954a16dc4202ff258acf69bb29b1"
                "ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa6b3954a172c4202f"
                "f258acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa6"
                "b3954a174ce05f5e100a2756ea3595154a3666565cd03e8a26676ce00"
                "6feac8a367656eac746573746e65742d76312e30a26768c4204863b51"
                "8a4b3c84ec810f22d4f1081cb0f71f059a7ac20dec62f7f70e5093a22"
                "a26c76ce006feeb0a46e6f7465c40f667261672069766e20797271747"
                "265a3736e64c4202ff258acf69bb29b1ce13ea2eadb83c7a374a64f2a"
                "6a5313c3633c96aa6b3954a474797065a461636667");

        const auto header = makeHeader(constants::acfg);
        const auto detailsAssetCreate =
            AssetConfigTxnFields::create(
                    AssetParams(
                        {},
                        std::string(ASSET_NAME),
                        std::string("uggcf://jjj.yrqtre.pbz/"),
                        Address(SEND_ADDRESS),
                        2,
                        false,
                        Address(SEND_ADDRESS),
                        Address(SEND_ADDRESS),
                        Address(SEND_ADDRESS),
                        100000000,
                        std::string("YQT")));
        const auto tx = Transaction(header, detailsAssetCreate);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

    TEST(TransactionSerialization, AssetConfig)
    {
        const auto refBinary = std::string(
                "81a374786e8aa46170617284a163c4202ff258acf69bb29b1ce13ea2e"
                "adb83c7a374a64f2a6a5313c3633c96aa6b3954a166c420769e258d99"
                "85738a385af7558118cbb3a2f6f1d417249af5292f247ace92ab2ea16"
                "dc4202ff258acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313c363"
                "3c96aa6b3954a172c4202ff258acf69bb29b1ce13ea2eadb83c7a374a"
                "64f2a6a5313c3633c96aa6b3954a463616964ce0093b229a3666565cd"
                "03e8a26676ce006feac8a367656eac746573746e65742d76312e30a26"
                "768c4204863b518a4b3c84ec810f22d4f1081cb0f71f059a7ac20dec6"
                "2f7f70e5093a22a26c76ce006feeb0a46e6f7465c40f6672616720697"
                "66e20797271747265a3736e64c4202ff258acf69bb29b1ce13ea2eadb"
                "83c7a374a64f2a6a5313c3633c96aa6b3954a474797065a461636667");

        const auto header = makeHeader(constants::acfg);
        const auto detailsAssetReconfigure =
            AssetConfigTxnFields::reconfigure(
                    AssetParams(
                        {},
                        {},
                        {},
                        Address(SEND_ADDRESS),
                        {},
                        {},
                        Address(RECV_ADDRESS),
                        Address(SEND_ADDRESS),
                        Address(SEND_ADDRESS),
                        {},
                        {}
                    ),
                    ASSET_ID);
        const auto tx = Transaction(header, detailsAssetReconfigure);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

    TEST(TransactionSerialization, AssetDestroy)
    {
        const auto refBinary = std::string(
                "81a374786e89a463616964ce0093b229a3666565cd03e8a26676ce00"
                "6feac8a367656eac746573746e65742d76312e30a26768c4204863b5"
                "18a4b3c84ec810f22d4f1081cb0f71f059a7ac20dec62f7f70e5093a"
                "22a26c76ce006feeb0a46e6f7465c40f667261672069766e20797271"
                "747265a3736e64c4202ff258acf69bb29b1ce13ea2eadb83c7a374a6"
                "4f2a6a5313c3633c96aa6b3954a474797065a461636667");

        const auto header = makeHeader(constants::acfg);
        const auto detailsAssetDestroy =
            AssetConfigTxnFields::destroy(
                    ASSET_ID);
        const auto tx = Transaction(header, detailsAssetDestroy);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

    TEST(TransactionSerialization, AssetOptIn)
    {
        const auto refBinary = std::string(
                "81a374786e8aa461726376c420769e258d9985738a385af7558118cb"
                "b3a2f6f1d417249af5292f247ace92ab2ea3666565cd03e8a26676ce"
                "006feac8a367656eac746573746e65742d76312e30a26768c4204863"
                "b518a4b3c84ec810f22d4f1081cb0f71f059a7ac20dec62f7f70e509"
                "3a22a26c76ce006feeb0a46e6f7465c40f667261672069766e207972"
                "71747265a3736e64c420769e258d9985738a385af7558118cbb3a2f6"
                "f1d417249af5292f247ace92ab2ea474797065a56178666572a47861"
                "6964ce0093b229");

        const auto header = [&]() {
            auto header = makeHeader(constants::axfer);
            header.sender = Address(RECV_ADDRESS);
            return header;
        }();
        const auto detailsAssetOptIn =
            AssetTransferTxnFields::optIn(
                    Address(RECV_ADDRESS),
                    ASSET_ID);
        const auto tx = Transaction(header, detailsAssetOptIn);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

    TEST(TransactionSerialization, AssetTransfer)
    {
        const auto refBinary = std::string(
                "81a374786e8ba461616d7414a461726376c420769e258d9985738a38"
                "5af7558118cbb3a2f6f1d417249af5292f247ace92ab2ea3666565cd"
                "03e8a26676ce006feac8a367656eac746573746e65742d76312e30a2"
                "6768c4204863b518a4b3c84ec810f22d4f1081cb0f71f059a7ac20de"
                "c62f7f70e5093a22a26c76ce006feeb0a46e6f7465c40f6672616720"
                "69766e20797271747265a3736e64c4202ff258acf69bb29b1ce13ea2"
                "eadb83c7a374a64f2a6a5313c3633c96aa6b3954a474797065a56178"
                "666572a478616964ce0093b229");

        const auto header = makeHeader(constants::axfer);
        const auto detailsAssetTransfer =
            AssetTransferTxnFields::transfer(
                    ASSET_AMOUNT,
                    {},
                    Address(RECV_ADDRESS),
                    ASSET_ID);
        const auto tx = Transaction(header, detailsAssetTransfer);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

    TEST(TransactionSerialization, AssetClawback)
    {
        const auto refBinary = std::string(
                "81a374786e8ca461616d7414a461726376c4202ff258acf69bb29b1ce"
                "13ea2eadb83c7a374a64f2a6a5313c3633c96aa6b3954a461736e64c4"
                "20769e258d9985738a385af7558118cbb3a2f6f1d417249af5292f247"
                "ace92ab2ea3666565cd03e8a26676ce006feac8a367656eac74657374"
                "6e65742d76312e30a26768c4204863b518a4b3c84ec810f22d4f1081c"
                "b0f71f059a7ac20dec62f7f70e5093a22a26c76ce006feeb0a46e6f74"
                "65c40f667261672069766e20797271747265a3736e64c4202ff258acf"
                "69bb29b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa6b3954a4"
                "74797065a56178666572a478616964ce0093b229");

        const auto header = makeHeader(constants::axfer);
        const auto detailsAssetClawback =
            AssetTransferTxnFields::clawback(
                    ASSET_AMOUNT,
                    {},
                    Address(SEND_ADDRESS),
                    Address(RECV_ADDRESS),
                    ASSET_ID);
        const auto tx = Transaction(header, detailsAssetClawback);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

    TEST(TransactionSerialization, AssetOptOut)
    {
        const auto refBinary = std::string(
                "81a374786e8ca461616d7414a661636c6f7365c4202ff258acf69bb2"
                "9b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa6b3954a46172"
                "6376c4202ff258acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313"
                "c3633c96aa6b3954a3666565cd03e8a26676ce006feac8a367656eac"
                "746573746e65742d76312e30a26768c4204863b518a4b3c84ec810f2"
                "2d4f1081cb0f71f059a7ac20dec62f7f70e5093a22a26c76ce006fee"
                "b0a46e6f7465c40f667261672069766e20797271747265a3736e64c4"
                "20769e258d9985738a385af7558118cbb3a2f6f1d417249af5292f24"
                "7ace92ab2ea474797065a56178666572a478616964ce0093b229");

        const auto header = [&]() {
            auto header = makeHeader(constants::axfer);
            header.sender = Address(RECV_ADDRESS);
            return header;
        }();
        const auto detailsAssetTransferClose =
            AssetTransferTxnFields::transfer(
                    ASSET_AMOUNT,
                    Address(SEND_ADDRESS),
                    Address(SEND_ADDRESS),
                    ASSET_ID);
        const auto tx = Transaction(header, detailsAssetTransferClose);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

    TEST(TransactionSerialization, CloseAccount)
    {
        const auto refBinary = std::string(
                "81a374786e8ba3616d74cd2710a5636c6f7365c4202ff258acf69bb2"
                "9b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa6b3954a36665"
                "65cd03e8a26676ce006feac8a367656eac746573746e65742d76312e"
                "30a26768c4204863b518a4b3c84ec810f22d4f1081cb0f71f059a7ac"
                "20dec62f7f70e5093a22a26c76ce006feeb0a46e6f7465c40f667261"
                "672069766e20797271747265a3726376c4202ff258acf69bb29b1ce1"
                "3ea2eadb83c7a374a64f2a6a5313c3633c96aa6b3954a3736e64c420"
                "769e258d9985738a385af7558118cbb3a2f6f1d417249af5292f247a"
                "ce92ab2ea474797065a3706179");

        const auto header = [&]() {
            auto header = makeHeader(constants::pay);
            header.sender = Address(RECV_ADDRESS);
            return header;
        }();
        const auto detailsCloseAccount = PaymentTxnFields(
                ALGO_AMOUNT,
                Address(SEND_ADDRESS),
                Address(SEND_ADDRESS));
        const auto tx = Transaction(header, detailsCloseAccount);
        const auto stx = SignedTransaction(tx);
        const auto binary = stx.serialize();

        EXPECT_EQ(refBinary, hex::toString(binary));
    }

} // namespace test
} // namespace model
} // namespace algorand
} // namespace core
} // namespace ledger

