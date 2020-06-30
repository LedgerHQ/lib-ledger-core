#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <api/KeychainEngines.hpp>
#include <api/BitcoinLikeScript.hpp>
#include <ledger/core/api/Networks.hpp>
#include <wallet/currencies.hpp>
#include <wallet/common/Amount.h>
#include <wallet/bitcoin/transaction_builders/BitcoinLikeStrategyUtxoPicker.h>
#include <spdlog/sinks/null_sink.h>


using namespace ledger::core;

class MockKeychain : public BitcoinLikeKeychain {
public:
    MockKeychain(
        const std::shared_ptr<api::DynamicObject>& configuration,
        const api::Currency& params,
        int account,
        const std::shared_ptr<Preferences>& preferences) :BitcoinLikeKeychain(configuration, params, account, preferences){};
    MOCK_METHOD1(markPathAsUsed, bool(const DerivationPath& path));
    MOCK_METHOD2(getAllObservableAddresses, std::vector<Address>(uint32_t from, uint32_t to));

    MOCK_METHOD3(getAllObservableAddresses, std::vector<Address>(KeyPurpose purpose, uint32_t from, uint32_t to));

    MOCK_METHOD1(getFreshAddress, Address(KeyPurpose purpose));
    MOCK_METHOD2(getFreshAddresses, std::vector<Address>(KeyPurpose purpose, size_t n));

    MOCK_CONST_METHOD1(getAddressPurpose, Option<KeyPurpose>(const std::string& address));
    MOCK_CONST_METHOD1(getAddressDerivationPath, Option<std::string>(const std::string& address));
    MOCK_CONST_METHOD0(isEmpty, bool());

    MOCK_CONST_METHOD1(getPublicKey, Option<std::vector<uint8_t>>(const std::string& address));

    MOCK_CONST_METHOD0(getRestoreKey, std::string());
    MOCK_CONST_METHOD0(getObservableRangeSize, int32_t());
    MOCK_CONST_METHOD1(contains, bool(const std::string& address));
    MOCK_CONST_METHOD0(getOutputSizeAsSignedTxInput, int32_t());
};

class MockBitcoinLikeScript : public api::BitcoinLikeScript {
public:
    MOCK_METHOD0(head, std::shared_ptr<api::BitcoinLikeScriptChunk>());
    MOCK_METHOD0(toString, std::string());
};

class MockBitcoinLikeOutput : public api::BitcoinLikeOutput {
public:
    MockBitcoinLikeOutput(int64_t amount) : _amount(std::make_shared<Amount>(currencies::BITCOIN, 0, BigInt(amount))) {};

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

BitcoinLikeApiUtxoVector createUtxos(const std::vector<int64_t>& values) {
    BitcoinLikeApiUtxoVector ans;
    for (auto val : values) {
        ans.push_back(std::make_shared<MockBitcoinLikeOutput>(val));
    }
    return ans;
}

std::shared_ptr<BitcoinLikeUtxoPicker::Buddy> createBuddy(int64_t feesPerByte, int64_t outputAmount, const api::Currency& currency) {
    BitcoinLikeTransactionBuildRequest r(std::make_shared<BigInt>(0));
    r.wipe = false;
    r.feePerByte = std::make_shared<BigInt>(feesPerByte);
    r.outputs.push_back(std::make_tuple(std::make_shared<BigInt>(outputAmount), std::make_shared<MockBitcoinLikeScript>()));
    r.utxoPicker = std::make_tuple(api::BitcoinLikePickingStrategy::OPTIMIZE_SIZE, 0);

    BitcoinLikeGetUtxoFunction g;
    BitcoinLikeGetTxFunction tx;
    std::shared_ptr<BitcoinLikeBlockchainExplorer> e;
    auto config = std::make_shared<ledger::core::DynamicObject>();
    config->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP32_P2PKH);
    std::shared_ptr<Preferences> preferences;
    std::shared_ptr<MockKeychain> k = std::make_shared<MockKeychain>(config, currency, 0, preferences);
    static std::shared_ptr<spdlog::logger> l = spdlog::null_logger_mt("null_sink");
    std::shared_ptr<BitcoinLikeTransactionApi> t;
    return std::make_shared<BitcoinLikeUtxoPicker::Buddy>(r, g, tx, e, k, l, t, false);
}

TEST(OptimizeSize, BacktrackingCalculateChangeCorrectly) {
    const api::Currency currency = currencies::BITCOIN;
    const int64_t feesPerByte = 20;
    const int64_t inputSizeInBytes = 148;
    const int64_t outputSizeInBytes = 34;
    const int64_t emtyTransactionSizeInBytes = 10;
    int64_t outputAmount = 25000;
    std::vector<int64_t> inputAmounts{16500, 16500};
    
    auto buddy = createBuddy(feesPerByte, outputAmount, currency);

    auto utxos = createUtxos(inputAmounts);
    auto pickedUtxos = BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(buddy, utxos, BigInt(-1), currency);
    int64_t totalInputsValue = 0;
    for (auto utxo : pickedUtxos) {
        totalInputsValue += BigInt::fromString(std::get<0>(utxo)).toInt64();
    }
    int64_t transactionFees = totalInputsValue - buddy->changeAmount.toInt64() - outputAmount;
    int64_t minimumRequiredFees = (emtyTransactionSizeInBytes + outputSizeInBytes * 2 + inputSizeInBytes * pickedUtxos.size()) * feesPerByte;
    EXPECT_GE(transactionFees, minimumRequiredFees);
    if (buddy->changeAmount.toInt64() != 0)
        EXPECT_GE(buddy->changeAmount.toInt64(), inputSizeInBytes * feesPerByte);
}

TEST(OptimizeSize, ChangeShouldBeBigEnoughToSpend) {
    const api::Currency currency = currencies::BITCOIN;
    const int64_t feesPerByte = 20;
    const int64_t inputSizeInBytes = 148;
    const int64_t outputSizeInBytes = 34;
    const int64_t emtyTransactionSizeInBytes = 10;
    int64_t outputAmount = 25000;
    std::vector<int64_t> inputAmounts{ 19090, 19090};

    auto buddy = createBuddy(feesPerByte, outputAmount, currency);

    auto utxos = createUtxos(inputAmounts);
    auto pickedUtxos = BitcoinLikeStrategyUtxoPicker::filterWithOptimizeSize(buddy, utxos, BigInt(-1), currency);
    int64_t totalInputsValue = 0;
    for (auto utxo : pickedUtxos) {
        totalInputsValue += BigInt::fromString(std::get<0>(utxo)).toInt64();
    }
    int64_t transactionFees = totalInputsValue - buddy->changeAmount.toInt64() - outputAmount;
    int64_t minimumRequiredFees = (emtyTransactionSizeInBytes + outputSizeInBytes * 2 + inputSizeInBytes * pickedUtxos.size()) * feesPerByte;
    EXPECT_GE(transactionFees, minimumRequiredFees);
    if (buddy->changeAmount.toInt64() != 0)
        EXPECT_GE(buddy->changeAmount.toInt64(), inputSizeInBytes * feesPerByte);
}
