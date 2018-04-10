#import "LGLockImpl.h"

@implementation LGLockImpl

/**
*Acquire lock by thread calling this method,
*If Lock already acquired by another thread, execution of calling thread should be blocked
*until the other thread call the unlock method
*/
- (void)lock
{}

/**
*Try to acquire lock
*If Lock already aquired by another thread, method returns false for calling thread
*without blocking its execution
*@return bool, return true if Lock acquire by calling thread, false otherwise
*/
- (BOOL)tryLock
{}

/**Release Lock ownership by calling thread */
- (void)unlock
{}

@end