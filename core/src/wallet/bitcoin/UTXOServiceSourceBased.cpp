#pragma once

#include <api/ExecutionContext.hpp>
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
                return _pendingTransactionsSource->getUTXOs(_executionContext)
                      .flatMap<UTXOSourceList>(
                          _executionContext,
                          [self](const UTXOSourceList& utxos) {
                              return                                       
                          }
                        )
                    .map<std::map<UTXOKey, UTXOValue>>(_executionContext, [](const UTXOSourceList& utxos) {return utxos.available; });
            }
        }
    }
}