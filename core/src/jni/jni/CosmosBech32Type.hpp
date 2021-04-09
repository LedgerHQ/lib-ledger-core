// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from addresses.djinni

#ifndef DJINNI_GENERATED_COSMOSBECH32TYPE_HPP_JNI_
#define DJINNI_GENERATED_COSMOSBECH32TYPE_HPP_JNI_

#include "../../api/CosmosBech32Type.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class CosmosBech32Type final : ::djinni::JniEnum {
public:
    using CppType = ::ledger::core::api::CosmosBech32Type;
    using JniType = jobject;

    using Boxed = CosmosBech32Type;

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return static_cast<CppType>(::djinni::JniClass<CosmosBech32Type>::get().ordinal(jniEnv, j)); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) { return ::djinni::JniClass<CosmosBech32Type>::get().create(jniEnv, static_cast<jint>(c)); }

private:
    CosmosBech32Type() : JniEnum("co/ledger/core/CosmosBech32Type") {}
    friend ::djinni::JniClass<CosmosBech32Type>;
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_COSMOSBECH32TYPE_HPP_JNI_
