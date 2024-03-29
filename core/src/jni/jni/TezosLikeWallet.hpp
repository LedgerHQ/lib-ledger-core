// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from tezos_like_wallet.djinni

#ifndef DJINNI_GENERATED_TEZOSLIKEWALLET_HPP_JNI_
#define DJINNI_GENERATED_TEZOSLIKEWALLET_HPP_JNI_

#include "../../api/TezosLikeWallet.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class TezosLikeWallet final : ::djinni::JniInterface<::ledger::core::api::TezosLikeWallet, TezosLikeWallet> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::TezosLikeWallet>;
    using CppOptType = std::shared_ptr<::ledger::core::api::TezosLikeWallet>;
    using JniType = jobject;

    using Boxed = TezosLikeWallet;

    ~TezosLikeWallet();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<TezosLikeWallet>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<TezosLikeWallet>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    TezosLikeWallet();
    friend ::djinni::JniClass<TezosLikeWallet>;
    friend ::djinni::JniInterface<::ledger::core::api::TezosLikeWallet, TezosLikeWallet>;

};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_TEZOSLIKEWALLET_HPP_JNI_
