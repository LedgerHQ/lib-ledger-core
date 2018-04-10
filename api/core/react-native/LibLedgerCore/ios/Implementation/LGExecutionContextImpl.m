#import "LGExecutionContextImpl.h"

@implementation LGExecutionContextImpl

/**
*Execute a given runnable
*@param runnalbe, Runnable object
*/
- (void)execute:(nullable LGRunnable *)runnable
{
  [runnable run];
}

/**
*Execute a given runnable with a delay
*@param runnalbe, Runnable object
*@param millis, 64 bits integer, delay in milli-seconds
*/
- (void)delay:(nullable LGRunnable *)runnable
millis:(int64_t)millis
{
  [runnable run];
}

@end