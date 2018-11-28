#include <boost/lexical_cast.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <memory>
#include <wallet/NetworkTypes.hpp>
#include <wallet/common/BlocksSynchronizer.hpp>
#include <wallet/common/InMemoryPartialBlocksDB.hpp>
#include <vector>

using namespace ledger::core;
using namespace testing;

namespace {
    struct T {
        std::vector<std::string> inputs;
        std::vector<std::pair<std::string, uint32_t>> outputs;
    };

    BitcoinLikeNetwork::Block toBlock(const B& b) {
        BitcoinLikeNetwork::Block block;
        block.hash = b.hash;
        block.height = b.height;
        return block;
    }

    BitcoinLikeNetwork::Transaction toTran(const T& tr) {
        BitcoinLikeNetwork::Transaction tran;
        tran.hash = "TR{";
        for (auto& in : tr.inputs) {
            BitcoinLikeNetwork::Input input;
            input.address = in;
            tran.inputs.push_back(input);
            tran.hash += in + ",";
        }
        tran.hash += "}->{";
        for (auto& out : tr.outputs) {
            BitcoinLikeNetwork::Output output;
            output.address = out.first;
            output.value = BigInt(out.second);
            tran.outputs.push_back(output);
            tran.hash += out.first + ",";
        }
        tran.hash += "}";
        return tran;
    }

    BitcoinLikeNetwork::Transaction toTran(const B& b, const T& tr) {
        BitcoinLikeNetwork::Block block = toBlock(b);
        BitcoinLikeNetwork::Transaction tran = toTran(tr);
        tran.block = block;
        return tran;
    }

    struct B {
        uint32_t height;
        std::string hash;
        std::vector<T> transactions;
    };
};

class FakeExplorer {
public:
    typedef ExplorerV2<BitcoinLikeNetwork>::TransactionBulk TransactionBulk;
    typedef BitcoinLikeNetwork::Transaction Tran;
    typedef BitcoinLikeNetwork::Input Input;
    typedef BitcoinLikeNetwork::Output Output;
    void setBlockchain(const std::vector<B>& blocks) {
        _transactions.clear();
        _blockHashes.clear();
        for (auto& b : blocks) {
            BitcoinLikeNetwork::Block block = toBlock(b);
            _blockHashes[block.hash] = block.height;
            for (auto& tr : b.transactions) {
                _transactions.push_back(toTran(b, tr));
            }
        }
        static std::function<bool(const Tran& l, const Tran& r)> blockLess = [](const Tran& l, const Tran& r)->bool {return l.block.getValue().height < r.block.getValue().height; };
        std::sort(_transactions.begin(), _transactions.end(), blockLess);
    }

    void setTruncationLevel(uint32_t numberOfTransactionsAllowed) {
        _numberOfTransactionsAllowed = numberOfTransactionsAllowed;
    }

    FuturePtr<TransactionBulk> getTransactions(const std::vector<std::string>& addresses, Option<std::string> fromBlockHash, Option<void*> session) {
        uint32_t fromBlockHeight = 0;
        if (fromBlockHash.hasValue())
        {
            auto it = _blockHashes.find(fromBlockHash.getValue());
            if (it == _blockHashes.end()) {
                return Future<std::shared_ptr<TransactionBulk>>::failure(Exception(api::ErrorCode::BLOCK_NOT_FOUND, "Very sorry"));
            }
            fromBlockHeight = it->second;
        }
        auto result = std::make_shared<TransactionBulk>();
        for (auto& tr : _transactions) {
            if (tr.block.getValue().height < fromBlockHeight)
                continue;
            if ()
            result->first.push_back();
        }
        return FuturePtr<TransactionBulk>::successful(result);
    }
private:
    std::vector<Tran> _transactions;
    std::map<std::string, uint32_t> _blockHashes;
    uint32_t _numberOfTransactionsAllowed
};

class FakeKeyChain : public Keychain<BitcoinLikeNetwork> {
public:
    FakeKeyChain(uint32_t alreadyUsed, uint32_t seed)
        : _alreadyUsed(alreadyUsed)
        , _seed(seed) {
    };

    uint32_t getNumberOfUsedAddresses() {
        return _alreadyUsed;
    }

    std::vector<std::string> getAddresses(uint32_t startIndex, uint32_t count) {
        std::vector<std::string> res;
        for (int i = startIndex; i < startIndex + count; ++i) {
            res.push_back(boost::lexical_cast<std::string>(i + _seed));
        }
        return res;
    }

    void markAsUsed(std::string& address) {
        try {
            auto addr = boost::lexical_cast<uint32_t>(address);
            _alreadyUsed = std::max(_alreadyUsed, addr - _seed + 1);
        }
        catch (const boost::bad_lexical_cast &) {}; // ignore
    }

private:
    uint32_t _alreadyUsed;
    uint32_t _seed;
};

class ExplorerMock : public ExplorerV2<BitcoinLikeNetwork> {
public:
    MOCK_METHOD0(startSession, Future<void *>());
    MOCK_METHOD1(killSession, Future<Unit>(void *session));
    MOCK_METHOD3(getTransactions, FuturePtr<TransactionBulk>(const std::vector<std::string>& addresses, Option<std::string> fromBlockHash, Option<void*> session));
    MOCK_METHOD0(getCurrentBlock, FuturePtr<Block>());
    MOCK_METHOD1(getRawTransaction, Future<Bytes>(const std::string& transactionHash));
    MOCK_METHOD1(getTransactionByHash, FuturePtr<Transaction>(const std::string& transactionHash));
    MOCK_METHOD1(pushTransaction, Future<std::string>(const std::vector<uint8_t>& transaction));
    MOCK_METHOD0(getTimestamp, Future<int64_t>());
};

class KeychainMock : public Keychain<BitcoinLikeNetwork> {
public:
    MOCK_METHOD0(getNumberOfUsedAddresses, uint32_t());
    MOCK_METHOD2(getAddresses, std::vector<std::string>(uint32_t startIndex, uint32_t count));
    MOCK_METHOD1(markAsUsed, void(std::string& address));
};

class BlocksDBMock : public BlockchainDatabase<BitcoinLikeNetwork> {
public:
    MOCK_METHOD1(addBlocks, Future<Unit> (const std::vector<BitcoinLikeNetwork::FilledBlock>& blocks));
    MOCK_METHOD2(removeBlocks, Future<Unit>(int heightFrom, int heightTo));
    MOCK_METHOD1(removeBlocksUpTo, Future<Unit>(int heightTo));
    MOCK_METHOD2(getBlocks, FuturePtr<std::vector<FilledBlock>>(int heightFrom, int heightTo));
    MOCK_METHOD0(getLastBlockHeader, FuturePtr<Block> ());
};

class BlockSyncTest : public ::testing::Test {
public:
    BlockSyncTest()
        : receivedFake(0, 0) // receive addresses are 0,1,2... change are 100, 101...
        , changeFake(0, 1000) { 
        firstBlock = std::make_shared<BitcoinLikeNetwork::Block>();
        lastBlock = std::make_shared<BitcoinLikeNetwork::Block>();
        _explorerMock = std::make_shared<ExplorerMock>();
        _keychainReceiveMock = std::make_shared<KeychainMock>();
        _keychainChangeMock = std::make_shared<KeychainMock>();
        _blocksDBMock = std::make_shared<BlocksDBMock>();
        synchronizer = std::make_shared<common::BlocksSynchronizer<BitcoinLikeNetwork>>(
            ImmediateExecutionContext::INSTANCE,
            _explorerMock,
            _keychainReceiveMock,
            _keychainChangeMock,
            _blocksDBMock,
            1,
            1);
    }

    void setupFakeKeychains() {
        ON_CALL(*_keychainReceiveMock, getNumberOfUsedAddresses()).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::getNumberOfUsedAddresses));
        ON_CALL(*_keychainReceiveMock, getAddresses(_, _)).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::getAddresses));
        ON_CALL(*_keychainReceiveMock, markAsUsed(_)).WillByDefault(Invoke(&receivedFake, &FakeKeyChain::markAsUsed));
    };

    void setBlockchain(const std::vector<B>& blockChain) {
        fakeExplorer.setBlockchain(blockChain);
        ON_CALL(*_explorerMock, getTransactions(_, _, _)).WillByDefault(Invoke(&fakeExplorer, &FakeExplorer::getTransactions));
    }

public:
    std::shared_ptr<common::BlocksSynchronizer<BitcoinLikeNetwork>> synchronizer;
    std::shared_ptr<BitcoinLikeNetwork::Block> firstBlock;
    std::shared_ptr<BitcoinLikeNetwork::Block> lastBlock;
    std::shared_ptr<ExplorerMock> _explorerMock;
    std::shared_ptr<KeychainMock> _keychainReceiveMock;
    std::shared_ptr<KeychainMock> _keychainChangeMock;
    std::shared_ptr<BlocksDBMock> _blocksDBMock;
    FakeKeyChain receivedFake;
    FakeKeyChain changeFake;
    FakeExplorer fakeExplorer;
};

TEST_F(BlockSyncTest, OneTransaction) {
    setupFakeKeychains();
    setBlockchain(
    { 
        B{0, "0 block hash",
            {T{}
            }}
    });
    synchronizer->synchronize(firstBlock, lastBlock);
}
