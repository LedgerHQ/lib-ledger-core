#include <iostream>
#include <memory>
#include <wallet/common/BlocksSynchronizer.hpp>
#include <async/QtThreadPoolExecutionContext.hpp>
#include <async/QtMainExecutionContext.hpp>
#include <net/QtHttpClient.hpp>
#include <crypto/DeterministicPublicKey.hpp>
#include <math/Base58.hpp>
#include <bytes/BytesReader.h>
#include <wallet/bitcoin/keychains/ChangeKeychain.hpp>
#include <wallet/bitcoin/explorers/BitcoinLikeExplorer.hpp>
#include <wallet/bitcoin/networks.hpp>
#include <collections/DynamicObject.hpp>
#include <net/HttpClient.hpp>

using namespace std;
using namespace ledger::qt;
using namespace ledger::core;

class DummyKeysDB : public KeysDB {
public:
    virtual void addKey(const DeterministicPublicKey& address, uint32_t index) {

    }
    virtual std::vector<DeterministicPublicKey> getAllKeys() {
        return std::vector<DeterministicPublicKey>();
    }
    virtual void setMaxUsedIndex(uint32_t index) {

    }
    virtual uint32_t getMaxUsedIndex() {
        return 0;
    }
};


class BlockDB : public BlockchainDatabase<BitcoinLikeNetwork> {
public:
    void addBlocks(const std::vector<FilledBlock>& blocks) {
        throw "not implemented";
    }
    void addBlock(const FilledBlock& block) {
        std::cout << block.first.height << std::endl;
    }
    
    void removeBlocks(int heightFrom, int heightTo) {
        throw "not implemented";
    }
    
    void removeBlocksUpTo(int heightTo) {
        throw "not implemented";
    }
    void CleanAll() {
        throw "not implemented";
    }
    
    Future<std::vector<FilledBlock>> getBlocks(int heightFrom, int heightTo) {
        throw "not implemented";
    }
    FuturePtr<FilledBlock> getBlock(int height) {
        throw "not implemented";
    }
    // Return the last block header or genesis block
    FuturePtr<Block> getLastBlockHeader() {
        throw "not implemented";
    }
};

static DeterministicPublicKey createKeyFromXpub(const std::string& xpub) {
    auto raw = Base58::decode(xpub);
    BytesReader reader(raw);

    reader.readNextBeUint(); // READ MAGIC
    auto depth = reader.readNextByte();
    auto fingerprint = reader.readNextBeUint();
    auto childNum = reader.readNextBeUint();
    auto chainCode = reader.read(32);
    auto publicKey = reader.read(33);
    return DeterministicPublicKey(publicKey, chainCode, childNum, depth, fingerprint);
}

int main() {
    auto xpub = "xpub6BezZZ1HGWH3iGguzANbj6zLM4hqjGcpPUtTTwmSUc2DSEybcHSn5BUp5pVH562sWQPCh8qgNZE1NKB4RYL2aEReu4kxBCEYf5nXvsERbxW";
    auto pub = createKeyFromXpub(xpub);
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto mainContext = dispatcher->getMainExecutionContext();
    auto keysDB = std::make_shared<DummyKeysDB>();
    auto recieveChain = std::make_shared<bitcoin::ChangeKeychain>(pub.derive(0), keysDB);
    auto changeChain = std::make_shared<bitcoin::ChangeKeychain>(pub.derive(0), keysDB);
    auto blockDB = std::make_shared<BlockDB>();
    auto httpClient = std::make_shared<QtHttpClient>(mainContext);
    auto coreHttpClient = std::make_shared<HttpClient>("http://api.ledgerwallet.com", httpClient, mainContext);
    auto config = std::make_shared<DynamicObject>();

    auto explorer = std::make_shared<bitcoin::BitcoinLikeExplorer>(mainContext, coreHttpClient, networks::getNetworkParameters("bitcoin"), config);
    auto block_sync = std::make_shared<common::BlocksSynchronizer<BitcoinLikeNetwork>>
        (
            mainContext,
            explorer,
            recieveChain,
            changeChain,
            blockDB,
            20,
            20,
            200
            );
    auto firstBlock = std::shared_ptr<Block>();
    firstBlock->height = 0;
    firstBlock->hash = "0x000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";
    auto lastBlock = std::shared_ptr<Block>();
    lastBlock->height = 552412;
    block_sync->synchronize(firstBlock, lastBlock).onComplete(mainContext, [](const Try<Unit>& t) {
        if (t.isSuccess())
            std::cout << "success" << std::endl;
        else
            std::cout << t.getFailure().getMessage() << std::endl;
    });
    dispatcher->waitUntilStopped();
    return 0;
}
