#include <wallet/Operation.h>
#include <wallet/Keychain.hpp>

namespace ledger {
    namespace core {
        Operation createOperation(const BitcoinLikeNetwork::Transaction& transaction, const std::shared_ptr<KeychainRegistry>& addressRegistry) {
            Operation op;
            op.bitcoinTransaction = transaction;
            auto fee = BigInt(0);
            auto sentByMe = BigInt(0);
            for (auto& input : transaction.inputs) {
                auto value = input.value.getValueOr(BigInt(0));
                fee = fee + value;
                if (!input.address.hasValue())
                    continue;
                const std::string& addr = input.address.getValue();
                op.senders.push_back(addr);
                if (addressRegistry->containsAddress(addr))
                    sentByMe = sentByMe + value;
            }

            auto receivedByMe = BigInt(0);
            for (auto& output : transaction.outputs) {
                fee = fee - output.value;
                if (!output.address.hasValue())
                    continue;
                const std::string& addr = output.address.getValue();
                op.recipients.push_back(addr);
                if (addressRegistry->containsAddress(addr))
                    receivedByMe = receivedByMe + output.value;
            }
            if (sentByMe > receivedByMe) {
                op.type = api::OperationType::SEND;
                op.amount = sentByMe - receivedByMe;
            }
            else {
                op.type = api::OperationType::RECEIVE;
                op.amount = receivedByMe - sentByMe;
            }
            op.fees = fee;
            op.block = transaction.block;
            op.date = transaction.receivedAt;
            return op;
        }
    }
}