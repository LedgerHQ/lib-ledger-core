#include <api/ExecutionContext.hpp>
#include <async/FutureUtils.hpp>
#include <memory>
#include <wallet/bitcoin/UTXOServiceSourceBased.hpp>
#include <wallet/bitcoin/UTXOService.hpp>
#include <wallet/bitcoin/UTXOSource.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            UTXOServiceSourceBased::UTXOServiceSourceBased(
                const std::shared_ptr<api::ExecutionContext>& executionContext,
                const std::shared_ptr<UTXOSource>& stableBlocksSource,
                const std::shared_ptr<UTXOSource>& unstableBlocksSource,
                const std::shared_ptr<UTXOSource>& pendingTransactionsSource)
            : _executionContext(executionContext)
            , _stableBlocksSource(stableBlocksSource)
            , _unstableBlocksSource(unstableBlocksSource)
            , _pendingTransactionsSource(pendingTransactionsSource) {
            }

            Future<std::map<UTXOKey, UTXOValue>> UTXOServiceSourceBased::getUTXOs() {
                auto self = shared_from_this();
                std::vector<Future<UTXOSourceList>> sourcesFuture {
                    _pendingTransactionsSource->getUTXOs(_executionContext),
                    _unstableBlocksSource->getUTXOs(_executionContext),
                    _stableBlocksSource->getUTXOs(_executionContext) };
                return
                    executeAll(_executionContext, sourcesFuture)
                    .map<std::map<UTXOKey, UTXOValue>>(_executionContext, [](const std::vector<UTXOSourceList>& lists){
                        const UTXOSourceList& pending = lists[0];
                        const UTXOSourceList& unstable = lists[1];
                        const UTXOSourceList& stable = lists[2];
                        std::map<UTXOKey, UTXOValue> res(stable.available.begin(), stable.available.end());
                        res.insert(unstable.available.begin(), unstable.available.end());
                        for (auto&key : unstable.spent)
                            res.erase(key);
                        res.insert(pending.available.begin(), pending.available.end());
                        for (auto&key : pending.spent)
                            res.erase(key);
                        return res;
                    });
            }
        }
    }
}
