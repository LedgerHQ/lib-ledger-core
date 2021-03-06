// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from messages.djinni

#ifndef DJINNI_GENERATED_COSMOSLIKEMSGUNDELEGATE_HPP
#define DJINNI_GENERATED_COSMOSLIKEMSGUNDELEGATE_HPP

#include "CosmosLikeAmount.hpp"
#include <iostream>
#include <string>
#include <utility>

namespace ledger { namespace core { namespace api {

struct CosmosLikeMsgUndelegate final {
    std::string delegatorAddress;
    std::string validatorAddress;
    CosmosLikeAmount amount;

    CosmosLikeMsgUndelegate(std::string delegatorAddress_,
                            std::string validatorAddress_,
                            CosmosLikeAmount amount_)
    : delegatorAddress(std::move(delegatorAddress_))
    , validatorAddress(std::move(validatorAddress_))
    , amount(std::move(amount_))
    {}

    CosmosLikeMsgUndelegate(const CosmosLikeMsgUndelegate& cpy) {
       this->delegatorAddress = cpy.delegatorAddress;
       this->validatorAddress = cpy.validatorAddress;
       this->amount = cpy.amount;
    }

    CosmosLikeMsgUndelegate() = default;


    CosmosLikeMsgUndelegate& operator=(const CosmosLikeMsgUndelegate& cpy) {
       this->delegatorAddress = cpy.delegatorAddress;
       this->validatorAddress = cpy.validatorAddress;
       this->amount = cpy.amount;
       return *this;
    }

    template <class Archive>
    void load(Archive& archive) {
        archive(delegatorAddress, validatorAddress, amount);
    }

    template <class Archive>
    void save(Archive& archive) const {
        archive(delegatorAddress, validatorAddress, amount);
    }
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_COSMOSLIKEMSGUNDELEGATE_HPP
