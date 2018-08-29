package co.ledger.core;

import com.facebook.react.bridge.Promise;
/**Class representing context in which tasks get executed */
public class ExecutionContextImpl extends co.ledger.core.ExecutionContext {
    private Promise promise;
    public void setPromise(Promise _promise) {
        this.promise = promise;
    }
    /**
     *Execute a given runnable
     *@param runnalbe, Runnable object
     */
    public void execute(Runnable runnable) {

    }

    /**
     *Execute a given runnable with a delay
     *@param runnalbe, Runnable object
     *@param millis, 64 bits integer, delay in milli-seconds
     */
    public void delay(Runnable runnable, long millis) {

    }
}
