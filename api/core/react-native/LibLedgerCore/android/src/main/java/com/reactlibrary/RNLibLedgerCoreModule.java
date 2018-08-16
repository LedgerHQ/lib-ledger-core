
package com.reactlibrary;

import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.Callback;
import co.ledger.core.*;

public class RNLibLedgerCoreModule extends ReactContextBaseJavaModule {

  private final ReactApplicationContext reactContext;
  private co.ledger.core.AccountCreationInfo mClass;
  public RNLibLedgerCoreModule(ReactApplicationContext reactContext) {
    super(reactContext);
    this.reactContext = reactContext;
  }

  @Override
  public String getName() {
    return "RNLibLedgerCore";
  }
}