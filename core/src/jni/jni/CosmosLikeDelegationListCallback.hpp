// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#ifndef DJINNI_GENERATED_COSMOSLIKEDELEGATIONLISTCALLBACK_HPP_JNI_
#define DJINNI_GENERATED_COSMOSLIKEDELEGATIONLISTCALLBACK_HPP_JNI_

#include "../../api/CosmosLikeDelegationListCallback.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class CosmosLikeDelegationListCallback final : ::djinni::JniInterface<::ledger::core::api::CosmosLikeDelegationListCallback, CosmosLikeDelegationListCallback> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::CosmosLikeDelegationListCallback>;
    using CppOptType = std::shared_ptr<::ledger::core::api::CosmosLikeDelegationListCallback>;
    using JniType = jobject;

    using Boxed = CosmosLikeDelegationListCallback;

    ~CosmosLikeDelegationListCallback();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<CosmosLikeDelegationListCallback>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<CosmosLikeDelegationListCallback>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    CosmosLikeDelegationListCallback();
    friend ::djinni::JniClass<CosmosLikeDelegationListCallback>;
    friend ::djinni::JniInterface<::ledger::core::api::CosmosLikeDelegationListCallback, CosmosLikeDelegationListCallback>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ledger::core::api::CosmosLikeDelegationListCallback
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void onCallback(const std::experimental::optional<std::vector<std::shared_ptr<::ledger::core::api::CosmosLikeDelegation>>> & result, const std::experimental::optional<::ledger::core::api::Error> & error) override;

    private:
        friend ::djinni::JniInterface<::ledger::core::api::CosmosLikeDelegationListCallback, ::djinni_generated::CosmosLikeDelegationListCallback>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/CosmosLikeDelegationListCallback") };
    const jmethodID method_onCallback { ::djinni::jniGetMethodID(clazz.get(), "onCallback", "(Ljava/util/ArrayList;Lco/ledger/core/Error;)V") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_COSMOSLIKEDELEGATIONLISTCALLBACK_HPP_JNI_
