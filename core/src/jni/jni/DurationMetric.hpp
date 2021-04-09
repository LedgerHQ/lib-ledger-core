// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from core.djinni

#ifndef DJINNI_GENERATED_DURATIONMETRIC_HPP_JNI_
#define DJINNI_GENERATED_DURATIONMETRIC_HPP_JNI_

#include "../../api/DurationMetric.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class DurationMetric final {
public:
    using CppType = ::ledger::core::api::DurationMetric;
    using JniType = jobject;

    using Boxed = DurationMetric;

    ~DurationMetric();

    static CppType toCpp(JNIEnv* jniEnv, JniType j);
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c);

private:
    DurationMetric();
    friend ::djinni::JniClass<DurationMetric>;

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/DurationMetric") };
    const jmethodID jconstructor { ::djinni::jniGetMethodID(clazz.get(), "<init>", "(JI)V") };
    const jfieldID field_totalMs { ::djinni::jniGetFieldID(clazz.get(), "totalMs", "J") };
    const jfieldID field_count { ::djinni::jniGetFieldID(clazz.get(), "count", "I") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_DURATIONMETRIC_HPP_JNI_
