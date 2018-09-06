// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from thread_dispatcher.djinni

package com.ledger.reactnative;

import co.ledger.core.Lock;
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

/**Class representing a lock, for thread safety purposes */
public class RCTCoreLock extends ReactContextBaseJavaModule {

    private final ReactApplicationContext reactContext;
    private Map<String, LockImpl> javaObjects;
    public Map<String, LockImpl> getJavaObjects()
    {
        return javaObjects;
    }

    public RCTCoreLock(ReactApplicationContext reactContext)
    {
        super(reactContext);
        this.reactContext = reactContext;
        this.javaObjects = new HashMap<String, LockImpl>();
    }

    @Override
    public String getName()
    {
        return "RCTCoreLock";
    }
    @ReactMethod
    public void newInstance(Promise promise)
    {
        LockImpl newInstance = new LockImpl(this.reactContext);
        String uuid = UUID.randomUUID().toString();
        this.javaObjects.put(uuid, newInstance);
        WritableNativeMap finalResult = new WritableNativeMap();
        finalResult.putString("type","RCTCoreLock");
        finalResult.putString("uid",uuid);
        promise.resolve(finalResult);
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
            promise.reject("Failed to release instance of RCTCoreLock", "First parameter of RCTCoreLock::release should be an instance of RCTCoreLock");
        }
    }
    @ReactMethod
    public void log(Promise promise)
    {
        WritableNativeArray result = new WritableNativeArray();
        for (Map.Entry<String, LockImpl> elem : this.javaObjects.entrySet())
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

    /**
     *Acquire lock by thread calling this method,
     *If Lock already acquired by another thread, execution of calling thread should be blocked
     *until the other thread call the unlock method
     */
    @ReactMethod
    public void lock(Map<String, String> currentInstance, Promise promise) {
        try
        {
            String sUid = currentInstance.get("uid");

            LockImpl currentInstanceObj = this.javaObjects.get(sUid);

            currentInstanceObj.lock();
        }
        catch(Exception e)
        {
            promise.reject(e.toString(), e.getMessage());
        }
    }
    /**
     *Try to acquire lock
     *If Lock already aquired by another thread, method returns false for calling thread
     *without blocking its execution
     *@return bool, return true if Lock acquire by calling thread, false otherwise
     */
    @ReactMethod
    public void tryLock(Map<String, String> currentInstance, Promise promise) {
        try
        {
            String sUid = currentInstance.get("uid");

            LockImpl currentInstanceObj = this.javaObjects.get(sUid);

            boolean javaResult = currentInstanceObj.tryLock();
            WritableNativeMap result = new WritableNativeMap();
            result.putBoolean("value", javaResult);

            promise.resolve(result);
        }
        catch(Exception e)
        {
            promise.reject(e.toString(), e.getMessage());
        }
    }
    /**Release Lock ownership by calling thread */
    @ReactMethod
    public void unlock(Map<String, String> currentInstance, Promise promise) {
        try
        {
            String sUid = currentInstance.get("uid");

            LockImpl currentInstanceObj = this.javaObjects.get(sUid);

            currentInstanceObj.unlock();
        }
        catch(Exception e)
        {
            promise.reject(e.toString(), e.getMessage());
        }
    }
}
