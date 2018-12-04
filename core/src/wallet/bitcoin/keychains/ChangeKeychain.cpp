#include <algorithm>
#include <collections/vector.hpp>
#include <math/Base58.hpp>
#include <string>
#include <wallet/bitcoin/keychains/ChangeKeychain.hpp>

namespace ledger{
    namespace core {
        namespace bitcoin {
            ChangeKeychain::ChangeKeychain(
                const DeterministicPublicKey& key,
                const AddressGetter& addressGetter,
                const std::shared_ptr<KeysDB>& keysDB)
                : _parentKey(key)
                , _addressGetter(addressGetter)
                , _keysDB(keysDB) {
                _maxUsedIndex  = _keysDB->getMaxUsedIndex();
                auto keys = _keysDB->getAllKeys();
                for (int i = 0; i < keys.size(); ++i) {
                    auto address = getAddresse(keys[i]);
                    _addresses.push_back(address);
                    _indexMap[address] = i;
                }
            }

            uint32_t ChangeKeychain::getNumberOfUsedAddresses() {
                return _maxUsedIndex;
            }

            std::string ChangeKeychain::getAddresse(const DeterministicPublicKey& key) {
                return _addressGetter(key);
            }

            std::vector<std::string> ChangeKeychain::getAddresses(uint32_t startIndex, uint32_t count) {
                std::lock_guard<std::mutex> lock(_lock);
                std::vector<std::string> res;
                int finishUnknownIndex = std::max<uint32_t>(startIndex + count, _addresses.size());
                // unknown addresses
                for (int i = _addresses.size(); i < finishUnknownIndex; ++i) {
                    auto newKey = _parentKey.derive(i);
                    _keysDB->addKey(newKey, i);
                    auto address = getAddresse(newKey);
                    _addresses.push_back(address);
                    _indexMap[address] = i;
                }
                for (int i = startIndex; i < startIndex + count; ++i)
                    res.push_back(_addresses[i]);
                return res;
            }

            void ChangeKeychain::markAsUsed(const std::string& address) {
                std::lock_guard<std::mutex> lock(_lock);
                auto it = _indexMap.find(address);
                if (it == _indexMap.end())
                    return;
                if (it->second > _maxUsedIndex) {
                    _maxUsedIndex = it->second;
                    _keysDB->setMaxUsedIndex(_maxUsedIndex);
                }
            }
        }
    } 
}