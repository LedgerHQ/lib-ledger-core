// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from tezos_like_wallet.djinni

#include "TezosLikeTransaction.hpp"  // my header
#include "Amount.hpp"
#include "BigInt.hpp"
#include "Marshal.hpp"
#include "TezosLikeAddress.hpp"
#include "TezosOperationTag.hpp"

namespace djinni_generated {

TezosLikeTransaction::TezosLikeTransaction() : ::djinni::JniInterface<::ledger::core::api::TezosLikeTransaction, TezosLikeTransaction>("co/ledger/core/TezosLikeTransaction$CppProxy") {}

TezosLikeTransaction::~TezosLikeTransaction() = default;


CJNIEXPORT void JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::ledger::core::api::TezosLikeTransaction>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getType(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getType();
        return ::djinni::release(::djinni_generated::TezosOperationTag::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getHash(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getHash();
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getFees(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getFees();
        return ::djinni::release(::djinni_generated::Amount::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getReceiver(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getReceiver();
        return ::djinni::release(::djinni::Optional<std::experimental::optional, ::djinni_generated::TezosLikeAddress>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getSender(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getSender();
        return ::djinni::release(::djinni_generated::TezosLikeAddress::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getValue(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getValue();
        return ::djinni::release(::djinni::Optional<std::experimental::optional, ::djinni_generated::Amount>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jbyteArray JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1serialize(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->serialize();
        return ::djinni::release(::djinni::Binary::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1setSignature(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jbyteArray j_rSignature, jbyteArray j_sSignature)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        ref->setSignature(::djinni::Binary::toCpp(jniEnv, j_rSignature),
                          ::djinni::Binary::toCpp(jniEnv, j_sSignature));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1setDERSignature(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jbyteArray j_signature)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        ref->setDERSignature(::djinni::Binary::toCpp(jniEnv, j_signature));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getDate(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getDate();
        return ::djinni::release(::djinni::Date::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jbyteArray JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getSigningPubKey(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getSigningPubKey();
        return ::djinni::release(::djinni::Binary::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getCounter(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getCounter();
        return ::djinni::release(::djinni_generated::BigInt::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getGasLimit(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getGasLimit();
        return ::djinni::release(::djinni_generated::Amount::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getStorageLimit(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getStorageLimit();
        return ::djinni::release(::djinni_generated::Amount::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_co_ledger_core_TezosLikeTransaction_00024CppProxy_native_1getBlockHash(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::TezosLikeTransaction>(nativeRef);
        auto r = ref->getBlockHash();
        return ::djinni::release(::djinni::Optional<std::experimental::optional, ::djinni::String>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

}  // namespace djinni_generated
