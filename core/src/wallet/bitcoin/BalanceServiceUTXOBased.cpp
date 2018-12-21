#include <wallet/bitcoin/BalanceServiceUTXOBased.hpp>
#include <wallet/bitcoin/UTXOService.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            BalanceServiceUTXOBased::BalanceServiceUTXOBased(const std::shared_ptr<api::ExecutionContext>& executionContext, const std::shared_ptr<UTXOService>& utxoService)
                : _executionContext(executionContext)
                , _utxoService(utxoService) {
            }

            Future<BigInt> BalanceServiceUTXOBased::getBalance() {
                return _utxoService->getUTXOs().map<BigInt>(
                    _executionContext,
                    [](const std::map<UTXOKey, UTXOValue>& utxos) {
                    BigInt res(0);
                    for (auto& it : utxos) {
                        res = res + it.second.amount;
                    }
                    return res;
                });
            }
        }
    }
}
