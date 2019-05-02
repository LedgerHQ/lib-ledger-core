// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from tezos_like_wallet.djinni

#ifndef DJINNI_GENERATED_TEZOSCONFIGURATIONDEFAULTS_HPP
#define DJINNI_GENERATED_TEZOSCONFIGURATIONDEFAULTS_HPP

#include <string>
#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER)
       #include <libcore_export.h>
    #else
       #define LIBCORE_EXPORT
    #endif
#endif

namespace ledger { namespace core { namespace api {

class LIBCORE_EXPORT TezosConfigurationDefaults {
public:
    virtual ~TezosConfigurationDefaults() {}

    static std::string const TEZOS_DEFAULT_API_ENDPOINT;

    static std::string const TEZOS_OBSERVER_NODE_ENDPOINT_S2;

    static std::string const TEZOS_OBSERVER_NODE_ENDPOINT_S3;

    static std::string const TEZOS_OBSERVER_WS_ENDPOINT_S2;

    static std::string const TEZOS_OBSERVER_WS_ENDPOINT_S3;

    static std::string const TEZOS_DEFAULT_PORT;
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_TEZOSCONFIGURATIONDEFAULTS_HPP
