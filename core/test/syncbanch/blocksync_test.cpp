#include <iostream>
#include <memory>
#include <wallet/common/BlocksSynchronizer.hpp>
#include <async/QtThreadPoolExecutionContext.hpp>
#include <async/QtMainExecutionContext.hpp>
#include <net/QtHttpClient.hpp>
#include <crypto/DeterministicPublicKey.hpp>
#include <math/Base58.hpp>
#include <crypto/HASH160.hpp>
#include <bytes/BytesReader.h>
#include <wallet/bitcoin/keychains/ChangeKeychain.hpp>
#include <wallet/bitcoin/explorers/BitcoinLikeExplorer.hpp>
#include <bitcoin/BitcoinLikeExtendedPublicKey.hpp>
#include <wallet/bitcoin/networks.hpp>
#include <collections/DynamicObject.hpp>
#include <collections/vector.hpp>
#include <net/HttpClient.hpp>
#include <wallet/currencies.hpp>
#include <ledger/core/api/BitcoinLikeAddress.hpp>
#include <database/BlockchainLevelDB.hpp>
#include <wallet/common/PersistentBlockchainDatabase.hpp>

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

static DeterministicPublicKey createKeyFromXpub(const std::string& xpub) {
    auto& params = currencies::BITCOIN.bitcoinLikeNetworkParameters.value();
    auto decodeResult = Base58::checkAndDecode(xpub);
    if (decodeResult.isFailure())
        throw decodeResult.getFailure();
    BytesReader reader(decodeResult.getValue());
    auto version = reader.read(params.XPUBVersion.size());
    if (version != params.XPUBVersion) {
        throw  Exception(api::ErrorCode::INVALID_NETWORK_ADDRESS_VERSION, "Provided network parameters and address version do not match.");
    }
    auto depth = reader.readNextByte();
    auto fingerprint = reader.readNextBeUint();
    auto childNum = reader.readNextBeUint();
    auto chainCode = reader.read(32);
    auto publicKey = reader.readUntilEnd();
    return DeterministicPublicKey(publicKey, chainCode, childNum, depth, fingerprint);
}

std::string getP2SHAddress(const DeterministicPublicKey& key) {
    //Script
    std::vector<uint8_t> script = { 0x00, 0x14 };
    //Hash160 of public key
    auto publicKeyHash160 = key.getPublicKeyHash160();
    script.insert(script.end(), publicKeyHash160.begin(), publicKeyHash160.end());
    auto hash160 = HASH160::hash(script);
    return Base58::encodeWithChecksum(ledger::core::vector::concat(currencies::BITCOIN.bitcoinLikeNetworkParameters->P2SHVersion, hash160));
}

std::string getP2PKHAddress(const DeterministicPublicKey& key) {
    auto publicKeyHash160 = key.getPublicKeyHash160();
    return Base58::encodeWithChecksum(ledger::core::vector::concat(currencies::DIGIBYTE.bitcoinLikeNetworkParameters->P2PKHVersion, publicKeyHash160));
}

std::string getP2PKHAddressVTC(const DeterministicPublicKey& key) {
    auto publicKeyHash160 = key.getPublicKeyHash160();
    return Base58::encodeWithChecksum(ledger::core::vector::concat(currencies::VERTCOIN.bitcoinLikeNetworkParameters->P2PKHVersion, publicKeyHash160));
}



int main() {
    std::string dbname = "default";
    auto keysDB = std::make_shared<DummyKeysDB>();
    /* BITCOIN 
    auto xpub = "xpub6BezZZ1HGWH3iGguzANbj6zLM4hqjGcpPUtTTwmSUc2DSEybcHSn5BUp5pVH562sWQPCh8qgNZE1NKB4RYL2aEReu4kxBCEYf5nXvsERbxW";
    auto firstBlock = std::make_shared<Block>();
    firstBlock->height = 0;
    firstBlock->hash = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";
    auto lastBlock = std::make_shared<Block>();
    lastBlock->height = 552412;
    */
    /*  DIGICODE */
    auto network = currencies::DIGIBYTE.bitcoinLikeNetworkParameters.value();
    auto xpub = "xpub6Brsv1WawWHbVBPre4YkJypg5h8n5XyfcXrNREgRWUjemejLB5DGcfSuJLsCT7Bq5AfUN1mnroRw34v31JrmyPQCSxc1dbMmHaRbDTfVgYo";
    auto pub = createKeyFromXpub(xpub);
    auto receiveKey = pub.derive(0);
    auto changeKey = pub.derive(1);
    auto recieveChain = std::make_shared<bitcoin::ChangeKeychain>(receiveKey, getP2PKHAddress, keysDB);
    auto changeChain = std::make_shared<bitcoin::ChangeKeychain>(changeKey, getP2PKHAddress, keysDB);
    BitcoinLikeNetwork::Block firstBlock;
    firstBlock.height = 1;
    firstBlock.hash = "4da631f2ac1bed857bd968c67c913978274d8aabed64ab2bcebc1665d7f4d3a0";
    BitcoinLikeNetwork::Block lastBlock;
    lastBlock.height = 7783743;
    dbname = "digi";
    /**/
    /*  VTC 
    auto network = currencies::VERTCOIN.bitcoinLikeNetworkParameters.value();
    auto xpub = "xpub6CjCxPW8W5gbr1gyJpGr5BLcocwUpECcmmNBjW8XEAA28eqR5Z9vBKc8tESUhMZGPy2nK7x2fAHnxyzpFqsBz2pFSMfJCNYjDnFsvkGh56P";
    auto pub = createKeyFromXpub(xpub);
    auto receiveKey = pub.derive(0);
    auto changeKey = pub.derive(1);
    auto recieveChain = std::make_shared<bitcoin::ChangeKeychain>(receiveKey, getP2PKHAddressVTC, keysDB);
    auto changeChain = std::make_shared<bitcoin::ChangeKeychain>(changeKey, getP2PKHAddressVTC, keysDB);
    bitcoin::Block firstBlock;
    firstBlock.height = 1;
    firstBlock.hash = "9249c198d4b1ca38bb92a740d6557df80e746108384552509225e7825556ebdf";
    bitcoin::Block lastBlock;
    lastBlock.height = 1045723;
    */
    
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto mainContext = dispatcher->getMainExecutionContext();

    std::shared_ptr<db::BlockchainDB> persistentLayer = std::make_shared<db::BlockchainLevelDB>(dbname);
    auto blockDB = std::make_shared<common::PersistentBlockchainDatabase<BitcoinLikeNetwork>>(mainContext, persistentLayer);
    auto httpClient = std::make_shared<QtHttpClient>(mainContext);
    auto coreHttpClient = std::make_shared<HttpClient>("http://api.ledgerwallet.com", httpClient, mainContext);
    auto config = std::make_shared<DynamicObject>();

    auto explorer = std::make_shared<bitcoin::BitcoinLikeExplorer>(mainContext, coreHttpClient, network, config);
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
       
    block_sync->synchronize(firstBlock.hash, firstBlock.height, lastBlock.height)
        .onComplete(mainContext, [mainContext, blockDB](const Try<Unit>& t) {
        if (t.isSuccess()) {
            std::cout << "success" << std::endl;
            blockDB->getLastBlockHeader().onComplete(mainContext, [](const Try<Option<BitcoinLikeNetwork::Block>>& block) {
                if (block.isSuccess()) {
                    if (block.getValue().hasValue())
                        std::cout << block.getValue().getValue().hash << " " << block.getValue().getValue().hash << std::endl;
                    else
                        std::cout << "not found" << std::endl;
                }
                else {
                    std::cout << "Failure during reading DB" << std::endl;
                    std::cout << block.getFailure().getMessage() << std::endl;
                }
            });
        }
        else {
            std::cout << "not good" << std::endl;
            std::cout << t.getFailure().getMessage() << std::endl;
        }
    });
    dispatcher->waitUntilStopped();
    return 0;
}
