package com.ledger.reactnative;


import com.facebook.react.bridge.ReactApplicationContext;

public class GetEthreumLikeWalletCallbackImpl extends co.ledger.core.GetEthreumLikeWalletCallback {

    private ReactApplicationContext reactContext;
    public GetEthreumLikeWalletCallbackImpl(ReactApplicationContext reactContext) {
        this.reactContext = reactContext;
    }
    public void onSuccess(co.ledger.core.EthereumLikeWallet wallet, boolean isCreated) {

    }

    public void onError(co.ledger.core.Error error) {

    }
}
