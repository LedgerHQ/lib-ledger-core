// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from addresses.djinni

#ifndef DJINNI_GENERATED_ETHEREUMLIKENETWORKPARAMETERS_HPP
#define DJINNI_GENERATED_ETHEREUMLIKENETWORKPARAMETERS_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace ledger { namespace core { namespace api {

struct EthereumLikeNetworkParameters final {
    std::string Identifier;
    std::string MessagePrefix;
    std::vector<uint8_t> XPUBVersion;
    std::vector<std::string> AdditionalEIPs;
    int64_t TimestampDelay;

    EthereumLikeNetworkParameters(std::string Identifier_,
                                  std::string MessagePrefix_,
                                  std::vector<uint8_t> XPUBVersion_,
                                  std::vector<std::string> AdditionalEIPs_,
                                  int64_t TimestampDelay_)
    : Identifier(std::move(Identifier_))
    , MessagePrefix(std::move(MessagePrefix_))
    , XPUBVersion(std::move(XPUBVersion_))
    , AdditionalEIPs(std::move(AdditionalEIPs_))
    , TimestampDelay(std::move(TimestampDelay_))
    {}

    EthereumLikeNetworkParameters(const EthereumLikeNetworkParameters& cpy) {
       this->Identifier = cpy.Identifier;
       this->MessagePrefix = cpy.MessagePrefix;
       this->XPUBVersion = cpy.XPUBVersion;
       this->AdditionalEIPs = cpy.AdditionalEIPs;
       this->TimestampDelay = cpy.TimestampDelay;
    }

    EthereumLikeNetworkParameters() = default;


    EthereumLikeNetworkParameters& operator=(const EthereumLikeNetworkParameters& cpy) {
       this->Identifier = cpy.Identifier;
       this->MessagePrefix = cpy.MessagePrefix;
       this->XPUBVersion = cpy.XPUBVersion;
       this->AdditionalEIPs = cpy.AdditionalEIPs;
       this->TimestampDelay = cpy.TimestampDelay;
       return *this;
    }

    template <class Archive>
    void load(Archive& archive) {
        archive(Identifier, MessagePrefix, XPUBVersion, AdditionalEIPs, TimestampDelay);
    }

    template <class Archive>
    void save(Archive& archive) const {
        archive(Identifier, MessagePrefix, XPUBVersion, AdditionalEIPs, TimestampDelay);
    }
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_ETHEREUMLIKENETWORKPARAMETERS_HPP
