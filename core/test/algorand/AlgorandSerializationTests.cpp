#include <gtest/gtest.h>

#include <wallet/algorand/model/transactions/AlgorandSignedTransaction.hpp>
#include <wallet/algorand/AlgorandAddress.hpp>

#include <utils/hex.h>
#include <utils/Option.hpp>

#include <cstdint>
#include <string>

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
                "8aa3616d74cd2710a3666565cd03e8a26676ce006feac8a367656eac"
                "746573746e65742d76312e30a26768c4204863b518a4b3c84ec810f2"
                "2d4f1081cb0f71f059a7ac20dec62f7f70e5093a22a26c76ce006fee"
                "b0a46e6f7465c40f667261672069766e20797271747265a3726376c4"
                "20769e258d9985738a385af7558118cbb3a2f6f1d417249af5292f24"
                "7ace92ab2ea3736e64c4202ff258acf69bb29b1ce13ea2eadb83c7a3"
                "74a64f2a6a5313c3633c96aa6b3954a474797065a3706179");

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
                "89a46170617289a2616eab5952515452454e46465247a26175b77567"
                "6763663a2f2f6a6a6a2e7972717472652e70627a2fa163c4202ff258"
                "acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa6b39"
                "54a2646302a166c4202ff258acf69bb29b1ce13ea2eadb83c7a374a6"
                "4f2a6a5313c3633c96aa6b3954a16dc4202ff258acf69bb29b1ce13e"
                "a2eadb83c7a374a64f2a6a5313c3633c96aa6b3954a172c4202ff258"
                "acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa6b39"
                "54a174ce05f5e100a2756ea3595154a3666565cd03e8a26676ce006f"
                "eac8a367656eac746573746e65742d76312e30a26768c4204863b518"
                "a4b3c84ec810f22d4f1081cb0f71f059a7ac20dec62f7f70e5093a22"
                "a26c76ce006feeb0a46e6f7465c40f667261672069766e2079727174"
                "7265a3736e64c4202ff258acf69bb29b1ce13ea2eadb83c7a374a64f"
                "2a6a5313c3633c96aa6b3954a474797065a461636667");

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
                "8aa46170617284a163c4202ff258acf69bb29b1ce13ea2eadb83c7a3"
                "74a64f2a6a5313c3633c96aa6b3954a166c420769e258d9985738a38"
                "5af7558118cbb3a2f6f1d417249af5292f247ace92ab2ea16dc4202f"
                "f258acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa"
                "6b3954a172c4202ff258acf69bb29b1ce13ea2eadb83c7a374a64f2a"
                "6a5313c3633c96aa6b3954a463616964ce0093b229a3666565cd03e8"
                "a26676ce006feac8a367656eac746573746e65742d76312e30a26768"
                "c4204863b518a4b3c84ec810f22d4f1081cb0f71f059a7ac20dec62f"
                "7f70e5093a22a26c76ce006feeb0a46e6f7465c40f66726167206976"
                "6e20797271747265a3736e64c4202ff258acf69bb29b1ce13ea2eadb"
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
                "89a463616964ce0093b229a3666565cd03e8a26676ce006feac8a367"
                "656eac746573746e65742d76312e30a26768c4204863b518a4b3c84e"
                "c810f22d4f1081cb0f71f059a7ac20dec62f7f70e5093a22a26c76ce"
                "006feeb0a46e6f7465c40f667261672069766e20797271747265a373"
                "6e64c4202ff258acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313"
                "c3633c96aa6b3954a474797065a461636667");

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
                "8aa461726376c420769e258d9985738a385af7558118cbb3a2f6f1d4"
                "17249af5292f247ace92ab2ea3666565cd03e8a26676ce006feac8a3"
                "67656eac746573746e65742d76312e30a26768c4204863b518a4b3c8"
                "4ec810f22d4f1081cb0f71f059a7ac20dec62f7f70e5093a22a26c76"
                "ce006feeb0a46e6f7465c40f667261672069766e20797271747265a3"
                "736e64c420769e258d9985738a385af7558118cbb3a2f6f1d417249a"
                "f5292f247ace92ab2ea474797065a56178666572a478616964ce0093"
                "b229");

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
                "8ba461616d7414a461726376c420769e258d9985738a385af7558118"
                "cbb3a2f6f1d417249af5292f247ace92ab2ea3666565cd03e8a26676"
                "ce006feac8a367656eac746573746e65742d76312e30a26768c42048"
                "63b518a4b3c84ec810f22d4f1081cb0f71f059a7ac20dec62f7f70e5"
                "093a22a26c76ce006feeb0a46e6f7465c40f667261672069766e2079"
                "7271747265a3736e64c4202ff258acf69bb29b1ce13ea2eadb83c7a3"
                "74a64f2a6a5313c3633c96aa6b3954a474797065a56178666572a478"
                "616964ce0093b229");

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
                "8ca461616d7414a461726376c4202ff258acf69bb29b1ce13ea2eadb"
                "83c7a374a64f2a6a5313c3633c96aa6b3954a461736e64c420769e25"
                "8d9985738a385af7558118cbb3a2f6f1d417249af5292f247ace92ab"
                "2ea3666565cd03e8a26676ce006feac8a367656eac746573746e6574"
                "2d76312e30a26768c4204863b518a4b3c84ec810f22d4f1081cb0f71"
                "f059a7ac20dec62f7f70e5093a22a26c76ce006feeb0a46e6f7465c4"
                "0f667261672069766e20797271747265a3736e64c4202ff258acf69b"
                "b29b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa6b3954a474"
                "797065a56178666572a478616964ce0093b229");

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
                "8ca461616d7414a661636c6f7365c4202ff258acf69bb29b1ce13ea2"
                "eadb83c7a374a64f2a6a5313c3633c96aa6b3954a461726376c4202f"
                "f258acf69bb29b1ce13ea2eadb83c7a374a64f2a6a5313c3633c96aa"
                "6b3954a3666565cd03e8a26676ce006feac8a367656eac746573746e"
                "65742d76312e30a26768c4204863b518a4b3c84ec810f22d4f1081cb"
                "0f71f059a7ac20dec62f7f70e5093a22a26c76ce006feeb0a46e6f74"
                "65c40f667261672069766e20797271747265a3736e64c420769e258d"
                "9985738a385af7558118cbb3a2f6f1d417249af5292f247ace92ab2e"
                "a474797065a56178666572a478616964ce0093b229");

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
                "8ba3616d74cd2710a5636c6f7365c4202ff258acf69bb29b1ce13ea2"
                "eadb83c7a374a64f2a6a5313c3633c96aa6b3954a3666565cd03e8a2"
                "6676ce006feac8a367656eac746573746e65742d76312e30a26768c4"
                "204863b518a4b3c84ec810f22d4f1081cb0f71f059a7ac20dec62f7f"
                "70e5093a22a26c76ce006feeb0a46e6f7465c40f667261672069766e"
                "20797271747265a3726376c4202ff258acf69bb29b1ce13ea2eadb83"
                "c7a374a64f2a6a5313c3633c96aa6b3954a3736e64c420769e258d99"
                "85738a385af7558118cbb3a2f6f1d417249af5292f247ace92ab2ea4"
                "74797065a3706179");

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

