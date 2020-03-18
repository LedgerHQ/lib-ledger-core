// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#include "CosmosLikeAccount.hpp"  // my header
#include "AmountCallback.hpp"
#include "BigIntCallback.hpp"
#include "CosmosLikeDelegationListCallback.hpp"
#include "CosmosLikeRewardListCallback.hpp"
#include "CosmosLikeTransaction.hpp"
#include "CosmosLikeTransactionBuilder.hpp"
#include "CosmosLikeValidatorCallback.hpp"
#include "CosmosLikeValidatorListCallback.hpp"
#include "Marshal.hpp"
#include "StringCallback.hpp"

namespace djinni_generated {

CosmosLikeAccount::CosmosLikeAccount() : ::djinni::JniInterface<::ledger::core::api::CosmosLikeAccount, CosmosLikeAccount>("co/ledger/core/CosmosLikeAccount$CppProxy") {}

CosmosLikeAccount::~CosmosLikeAccount() = default;


CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::ledger::core::api::CosmosLikeAccount>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1broadcastRawTransaction(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jstring j_transaction, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->broadcastRawTransaction(::djinni::String::toCpp(jniEnv, j_transaction),
                                     ::djinni_generated::StringCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1broadcastTransaction(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_transaction, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->broadcastTransaction(::djinni_generated::CosmosLikeTransaction::toCpp(jniEnv, j_transaction),
                                  ::djinni_generated::StringCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1buildTransaction(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        auto r = ref->buildTransaction();
        return ::djinni::release(::djinni_generated::CosmosLikeTransactionBuilder::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1getEstimatedGasLimit(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_transaction, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->getEstimatedGasLimit(::djinni_generated::CosmosLikeTransaction::toCpp(jniEnv, j_transaction),
                                  ::djinni_generated::BigIntCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1getLatestValidatorSet(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
CJNIEXPORT jobject JNICALL Java_CosmosLikeAccount_00024CppProxy_native_1getDelegations(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
CJNIEXPORT void JNICALL Java_CosmosLikeAccount_00024CppProxy_native_1getDelegations(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->getLatestValidatorSet(::djinni_generated::CosmosLikeValidatorListCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1getValidatorInfo(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jstring j_validatorAddress, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->getValidatorInfo(::djinni::String::toCpp(jniEnv, j_validatorAddress),
                              ::djinni_generated::CosmosLikeValidatorCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
        auto r = ref->getDelegations();
        return ::djinni::release(::djinni::List<::djinni_generated::CosmosLikeDelegation>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
        ref->getDelegations(::djinni_generated::CosmosLikeDelegationListCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1getTotalBalance(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
CJNIEXPORT void JNICALL Java_CosmosLikeAccount_00024CppProxy_native_1getPendingRewards(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->getTotalBalance(::djinni_generated::AmountCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1getDelegatedBalance(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->getDelegatedBalance(::djinni_generated::AmountCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1getPendingRewardsBalance(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->getPendingRewardsBalance(::djinni_generated::AmountCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1getUnbondingBalance(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->getUnbondingBalance(::djinni_generated::AmountCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_co_ledger_core_CosmosLikeAccount_00024CppProxy_native_1getSpendableBalance(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_callback)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::CosmosLikeAccount>(nativeRef);
        ref->getSpendableBalance(::djinni_generated::AmountCallback::toCpp(jniEnv, j_callback));
        ref->getPendingRewards(::djinni_generated::CosmosLikeRewardListCallback::toCpp(jniEnv, j_callback));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

}  // namespace djinni_generated
