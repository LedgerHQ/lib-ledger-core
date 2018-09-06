package com.ledger.reactnative;

import com.facebook.react.bridge.ReactApplicationContext;

public class EthereumPublicKeyProviderImpl extends co.ledger.core.EthereumPublicKeyProvider {
    private ReactApplicationContext reactContext;
    public EthereumPublicKeyProviderImpl(ReactApplicationContext reactContext) {
        this.reactContext = reactContext;
    }
}
