package com.ledger.reactnative;

import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import co.ledger.core.*;

public class RNLedgerCoreModule extends ReactContextBaseJavaModule {
  //static {
  //  System.loadLibrary("lib-binding");
  //}
  private final ReactApplicationContext reactContext;
  public BitcoinLikeAccount account;
  public RNLedgerCoreModule(ReactApplicationContext reactContext) {
    super(reactContext);
    this.reactContext = reactContext;
  }

  @Override
  public String getName() {
    return "RNLedgerCore";
  }
}