// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from messages.djinni

#ifndef DJINNI_GENERATED_COSMOSLIKEVALIDATORDESCRIPTION_HPP_JNI_
#define DJINNI_GENERATED_COSMOSLIKEVALIDATORDESCRIPTION_HPP_JNI_

#include "../../api/CosmosLikeValidatorDescription.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class CosmosLikeValidatorDescription final {
public:
    using CppType = ::ledger::core::api::CosmosLikeValidatorDescription;
    using JniType = jobject;

    using Boxed = CosmosLikeValidatorDescription;

    ~CosmosLikeValidatorDescription();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    CosmosLikeValidatorDescription();
    friend ::djinni::JniClass<CosmosLikeValidatorDescription>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/CosmosLikeValidatorDescription") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V") };
    const jfieldID field_moniker { ::djinni::jniGetFieldID(clazz.get(), "moniker", "Ljava/lang/String;") };
    const jfieldID field_identity { ::djinni::jniGetFieldID(clazz.get(), "identity", "Ljava/lang/String;") };
    const jfieldID field_website { ::djinni::jniGetFieldID(clazz.get(), "website", "Ljava/lang/String;") };
    const jfieldID field_details { ::djinni::jniGetFieldID(clazz.get(), "details", "Ljava/lang/String;") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_COSMOSLIKEVALIDATORDESCRIPTION_HPP_JNI_