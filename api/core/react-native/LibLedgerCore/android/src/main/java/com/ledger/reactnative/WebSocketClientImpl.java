package com.ledger.reactnative;

import com.facebook.react.bridge.ReactApplicationContext;

public class WebSocketClientImpl extends co.ledger.core.WebSocketClient {
    private ReactApplicationContext reactContext;
    public WebSocketClientImpl(ReactApplicationContext reactContext) {
        this.reactContext = reactContext;
    }
    public void connect(String url, co.ledger.core.WebSocketConnection connection) {

    }

    public void send(co.ledger.core.WebSocketConnection connection, String data) {

    }

    public void disconnect(co.ledger.core.WebSocketConnection connection) {

    }
}
