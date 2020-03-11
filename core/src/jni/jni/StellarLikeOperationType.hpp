// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from stellar_like_wallet.djinni

#ifndef DJINNI_GENERATED_STELLARLIKEOPERATIONTYPE_HPP_JNI_
#define DJINNI_GENERATED_STELLARLIKEOPERATIONTYPE_HPP_JNI_

#include "../../api/StellarLikeOperationType.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class StellarLikeOperationType final : ::djinni::JniEnum {
public:
    using CppType = ::ledger::core::api::StellarLikeOperationType;
    using JniType = jobject;

    using Boxed = StellarLikeOperationType;

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return static_cast<CppType>(::djinni::JniClass<StellarLikeOperationType>::get().ordinal(jniEnv, j)); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) { return ::djinni::JniClass<StellarLikeOperationType>::get().create(jniEnv, static_cast<jint>(c)); }

private:
    StellarLikeOperationType() : JniEnum("co/ledger/core/StellarLikeOperationType") {}
    friend ::djinni::JniClass<StellarLikeOperationType>;
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_STELLARLIKEOPERATIONTYPE_HPP_JNI_
