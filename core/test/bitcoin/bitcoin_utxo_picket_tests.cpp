#include <api/BitcoinLikeScript.hpp>
#include <api/KeychainEngines.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ledger/core/api/Networks.hpp>
#include <spdlog/sinks/null_sink.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeScriptApi.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <wallet/bitcoin/scripts/BitcoinLikeScript.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeStrategyUtxoPicker.h>
#include <wallet/common/Amount.h>
#include <wallet/currencies.hpp>

using namespace ledger::core;

class MockKeychain : public BitcoinLikeKeychain {
  public:
    MockKeychain(
        const std::shared_ptr<api::DynamicObject> &configuration,
        const api::Currency &params,
        int account,
        const std::shared_ptr<Preferences> &preferences) : BitcoinLikeKeychain(configuration, params, account, preferences){};
    MOCK_METHOD2(markPathAsUsed, bool(const DerivationPath &path, bool needExtendKeychain));
    MOCK_METHOD2(getAllObservableAddresses, std::vector<Address>(uint32_t from, uint32_t to));

    MOCK_METHOD3(getAllObservableAddresses, std::vector<Address>(KeyPurpose purpose, uint32_t from, uint32_t to));
    MOCK_METHOD2(getAllObservableAddressString, std::vector<std::string>(uint32_t from, uint32_t to));

    MOCK_METHOD1(getFreshAddress, Address(KeyPurpose purpose));
    MOCK_METHOD2(getFreshAddresses, std::vector<Address>(KeyPurpose purpose, size_t n));

    MOCK_METHOD0(getAllAddresses, std::vector<Address>());

    MOCK_CONST_METHOD1(getAddressPurpose, Option<KeyPurpose>(const std::string &address));
    MOCK_CONST_METHOD1(getAddressDerivationPath, Option<std::string>(const std::string &address));
    MOCK_CONST_METHOD0(isEmpty, bool());

    MOCK_CONST_METHOD1(getPublicKey, Option<std::vector<uint8_t>>(const std::string &address));

    MOCK_CONST_METHOD0(getRestoreKey, std::string());
    MOCK_CONST_METHOD0(getObservableRangeSize, int32_t());
    MOCK_CONST_METHOD1(contains, bool(const std::string &address));
    MOCK_CONST_METHOD0(getOutputSizeAsSignedTxInput, int32_t());
};

class MockBitcoinLikeOutput : public api::BitcoinLikeOutput {
  public:
    MockBitcoinLikeOutput(int64_t amount) : _amount(std::make_shared<Amount>(currencies::BITCOIN, 0, BigInt(amount))){};

    std::string getTransactionHash() { return _amount->toString(); };

    int32_t getOutputIndex() { return 0; };

    std::shared_ptr<api::Amount> getValue() {
        return _amount;
    }

    MOCK_METHOD0(getScript, std::vector<uint8_t>());

    MOCK_METHOD0(parseScript, std::shared_ptr<api::BitcoinLikeScript>());

    MOCK_METHOD0(getAddress, std::experimental::optional<std::string>());

    MOCK_METHOD0(getDerivationPath, std::shared_ptr<api::DerivationPath>());

    MOCK_METHOD0(getBlockHeight, std::experimental::optional<int64_t>());

    MOCK_CONST_METHOD0(isReplaceable, bool());

  private:
    std::shared_ptr<Amount> _amount;
};

std::vector<BitcoinLikeUtxo> createUtxos(const std::vector<int64_t> &values, const std::vector<Option<uint64_t>> &blockHeights) {
    std::vector<BitcoinLikeUtxo> utxos;
    const auto sz = values.size();
    assert(sz == blockHeights.size());
    utxos.reserve(sz);

    for (unsigned int i = 0; i < sz; ++i) {
        auto amount = Amount(currencies::BITCOIN, 0, BigInt(values[i]));
        utxos.emplace_back(BitcoinLikeUtxo{
            i,
            amount.toString(),
            amount,
            Option<std::string>{},
            Option<std::string>{},
            "",
            blockHeights[i]});
    }

    return utxos;
}

std::vector<BitcoinLikeUtxo> createUtxos(const std::vector<int64_t> &values) {
    std::vector<Option<uint64_t>> blockHeights(3, Option<uint64_t>{});
    return createUtxos(values, blockHeights);
}

std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> createBuddy(int64_t feesPerByte, int64_t outputAmount, const api::Currency &currency, const std::string keychainEngine = api::KeychainEngines::BIP32_P2PKH, const std::string address = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4") {
    BitcoinLikeTransactionBuildRequest r(std::make_shared<BigInt>(0));
    r.wipe                                       = false;
    r.feePerByte                                 = std::make_shared<BigInt>(feesPerByte);
    ledger::core::BitcoinLikeScript outputScript = ledger::core::BitcoinLikeScript::fromAddress(address, currency);
    r.outputs.push_back(std::make_tuple(std::make_shared<BigInt>(outputAmount), std::make_shared<BitcoinLikeScriptApi>(outputScript)));
    r.utxoPicker = BitcoinUtxoPickerParams{api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0, optional<int32_t>()};

    BitcoinLikeGetUtxoFunction g;
    BitcoinLikeGetTxFunction tx;
    std::shared_ptr<BitcoinLikeBlockchainExplorer> e;
    auto config = std::make_shared<ledger::core::DynamicObject>();
    config->putString(api::Configuration::KEYCHAIN_ENGINE, keychainEngine);
    config->putBoolean(api::Configuration::ALLOW_P2TR, true);
    std::shared_ptr<Preferences> preferences;
    std::shared_ptr<MockKeychain> k          = std::make_shared<MockKeychain>(config, currency, 0, preferences);
    static std::shared_ptr<spdlog::logger> l = spdlog::null_logger_mt("null_sink");
    std::shared_ptr<BitcoinLikeTransactionApi> t;
    return std::make_shared<BitcoinLikeUtxoPicker::Buddy>(r, g, tx, e, k, l, t, false);
}

TEST(OptimizeSize, BacktrackingCalculateChangeCorrectly) {
    const api::Currency currency             = currencies::BITCOIN;
    const int64_t feesPerByte                = 20;
    const int64_t inputSizeInBytes           = 148;
    const int64_t outputSizeInBytes          = 34;
    const int64_t emtyTransactionSizeInBytes = 10;
    int64_t outputAmount                     = 25000;
    std::vector<int64_t> inputAmounts{16500, 16500};

    auto buddy               = createBuddy(feesPerByte, outputAmount, currency);

    auto utxos               = createUtxos(inputAmounts);
    auto pickedUtxos         = BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(buddy, utxos, BigInt(-1), currency, false);
    int64_t totalInputsValue = 0;
    for (auto utxo : pickedUtxos) {
        totalInputsValue += utxo.value.toLong();
    }
    int64_t transactionFees     = totalInputsValue - buddy->changeAmount.toInt64() - outputAmount;
    int64_t minimumRequiredFees = (emtyTransactionSizeInBytes + outputSizeInBytes * 2 + inputSizeInBytes * pickedUtxos.size()) * feesPerByte;
    EXPECT_GE(transactionFees, minimumRequiredFees);
    if (buddy->changeAmount.toInt64() != 0)
        EXPECT_GE(buddy->changeAmount.toInt64(), inputSizeInBytes * feesPerByte);
}

TEST(OptimizeSize, ChangeShouldBeBigEnoughToSpend) {
    const api::Currency currency             = currencies::BITCOIN;
    const int64_t feesPerByte                = 20;
    const int64_t inputSizeInBytes           = 148;
    const int64_t outputSizeInBytes          = 34;
    const int64_t emtyTransactionSizeInBytes = 10;
    int64_t outputAmount                     = 25000;
    std::vector<int64_t> inputAmounts{19090, 19090};

    auto buddy               = createBuddy(feesPerByte, outputAmount, currency);

    auto utxos               = createUtxos(inputAmounts);
    auto pickedUtxos         = BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(buddy, utxos, BigInt(-1), currency, false);
    int64_t totalInputsValue = 0;
    for (auto utxo : pickedUtxos) {
        totalInputsValue += utxo.value.toLong();
    }
    int64_t transactionFees     = totalInputsValue - buddy->changeAmount.toInt64() - outputAmount;
    int64_t minimumRequiredFees = (emtyTransactionSizeInBytes + outputSizeInBytes * 2 + inputSizeInBytes * pickedUtxos.size()) * feesPerByte;
    EXPECT_GE(transactionFees, minimumRequiredFees);
    if (buddy->changeAmount.toInt64() != 0)
        EXPECT_GE(buddy->changeAmount.toInt64(), inputSizeInBytes * feesPerByte);
}

TEST(OptimizeSize, ApproximationShouldTookEnough) {
    const api::Currency currency             = currencies::BITCOIN;
    const int64_t feesPerByte                = 20;
    const int64_t inputSizeInBytes           = 148;
    const int64_t outputSizeInBytes          = 34;
    const int64_t emtyTransactionSizeInBytes = 10;
    int64_t outputAmount                     = 25000;
    std::vector<int64_t> inputAmounts{15000, 15000, 15000};

    auto buddy               = createBuddy(feesPerByte, outputAmount, currency);

    auto utxos               = createUtxos(inputAmounts);
    auto pickedUtxos         = BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(buddy, utxos, BigInt(-1), currency, false);
    int64_t totalInputsValue = 0;
    for (auto utxo : pickedUtxos) {
        totalInputsValue += utxo.value.toLong();
    }
    int64_t transactionFees     = totalInputsValue - buddy->changeAmount.toInt64() - outputAmount;
    int64_t minimumRequiredFees = (emtyTransactionSizeInBytes + outputSizeInBytes * 2 + inputSizeInBytes * pickedUtxos.size()) * feesPerByte;
    EXPECT_GE(transactionFees, minimumRequiredFees);
    if (buddy->changeAmount.toInt64() != 0)
        EXPECT_GE(buddy->changeAmount.toInt64(), inputSizeInBytes * feesPerByte);
}

TEST(OptimizeSize, UtxoOrderingShouldUseConfirmedFirst) {
    const api::Currency currency = currencies::BITCOIN;
    const int64_t feesPerByte    = 5;
    int64_t outputAmount         = 25000;
    std::vector<int64_t> inputAmounts{10000, 10000, 10000};
    std::vector<Option<uint64_t>> blockHeights = {Option<uint64_t>{}, 12000, Option<uint64_t>{}};

    auto buddy                                 = createBuddy(feesPerByte, outputAmount, currency);

    auto utxos                                 = createUtxos(inputAmounts, blockHeights);
    {
        auto pickedUtxos = BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(buddy, utxos, BigInt(-1), currency, true);
        EXPECT_EQ(pickedUtxos.size(), 3);
        EXPECT_EQ(pickedUtxos[0].index, 1);
    }
    // Cannot perform the "negative" as the algorithm may use randomization and thus might deliver a confirmed-first even if not requested.
}

TEST(DeepFirst, UtxoOrderingShouldUseConfirmedFirst) {
    const api::Currency currency = currencies::BITCOIN;
    const int64_t feesPerByte    = 20;
    int64_t outputAmount         = 25000;
    std::vector<int64_t> inputAmounts{15000, 15000, 15000, 15000};
    std::vector<Option<uint64_t>> blockHeights = {12000, Option<uint64_t>{}, 1000, Option<uint64_t>{}};

    auto buddy                                 = createBuddy(feesPerByte, outputAmount, currency);

    auto utxos                                 = createUtxos(inputAmounts, blockHeights);
    {
        auto pickedUtxos = BitcoinLikeStrategyUtxoPicker::filterWithDeepFirst(buddy, utxos, BigInt(-1), currency);
        EXPECT_EQ(pickedUtxos.size(), 3);
        EXPECT_EQ(pickedUtxos[0].index, 2);
        EXPECT_EQ(pickedUtxos[1].index, 0);
    }
}

TEST(MergeOutput, UtxoOrderingShouldUseConfirmedFirst) {
    const api::Currency currency = currencies::BITCOIN;
    const int64_t feesPerByte    = 5;
    int64_t outputAmount         = 25000;
    std::vector<int64_t> inputAmounts{15000, 5000, 5000, 1000, 1001, 1002, 1003, 3000};
    std::vector<Option<uint64_t>> blockHeights(inputAmounts.size(), Option<uint64_t>{});
    blockHeights[1] = 1000;
    blockHeights[6] = 2000;

    auto buddy      = createBuddy(feesPerByte, outputAmount, currency);

    auto utxos      = createUtxos(inputAmounts, blockHeights);
    {
        auto pickedUtxos = BitcoinLikeStrategyUtxoPicker::filterWithMergeOutputs(buddy, utxos, BigInt(-1), currency, true);
        EXPECT_EQ(pickedUtxos.size(), 8);
        EXPECT_EQ(pickedUtxos[0].index, 6);
        EXPECT_EQ(pickedUtxos[1].index, 1);
    }
    {
        auto pickedUtxos = BitcoinLikeStrategyUtxoPicker::filterWithMergeOutputs(buddy, utxos, BigInt(-1), currency, false);
        EXPECT_EQ(pickedUtxos.size(), 8);
        EXPECT_EQ(pickedUtxos[0].index, 3);
        EXPECT_EQ(pickedUtxos[1].index, 4);
    }
}

void feeIsEnoughFor(const std::string address, const int64_t targetOutputSizeInBytes, const int64_t feesPerByte) {
    const api::Currency currency             = currencies::BITCOIN_TESTNET;
    const int64_t inputSizeInBytes           = 68; // we are spending P2WPKH input

    const int64_t emtyTransactionSizeInBytes = 10;
    int64_t outputAmount                     = 50000000;
    std::vector<int64_t> inputAmounts{100000000};

    auto buddy                            = createBuddy(feesPerByte, outputAmount, currency, api::KeychainEngines::BIP173_P2WPKH, address);

    const int64_t changeOutputSizeInBytes = 8 + 1 + 1 + 1 + 20;
    const int64_t allOutputsSizeInBytes   = targetOutputSizeInBytes + changeOutputSizeInBytes;

    auto utxos                            = createUtxos(inputAmounts);
    auto pickedUtxos                      = BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(buddy, utxos, BigInt(-1), currency, false);
    int64_t totalInputsValue              = 0;
    for (auto utxo : pickedUtxos) {
        totalInputsValue += utxo.value.toLong();
    }
    int64_t transactionFees     = totalInputsValue - buddy->changeAmount.toInt64() - outputAmount;
    int64_t minimumRequiredFees = (emtyTransactionSizeInBytes + allOutputsSizeInBytes + inputSizeInBytes * pickedUtxos.size()) * feesPerByte;

    std::cerr << "transactionFees     == " << transactionFees << std::endl;
    std::cerr << "minimumRequiredFees == " << minimumRequiredFees << std::endl;

    EXPECT_GE(transactionFees, minimumRequiredFees);
    if (buddy->changeAmount.toInt64() != 0)
        EXPECT_GE(buddy->changeAmount.toInt64(), inputSizeInBytes * feesPerByte);
}

TEST(OptimizeSize, FeeIsEnoughForP2WPKH) {
    const std::string address             = "tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx"; // P2WPKH

    const int64_t targetOutputSizeInBytes = 8 + 1 + 1 + 1 + 20;
    // amount + script length + witness version + hash size + hash

    for (int64_t feesPerByte = 1; feesPerByte < 1000000; feesPerByte *= 10)
        feeIsEnoughFor(address, targetOutputSizeInBytes, feesPerByte);
}

TEST(OptimizeSize, FeeIsEnoughForP2WSH) {
    const std::string address             = "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sl5k7"; // P2WSH

    const int64_t targetOutputSizeInBytes = 8 + 1 + 1 + 1 + 32;
    // amount + script length + witness version + hash size + hash

    for (int64_t feesPerByte = 1; feesPerByte < 1000000; feesPerByte *= 10)
        feeIsEnoughFor(address, targetOutputSizeInBytes, feesPerByte);
}

TEST(OptimizeSize, FeeIsEnoughForP2TR) {
    const std::string address             = "tb1pqqqqp399et2xygdj5xreqhjjvcmzhxw4aywxecjdzew6hylgvsesf3hn0c"; // P2TR

    const int64_t targetOutputSizeInBytes = 8 + 1 + 1 + 1 + 32;
    // amount + script length + witness version + hash size + hash

    for (int64_t feesPerByte = 1; feesPerByte < 1000000; feesPerByte *= 10)
        feeIsEnoughFor(address, targetOutputSizeInBytes, feesPerByte);
}
