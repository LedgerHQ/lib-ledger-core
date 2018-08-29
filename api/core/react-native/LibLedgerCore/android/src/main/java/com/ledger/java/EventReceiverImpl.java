package co.ledger.core;

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

    }
}
