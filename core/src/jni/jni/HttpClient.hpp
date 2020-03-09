// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http_client.djinni

#ifndef DJINNI_GENERATED_HTTPCLIENT_HPP_JNI_
#define DJINNI_GENERATED_HTTPCLIENT_HPP_JNI_

#include "../../api/HttpClient.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class HttpClient final : ::djinni::JniInterface<::ledger::core::api::HttpClient, HttpClient> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::HttpClient>;
    using CppOptType = std::shared_ptr<::ledger::core::api::HttpClient>;
    using JniType = jobject;

    using Boxed = HttpClient;

    ~HttpClient();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<HttpClient>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<HttpClient>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    HttpClient();
    friend ::djinni::JniClass<HttpClient>;
    friend ::djinni::JniInterface<::ledger::core::api::HttpClient, HttpClient>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ledger::core::api::HttpClient
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void execute(const std::shared_ptr<::ledger::core::api::HttpRequest> & request) override;

    private:
        friend ::djinni::JniInterface<::ledger::core::api::HttpClient, ::djinni_generated::HttpClient>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("HttpClient") };
    const jmethodID method_execute { ::djinni::jniGetMethodID(clazz.get(), "execute", "(LHttpRequest;)V") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_HTTPCLIENT_HPP_JNI_
