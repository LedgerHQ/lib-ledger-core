// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#ifndef DJINNI_GENERATED_QUERYFILTER_HPP_JNI_
#define DJINNI_GENERATED_QUERYFILTER_HPP_JNI_

#include "../../api/QueryFilter.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class QueryFilter final : ::djinni::JniInterface<::ledger::core::api::QueryFilter, QueryFilter> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::QueryFilter>;
    using CppOptType = std::shared_ptr<::ledger::core::api::QueryFilter>;
    using JniType = jobject;

    using Boxed = QueryFilter;

    ~QueryFilter();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<QueryFilter>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<QueryFilter>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    QueryFilter();
    friend ::djinni::JniClass<QueryFilter>;
    friend ::djinni::JniInterface<::ledger::core::api::QueryFilter, QueryFilter>;

};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_QUERYFILTER_HPP_JNI_
