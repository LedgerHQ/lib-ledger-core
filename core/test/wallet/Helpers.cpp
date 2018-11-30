#include <Helpers.hpp>

namespace ledger {
    namespace core {
        namespace tests {
            BitcoinLikeNetwork::Block toBlock(const BL& b) {
                BitcoinLikeNetwork::Block block;
                block.hash = b.hash;
                block.height = b.height;
                return block;
            }

            BitcoinLikeNetwork::Transaction toTran(const std::string& blockHash, const TR& tr) {
                BitcoinLikeNetwork::Transaction tran;
                tran.hash = blockHash + "TR{";
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

            BitcoinLikeNetwork::Transaction toTran(const BL& b, const TR& tr) {
                BitcoinLikeNetwork::Block block = toBlock(b);
                BitcoinLikeNetwork::Transaction tran = toTran(block.hash, tr);
                tran.block = block;
                return tran;
            }

            std::vector<BitcoinLikeNetwork::FilledBlock> toFilledBlocks(const std::vector<BL>& blocks) {
                std::vector<BitcoinLikeNetwork::FilledBlock> res;
                for (auto& b : blocks) {
                    BitcoinLikeNetwork::FilledBlock fb;
                    fb.first = toBlock(b);
                    std::vector<BitcoinLikeNetwork::Transaction> transactions;
                    for (auto& tr : b.transactions) {
                        fb.second.push_back(toTran(b, tr));
                    }
                    res.push_back(fb);
                }
                return res;
            }

            void FakeExplorer::setBlockchain(const std::vector<BitcoinLikeNetwork::FilledBlock>& blockchain) {
                _transactions.clear();
                _blockHashes.clear();
                for (auto& fb : blockchain) {
                    _blockHashes[fb.first.hash] = fb.first.height;
                    for (auto& tr : fb.second) {
                        _transactions.push_back(tr);
                    }
                }
                static std::function<bool(const Tran& l, const Tran& r)> blockLess = [](const Tran& l, const Tran& r)->bool {return l.block.getValue().height < r.block.getValue().height; };
                std::sort(_transactions.begin(), _transactions.end(), blockLess);
            }

            void FakeExplorer::setTruncationLevel(uint32_t numberOfTransactionsAllowed) {
                _numberOfTransactionsAllowed = numberOfTransactionsAllowed;
            }

            FuturePtr<FakeExplorer::TransactionBulk> FakeExplorer::getTransactions(const std::vector<std::string>& addresses, Option<std::string> fromBlockHash, Option<void*> session) {
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
                result->second = false;
                static std::function<bool(const Tran& l, const Tran& r)> blockLess = [](const Tran& l, const Tran& r)->bool {return l.block.getValue().height < r.block.getValue().height; };
                Tran fff;
                Block bbb;
                bbb.height = fromBlockHeight;
                fff.block = bbb;
                auto lower = std::lower_bound(_transactions.begin(), _transactions.end(), fff, blockLess);
                for (auto x = lower; x != _transactions.end(); x++) {
                    auto&tr = *x;
                    bool hasAddress = false;
                    for (auto& in : tr.inputs) {
                        if (in.address.hasValue() && (std::find(addresses.begin(), addresses.end(), in.address.getValue()) != addresses.end())) {
                            hasAddress = true;
                            break;
                        }
                    }
                    if (!hasAddress) {
                        for (auto& out : tr.outputs) {
                            if (out.address.hasValue() && (std::find(addresses.begin(), addresses.end(), out.address.getValue()) != addresses.end())) {
                                hasAddress = true;
                                break;
                            }
                        }
                    }
                    if (hasAddress) {
                        if (result->first.size() == _numberOfTransactionsAllowed) {
                            result->second = true; // truncated
                            break;
                        }
                        result->first.push_back(tr);
                    }
                }
                // to not execute in stack, but simulate really async environment
                return FuturePtr<TransactionBulk>::successful(result);
            };


            FakeKeyChain::FakeKeyChain(uint32_t alreadyUsed, uint32_t seed)
                : _alreadyUsed(alreadyUsed)
                , _seed(seed) {
            };

            uint32_t FakeKeyChain::getNumberOfUsedAddresses() {
                return _alreadyUsed;
            }

            std::vector<std::string> FakeKeyChain::getAddresses(uint32_t startIndex, uint32_t count) {
                std::vector<std::string> res;
                for (uint32_t i = startIndex; i < startIndex + count; ++i) {
                    res.push_back(boost::lexical_cast<std::string>(i + _seed));
                }
                return res;
            }

            void FakeKeyChain::markAsUsed(std::string& address) {
                try {
                    auto addr = boost::lexical_cast<uint32_t>(address);
                    _alreadyUsed = std::max(_alreadyUsed, addr - _seed + 1);
                }
                catch (const boost::bad_lexical_cast &) {}; // ignore
            }

            bool blocksSame(const std::vector<BitcoinLikeNetwork::FilledBlock>& fbs, const std::vector<BL>& bs) {
                if (fbs.size() != bs.size())
                    return false;
                for (int i = 0; i < fbs.size(); ++i) {
                    if (fbs[i].first.hash != bs[i].hash)
                        return false;
                    if (fbs[i].first.height != bs[i].height)
                        return false;
                    if (fbs[i].second.size() != bs[i].transactions.size())
                        return false;
                    for (int j = 0; j < bs[i].transactions.size(); ++j) {
                        if (fbs[i].second[j].inputs.size() != bs[i].transactions[j].inputs.size())
                            return false;
                        if (fbs[i].second[j].outputs.size() != bs[i].transactions[j].outputs.size())
                            return false;
                        for (int k = 0; k < bs[i].transactions[j].inputs.size(); ++k) {
                            if (!fbs[i].second[j].inputs[k].address.hasValue())
                                return false;
                            if (fbs[i].second[j].inputs[k].address.getValue() != bs[i].transactions[j].inputs[k])
                                return false;
                        }
                        for (int k = 0; k < bs[i].transactions[j].outputs.size(); ++k) {
                            if (!fbs[i].second[j].outputs[k].address.hasValue())
                                return false;
                            if (fbs[i].second[j].outputs[k].address.getValue() != bs[i].transactions[j].outputs[k].first)
                                return false;
                            if (!(fbs[i].second[j].outputs[k].value == BigInt(bs[i].transactions[j].outputs[k].second)))
                                return false;
                        }
                    }
                }
                return true;
            };
        }
    }
}