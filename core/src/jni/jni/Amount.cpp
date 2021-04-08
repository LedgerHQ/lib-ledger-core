// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from amount.djinni

#include "Amount.hpp"  // my header
#include "BigInt.hpp"
#include "Currency.hpp"
#include "CurrencyUnit.hpp"
#include "FormatRules.hpp"
#include "Locale.hpp"
#include "Marshal.hpp"

namespace djinni_generated {

Amount::Amount() : ::djinni::JniInterface<::ledger::core::api::Amount, Amount>("Amount$CppProxy") {}

Amount::~Amount() = default;


CJNIEXPORT void JNICALL Java_Amount_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::ledger::core::api::Amount>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_Amount_00024CppProxy_native_1toBigInt(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->toBigInt();
        return ::djinni::release(::djinni_generated::BigInt::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_Amount_00024CppProxy_native_1getCurrency(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->getCurrency();
        return ::djinni::release(::djinni_generated::Currency::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_Amount_00024CppProxy_native_1getUnit(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->getUnit();
        return ::djinni::release(::djinni_generated::CurrencyUnit::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_Amount_00024CppProxy_native_1toUnit(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_unit)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->toUnit(::djinni_generated::CurrencyUnit::toCpp(jniEnv, j_unit));
        return ::djinni::release(::djinni_generated::Amount::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_Amount_00024CppProxy_native_1toMagnitude(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jint j_magnitude)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->toMagnitude(::djinni::I32::toCpp(jniEnv, j_magnitude));
        return ::djinni::release(::djinni_generated::Amount::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_Amount_00024CppProxy_native_1toString(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->toString();
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jlong JNICALL Java_Amount_00024CppProxy_native_1toLong(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->toLong();
        return ::djinni::release(::djinni::I64::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jdouble JNICALL Java_Amount_00024CppProxy_native_1toDouble(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->toDouble();
        return ::djinni::release(::djinni::F64::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_Amount_00024CppProxy_native_1format(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_locale, jobject j_rules)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::ledger::core::api::Amount>(nativeRef);
        auto r = ref->format(::djinni_generated::Locale::toCpp(jniEnv, j_locale),
                             ::djinni::Optional<std::experimental::optional, ::djinni_generated::FormatRules>::toCpp(jniEnv, j_rules));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_Amount_fromHex(JNIEnv* jniEnv, jobject /*this*/, jobject j_currency, jstring j_hex)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::ledger::core::api::Amount::fromHex(::djinni_generated::Currency::toCpp(jniEnv, j_currency),
                                                      ::djinni::String::toCpp(jniEnv, j_hex));
        return ::djinni::release(::djinni_generated::Amount::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_Amount_fromLong(JNIEnv* jniEnv, jobject /*this*/, jobject j_currency, jlong j_value)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::ledger::core::api::Amount::fromLong(::djinni_generated::Currency::toCpp(jniEnv, j_currency),
                                                       ::djinni::I64::toCpp(jniEnv, j_value));
        return ::djinni::release(::djinni_generated::Amount::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

}  // namespace djinni_generated
