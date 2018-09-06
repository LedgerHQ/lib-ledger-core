package com.ledger.java;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**Class representing a thread dispatcher */
public class ThreadDispatcherImpl extends co.ledger.core.ThreadDispatcher {
    private Map<String, ExecutionContextImpl> contexts;
    public ThreadDispatcherImpl() {
        this.contexts = new HashMap<String, ExecutionContextImpl>();
    }
    /**
     *Get an execution context where tasks are executed sequentially
     *@param name, string, name of execution context to retrieve
     *@return ExecutionContext object
     */
    public co.ledger.core.ExecutionContext getSerialExecutionContext(String name) {
        ExecutionContextImpl context = this.contexts.get(name);
        if (context == null) {
            context = new ExecutionContextImpl(name);
        }
        return context;
    }

    /**
     *Get an execution context where tasks are executed in parallel thanks to a thread pool
     *where a system of inter-thread communication was designed
     *@param name, string, name of execution context to retrieve
     *@return ExecutionContext object
     */
    public co.ledger.core.ExecutionContext getThreadPoolExecutionContext(String name) {

        return null;
    }

    /**
     *Get main execution context (generally where tasks that should never get blocked are executed)
     *@return ExecutionContext object
     */
    public co.ledger.core.ExecutionContext getMainExecutionContext() {

        return null;
    }

    /**
     *Get lock to handle multithreading
     *@return Lock object
     */
    public co.ledger.core.Lock newLock() {

        return null;
    }
}
