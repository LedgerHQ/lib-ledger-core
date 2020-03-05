// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#include "CosmosLikeTransactionBuilder.hpp"  // my header
#include "Amount.hpp"
#include "CosmosLikeMessage.hpp"
#include "CosmosLikeTransaction.hpp"
#include "CosmosLikeTransactionCallback.hpp"
#include "Currency.hpp"
#include "Marshal.hpp"

namespace djinni_generated {

CosmosLikeTransactionBuilder::CosmosLikeTransactionBuilder() : ::djinni::JniInterface<::ledger::core::api::CosmosLikeTransactionBuilder, CosmosLikeTransactionBuilder>("CosmosLikeTransactionBuilder$CppProxy") {}

CosmosLikeTransactionBuilder::~CosmosLikeTransactionBuilder() = default;


CJNIEXPORT void JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::ledger::core::api::CosmosLikeTransactionBuilder>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_native_1setMemo(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jstring j_memo)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeTransactionBuilder>(nativeRef);
        auto r = ref->setMemo(::djinni::String::toCpp(jniEnv, j_memo));
        return ::djinni::release(::djinni_generated::CosmosLikeTransactionBuilder::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_native_1setSequence(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jstring j_sequence)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeTransactionBuilder>(nativeRef);
        auto r = ref->setSequence(::djinni::String::toCpp(jniEnv, j_sequence));
        return ::djinni::release(::djinni_generated::CosmosLikeTransactionBuilder::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_native_1addMessage(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_msg)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeTransactionBuilder>(nativeRef);
        auto r = ref->addMessage(::djinni_generated::CosmosLikeMessage::toCpp(jniEnv, j_msg));
        return ::djinni::release(::djinni_generated::CosmosLikeTransactionBuilder::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_native_1setGas(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_gas)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeTransactionBuilder>(nativeRef);
        auto r = ref->setGas(::djinni_generated::Amount::toCpp(jniEnv, j_gas));
        return ::djinni::release(::djinni_generated::CosmosLikeTransactionBuilder::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_native_1setFee(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_fee)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeTransactionBuilder>(nativeRef);
        auto r = ref->setFee(::djinni_generated::Amount::toCpp(jniEnv, j_fee));
        return ::djinni::release(::djinni_generated::CosmosLikeTransactionBuilder::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_native_1build(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeTransactionBuilder>(nativeRef);
        ref->build(::djinni_generated::CosmosLikeTransactionCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_native_1clone(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeTransactionBuilder>(nativeRef);
        auto r = ref->clone();
        return ::djinni::release(::djinni_generated::CosmosLikeTransactionBuilder::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_CosmosLikeTransactionBuilder_00024CppProxy_native_1reset(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeTransactionBuilder>(nativeRef);
        ref->reset();
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_CosmosLikeTransactionBuilder_parseRawUnsignedTransaction(JNIEnv* jniEnv, jobject /*this*/, jobject j_currency, jstring j_rawTransaction)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::ledger::core::api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(::djinni_generated::Currency::toCpp(jniEnv, j_currency),
                                                                                                ::djinni::String::toCpp(jniEnv, j_rawTransaction));
        return ::djinni::release(::djinni_generated::CosmosLikeTransaction::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_CosmosLikeTransactionBuilder_parseRawSignedTransaction(JNIEnv* jniEnv, jobject /*this*/, jobject j_currency, jstring j_rawTransaction)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::ledger::core::api::CosmosLikeTransactionBuilder::parseRawSignedTransaction(::djinni_generated::Currency::toCpp(jniEnv, j_currency),
                                                                                              ::djinni::String::toCpp(jniEnv, j_rawTransaction));
        return ::djinni::release(::djinni_generated::CosmosLikeTransaction::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

}  // namespace djinni_generated
