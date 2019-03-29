// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#ifndef DJINNI_GENERATED_ACCOUNTCREATIONINFOCALLBACK_HPP_JNI_
#define DJINNI_GENERATED_ACCOUNTCREATIONINFOCALLBACK_HPP_JNI_

#include "../../api/AccountCreationInfoCallback.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class AccountCreationInfoCallback final : ::djinni::JniInterface<::ledger::core::api::AccountCreationInfoCallback, AccountCreationInfoCallback> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::AccountCreationInfoCallback>;
    using CppOptType = std::shared_ptr<::ledger::core::api::AccountCreationInfoCallback>;
    using JniType = jobject;

    using Boxed = AccountCreationInfoCallback;

    ~AccountCreationInfoCallback();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<AccountCreationInfoCallback>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<AccountCreationInfoCallback>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    AccountCreationInfoCallback();
    friend ::djinni::JniClass<AccountCreationInfoCallback>;
    friend ::djinni::JniInterface<::ledger::core::api::AccountCreationInfoCallback, AccountCreationInfoCallback>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ledger::core::api::AccountCreationInfoCallback
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void onCallback(const std::ledger_exp::optional<::ledger::core::api::AccountCreationInfo> & result, const std::ledger_exp::optional<::ledger::core::api::Error> & error) override;

    private:
        friend ::djinni::JniInterface<::ledger::core::api::AccountCreationInfoCallback, ::djinni_generated::AccountCreationInfoCallback>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/AccountCreationInfoCallback") };
    const jmethodID method_onCallback { ::djinni::jniGetMethodID(clazz.get(), "onCallback", "(Lco/ledger/core/AccountCreationInfo;Lco/ledger/core/Error;)V") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_ACCOUNTCREATIONINFOCALLBACK_HPP_JNI_
