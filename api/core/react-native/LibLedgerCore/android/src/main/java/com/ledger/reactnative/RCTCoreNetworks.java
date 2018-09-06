// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from networks.djinni

package com.ledger.reactnative;

import co.ledger.core.BitcoinLikeNetworkParameters;
import co.ledger.core.Networks;
import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.WritableNativeArray;
import com.facebook.react.bridge.WritableNativeMap;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

public class RCTCoreNetworks extends ReactContextBaseJavaModule {

    private final ReactApplicationContext reactContext;
    private Map<String, Networks> javaObjects;
    public Map<String, Networks> getJavaObjects()
    {
        return javaObjects;
    }

    public RCTCoreNetworks(ReactApplicationContext reactContext)
    {
        super(reactContext);
        this.reactContext = reactContext;
        this.javaObjects = new HashMap<String, Networks>();
    }

    @Override
    public String getName()
    {
        return "RCTCoreNetworks";
    }
    @ReactMethod
    public void release(Map<String, String> currentInstance, Promise promise)
    {
        String uid = currentInstance.get("uid");
        if (uid.length() > 0)
        {
            this.javaObjects.remove(uid);
            promise.resolve(0);
        }
        else
        {
            promise.reject("Failed to release instance of RCTCoreNetworks", "First parameter of RCTCoreNetworks::release should be an instance of RCTCoreNetworks");
        }
    }
    @ReactMethod
    public void log(Promise promise)
    {
        WritableNativeArray result = new WritableNativeArray();
        for (Map.Entry<String, Networks> elem : this.javaObjects.entrySet())
        {
            result.pushString(elem.getKey());
        }
        promise.resolve(result);
    }
    @ReactMethod
    public void flush(Promise promise)
    {
        this.javaObjects.clear();
        promise.resolve(0);
    }

    @ReactMethod
    public void bitcoin(Promise promise) {
        try
        {
            BitcoinLikeNetworkParameters javaResult = Networks.bitcoin();

            String uuid = UUID.randomUUID().toString();
            RCTCoreBitcoinLikeNetworkParameters rctImpl_javaResult = this.reactContext.getNativeModule(RCTCoreBitcoinLikeNetworkParameters.class);
            rctImpl_javaResult.getJavaObjects().put(uuid, javaResult);
            WritableNativeMap result = new WritableNativeMap();
            result.putString("type","RCTCoreBitcoinLikeNetworkParameters");
            result.putString("uid",uuid);

            promise.resolve(result);
        }
        catch(Exception e)
        {
            promise.reject(e.toString(), e.getMessage());
        }
    }
}
