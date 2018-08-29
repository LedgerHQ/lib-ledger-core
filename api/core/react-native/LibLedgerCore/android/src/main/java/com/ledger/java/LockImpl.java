package com.ledger.java;

/**Class representing a lock, for thread safety purposes */
public class LockImpl extends co.ledger.core.Lock {
    /**
     *Acquire lock by thread calling this method,
     *If Lock already acquired by another thread, execution of calling thread should be blocked
     *until the other thread call the unlock method
     */
    public void lock() {

    }

    /**
     *Try to acquire lock
     *If Lock already aquired by another thread, method returns false for calling thread
     *without blocking its execution
     *@return bool, return true if Lock acquire by calling thread, false otherwise
     */
    public boolean tryLock() {
    	return true;
    }

    /**Release Lock ownership by calling thread */
    public void unlock() {

    }
}
