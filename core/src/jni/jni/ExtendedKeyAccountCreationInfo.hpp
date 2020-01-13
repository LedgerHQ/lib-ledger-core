// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#ifndef DJINNI_GENERATED_EXTENDEDKEYACCOUNTCREATIONINFO_HPP_JNI_
#define DJINNI_GENERATED_EXTENDEDKEYACCOUNTCREATIONINFO_HPP_JNI_

#include "../../api/ExtendedKeyAccountCreationInfo.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class ExtendedKeyAccountCreationInfo final {
public:
    using CppType = ::ledger::core::api::ExtendedKeyAccountCreationInfo;
    using JniType = jobject;

    using Boxed = ExtendedKeyAccountCreationInfo;

    ~ExtendedKeyAccountCreationInfo();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    ExtendedKeyAccountCreationInfo();
    friend ::djinni::JniClass<ExtendedKeyAccountCreationInfo>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("ExtendedKeyAccountCreationInfo") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(ILjava/util/ArrayList;Ljava/util/ArrayList;Ljava/util/ArrayList;)V") };
    const jfieldID field_index { ::djinni::jniGetFieldID(clazz.get(), "index", "I") };
    const jfieldID field_owners { ::djinni::jniGetFieldID(clazz.get(), "owners", "Ljava/util/ArrayList;") };
    const jfieldID field_derivations { ::djinni::jniGetFieldID(clazz.get(), "derivations", "Ljava/util/ArrayList;") };
    const jfieldID field_extendedKeys { ::djinni::jniGetFieldID(clazz.get(), "extendedKeys", "Ljava/util/ArrayList;") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_EXTENDEDKEYACCOUNTCREATIONINFO_HPP_JNI_
