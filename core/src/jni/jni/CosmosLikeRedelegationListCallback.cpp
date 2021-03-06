// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#include "CosmosLikeRedelegationListCallback.hpp"  // my header
#include "CosmosLikeRedelegation.hpp"
#include "Error.hpp"
#include "Marshal.hpp"

namespace djinni_generated {

CosmosLikeRedelegationListCallback::CosmosLikeRedelegationListCallback() : ::djinni::JniInterface<::ledger::core::api::CosmosLikeRedelegationListCallback, CosmosLikeRedelegationListCallback>() {}

CosmosLikeRedelegationListCallback::~CosmosLikeRedelegationListCallback() = default;

CosmosLikeRedelegationListCallback::JavaProxy::JavaProxy(JniType j) : Handle(::djinni::jniGetThreadEnv(), j) { }

CosmosLikeRedelegationListCallback::JavaProxy::~JavaProxy() = default;

void CosmosLikeRedelegationListCallback::JavaProxy::onCallback(const std::experimental::optional<std::vector<std::shared_ptr<::ledger::core::api::CosmosLikeRedelegation>>> & c_result, const std::experimental::optional<::ledger::core::api::Error> & c_error) {
    auto jniEnv = ::djinni::jniGetThreadEnv();
    ::djinni::JniLocalScope jscope(jniEnv, 10);
    const auto& data = ::djinni::JniClass<::djinni_generated::CosmosLikeRedelegationListCallback>::get();
    jniEnv->CallVoidMethod(Handle::get().get(), data.method_onCallback,
                           ::djinni::get(::djinni::Optional<std::experimental::optional, ::djinni::List<::djinni_generated::CosmosLikeRedelegation>>::fromCpp(jniEnv, c_result)),
                           ::djinni::get(::djinni::Optional<std::experimental::optional, ::djinni_generated::Error>::fromCpp(jniEnv, c_error)));
    ::djinni::jniExceptionCheck(jniEnv);
}

}  // namespace djinni_generated
