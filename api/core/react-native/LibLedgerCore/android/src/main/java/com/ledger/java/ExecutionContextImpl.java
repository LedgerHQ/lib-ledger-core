package com.ledger.java;

import com.facebook.react.bridge.Promise;
import co.ledger.core.Runnable;
import java.util.Deque;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.concurrent.SynchronousQueue;

/**Class representing context in which tasks get executed */
public class ExecutionContextImpl extends co.ledger.core.ExecutionContext {
    private Queue<co.ledger.core.Runnable> queue;
    private Promise promise;

    public ExecutionContextImpl(String name) {
        if (name == "__main__") {
            this.queue = new PriorityQueue<co.ledger.core.Runnable>();
        } else {
            this.queue = new SynchronousQueue<co.ledger.core.Runnable>();
        }
    }

    public void setPromise(Promise _promise) {
        this.promise = promise;
    }
    /**
     *Execute a given runnable
     *@param runnalbe, Runnable object
     */
    public void execute(final co.ledger.core.Runnable runnable) {
        this.queue.add(runnable);
    }

    /**
     *Execute a given runnable with a delay
     *@param runnalbe, Runnable object
     *@param millis, 64 bits integer, delay in milli-seconds
     */
    public void delay(Runnable runnable, long millis) {
        try {
            Thread.sleep(millis);
            this.queue.add(runnable);
        } catch(Exception e) {
            System.err.println(e);
        }
    }
}
