// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from erc20.djinni

#ifndef DJINNI_GENERATED_ERC20LIKEACCOUNT_HPP
#define DJINNI_GENERATED_ERC20LIKEACCOUNT_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ledger { namespace core { namespace api {

class BigInt;
class ERC20LikeOperation;
class OperationQuery;
struct ERC20Token;

/**ERC20-like accounts class */
class ERC20LikeAccount {
public:
    virtual ~ERC20LikeAccount() {}

    virtual ERC20Token getToken() = 0;

    virtual std::string getAddress() = 0;

    virtual std::shared_ptr<BigInt> getBalance() = 0;

    virtual std::vector<std::shared_ptr<ERC20LikeOperation>> getOperations() = 0;

    virtual std::vector<uint8_t> getTransferToAddressData(const std::shared_ptr<BigInt> & amount, const std::string & address) = 0;

    virtual std::shared_ptr<OperationQuery> queryOperations() = 0;
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_ERC20LIKEACCOUNT_HPP
