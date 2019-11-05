#include <gtest/gtest.h>

#include <core/explorers/AbstractBlockParser.hpp>
#include <core/utils/JSONUtils.hpp>

using namespace ledger::core;

struct TestBlock {
    uint64_t height;
    std::string blockHash;
    std::chrono::system_clock::time_point time;
};

struct TestParser : AbstractBlockParser<TestBlock> {
    typedef TestBlock Result;

    TestParser(std::string& lastKey) : _lastKey(lastKey) {}
    ~TestParser() = default;

    bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
        _lastKey = std::string(str, length);
        return true;
    }

protected:
    std::string& _lastKey;

    std::string& getLastKey() override {
        return _lastKey;
    }
};

TEST(TXParser, NumbersAsStrings) {
    auto json = "{\"height\": 12}";
    auto parsed = JSONUtils::parse<TestParser>(json);
    EXPECT_EQ(parsed->height, 12);
}
