// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from websocket_client.djinni

#ifndef DJINNI_GENERATED_WEBSOCKETCLIENT_HPP_JNI_
#define DJINNI_GENERATED_WEBSOCKETCLIENT_HPP_JNI_

#include "../../api/WebSocketClient.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class WebSocketClient final : ::djinni::JniInterface<::ledger::core::api::WebSocketClient, WebSocketClient> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::WebSocketClient>;
    using CppOptType = std::shared_ptr<::ledger::core::api::WebSocketClient>;
    using JniType = jobject;

    using Boxed = WebSocketClient;

    ~WebSocketClient();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<WebSocketClient>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<WebSocketClient>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    WebSocketClient();
    friend ::djinni::JniClass<WebSocketClient>;
    friend ::djinni::JniInterface<::ledger::core::api::WebSocketClient, WebSocketClient>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ledger::core::api::WebSocketClient
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        void connect(const std::string & url, const std::shared_ptr<::ledger::core::api::WebSocketConnection> & connection) override;
        void send(const std::shared_ptr<::ledger::core::api::WebSocketConnection> & connection, const std::string & data) override;
        void disconnect(const std::shared_ptr<::ledger::core::api::WebSocketConnection> & connection) override;

    private:
        friend ::djinni::JniInterface<::ledger::core::api::WebSocketClient, ::djinni_generated::WebSocketClient>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/WebSocketClient") };
    const jmethodID method_connect { ::djinni::jniGetMethodID(clazz.get(), "connect", "(Ljava/lang/String;Lco/ledger/core/WebSocketConnection;)V") };
    const jmethodID method_send { ::djinni::jniGetMethodID(clazz.get(), "send", "(Lco/ledger/core/WebSocketConnection;Ljava/lang/String;)V") };
    const jmethodID method_disconnect { ::djinni::jniGetMethodID(clazz.get(), "disconnect", "(Lco/ledger/core/WebSocketConnection;)V") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_WEBSOCKETCLIENT_HPP_JNI_
