// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from mapcallback.djinni

#include "AlgorandAssetAmountMapCallback.hpp"  // my header
#include "AlgorandAssetAmount.hpp"
#include "Error.hpp"
#include "Marshal.hpp"

namespace djinni_generated {

AlgorandAssetAmountMapCallback::AlgorandAssetAmountMapCallback() : ::djinni::JniInterface<::ledger::core::api::AlgorandAssetAmountMapCallback, AlgorandAssetAmountMapCallback>() {}

AlgorandAssetAmountMapCallback::~AlgorandAssetAmountMapCallback() = default;

AlgorandAssetAmountMapCallback::JavaProxy::JavaProxy(JniType j) : Handle(::djinni::jniGetThreadEnv(), j) { }

AlgorandAssetAmountMapCallback::JavaProxy::~JavaProxy() = default;

void AlgorandAssetAmountMapCallback::JavaProxy::onCallback(const std::experimental::optional<std::unordered_map<std::string, ::ledger::core::api::AlgorandAssetAmount>> & c_result, const std::experimental::optional<::ledger::core::api::Error> & c_error) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::AlgorandAssetAmountMapCallback>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_onCallback,
                           ::djinni::get(::djinni::Optional<std::experimental::optional, ::djinni::Map<::djinni::String, ::djinni_generated::AlgorandAssetAmount>>::fromCpp(jniEnv, c_result)),
                           ::djinni::get(::djinni::Optional<std::experimental::optional, ::djinni_generated::Error>::fromCpp(jniEnv, c_error)));
    ::djinni::jniExceptionCheck(jniEnv);
}

}  // namespace djinni_generated
