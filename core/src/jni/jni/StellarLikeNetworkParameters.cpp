// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from network.djinni

#include "StellarLikeNetworkParameters.hpp"  // my header
#include "Marshal.hpp"

namespace djinni_generated {

StellarLikeNetworkParameters::StellarLikeNetworkParameters() = default;

StellarLikeNetworkParameters::~StellarLikeNetworkParameters() = default;

auto StellarLikeNetworkParameters::fromCpp(JNIEnv* jniEnv, const CppType& c) -> ::djinni::LocalRef<JniType> {
    const auto& data = ::djinni::JniClass<StellarLikeNetworkParameters>::get();
    auto r = ::djinni::LocalRef<JniType>{jniEnv->NewObject(data.clazz.get(), data.jconstructor,
                                                           ::djinni::get(::djinni::String::fromCpp(jniEnv, c.Identifier)))};
    ::djinni::jniExceptionCheck(jniEnv);
    return r;
}

auto StellarLikeNetworkParameters::toCpp(JNIEnv* jniEnv, JniType j) -> CppType {
    ::djinni::JniLocalScope jscope(jniEnv, 2);
    assert(j != nullptr);
    const auto& data = ::djinni::JniClass<StellarLikeNetworkParameters>::get();
    return {::djinni::String::toCpp(jniEnv, (jstring)jniEnv->GetObjectField(j, data.field_Identifier))};
}

}  // namespace djinni_generated
