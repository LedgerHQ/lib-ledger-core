package co.ledger.core;

/**Class representing a thread dispatcher */
public class ThreadDispatcherImpl extends ThreadDispatcher {
    /**
     *Get an execution context where tasks are executed sequentially
     *@param name, string, name of execution context to retrieve
     *@return ExecutionContext object
     */
    public ExecutionContext getSerialExecutionContext(String name) {
		return null;
    }

    /**
     *Get an execution context where tasks are executed in parallel thanks to a thread pool
     *where a system of inter-thread communication was designed
     *@param name, string, name of execution context to retrieve
     *@return ExecutionContext object
     */
    public ExecutionContext getThreadPoolExecutionContext(String name) {
		return null;
    }

    /**
     *Get main execution context (generally where tasks that should never get blocked are executed)
     *@return ExecutionContext object
     */
    public ExecutionContext getMainExecutionContext() {
		return null;
    }

    /**
     *Get lock to handle multithreading
     *@return Lock object
     */
    public Lock newLock() {
		return null;
    }
}
