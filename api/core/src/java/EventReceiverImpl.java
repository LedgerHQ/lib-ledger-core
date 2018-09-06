package com.ledger.java;
import co.ledger.core.Event;
import co.ledger.core.EventCode;
import co.ledger.core.DynamicObject;

import com.facebook.react.bridge.Promise;
/**Class respresenting an event receiver */
public class EventReceiverImpl extends co.ledger.core.EventReceiver {
    private Promise promise;
    public void setPromise(Promise _promise) {
        this.promise = promise;
    }
    /**
     *Method triggered when an event occurs
     *@param event, Event object that triggers this method
     */
    public void onEvent(Event event) {
        EventCode eventCode = event.getCode();
        DynamicObject payload = event.getPayload();
        if (eventCode == EventCode.UNDEFINED || eventCode == EventCode.SYNCHRONIZATION_FAILED) {
            String error = payload.getString("EV_SYNC_ERROR_MESSAGE");
            if (error.length() == 0) {
                error = "Synchronization Failed";
            }
            this.promise.reject(error, "EventReceiverImpl::onEvent: synchronization failure");
        } else if (eventCode == EventCode.SYNCHRONIZATION_SUCCEED || eventCode == EventCode.SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT) {
            //TODO: check if we should resolve with something else
            this.promise.resolve(0);
        }
    }
}
