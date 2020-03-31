// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#include "CosmosLikeUnbondingEntry.hpp"  // my header
#include "BigInt.hpp"
#include "Marshal.hpp"

namespace djinni_generated {

CosmosLikeUnbondingEntry::CosmosLikeUnbondingEntry() : ::djinni::JniInterface<::ledger::core::api::CosmosLikeUnbondingEntry, CosmosLikeUnbondingEntry>("co/ledger/core/CosmosLikeUnbondingEntry$CppProxy") {}

CosmosLikeUnbondingEntry::~CosmosLikeUnbondingEntry() = default;


CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeUnbondingEntry_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::ledger::core::api::CosmosLikeUnbondingEntry>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jint JNICALL Java_co_ledger_core_CosmosLikeUnbondingEntry_00024CppProxy_native_1getCreationHeight(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeUnbondingEntry>(nativeRef);
        auto r = ref->getCreationHeight();
        return ::djinni::release(::djinni::I32::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_CosmosLikeUnbondingEntry_00024CppProxy_native_1getCompletionTime(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeUnbondingEntry>(nativeRef);
        auto r = ref->getCompletionTime();
        return ::djinni::release(::djinni::Date::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_CosmosLikeUnbondingEntry_00024CppProxy_native_1getInitialBalance(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeUnbondingEntry>(nativeRef);
        auto r = ref->getInitialBalance();
        return ::djinni::release(::djinni_generated::BigInt::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_CosmosLikeUnbondingEntry_00024CppProxy_native_1getBalance(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeUnbondingEntry>(nativeRef);
        auto r = ref->getBalance();
        return ::djinni::release(::djinni_generated::BigInt::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

}  // namespace djinni_generated
