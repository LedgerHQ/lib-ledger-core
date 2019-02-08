// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#ifndef DJINNI_GENERATED_TRUSTINDICATOR_HPP
#define DJINNI_GENERATED_TRUSTINDICATOR_HPP

#include <cstdint>
#include <string>
#include <vector>
#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER) && _MSC_VER <= 1900
       #include <libcore_export.h>
    #else
       #define LIBCORE_EXPORT
    #endif
#endif

namespace ledger { namespace core { namespace api {

enum class TrustLevel;

/** The trust indicator of an operation. */
class LIBCORE_EXPORT TrustIndicator {
public:
    virtual ~TrustIndicator() {}

    virtual int32_t getTrustWeight() = 0;

    virtual TrustLevel getTrustLevel() = 0;

    virtual std::vector<std::string> getConflictingOperationUids() = 0;

    virtual std::string getOrigin() = 0;
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_TRUSTINDICATOR_HPP
