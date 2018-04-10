#import "LGThreadDispatcherImpl.h"

@implementation LGThreadDispatcherImpl

/**
*Get an execution context where tasks are executed sequentially
*@param name, string, name of execution context to retrieve
*@return ExecutionContext object
*/
- (nullable id<LGExecutionContext>)getSerialExecutionContext:(nonnull NSString *)name
{
  return nil;
}

/**
*Get an execution context where tasks are executed in parallel thanks to a thread pool
*where a system of inter-thread communication was designed
*@param name, string, name of execution context to retrieve
*@return ExecutionContext object
*/
- (nullable id<LGExecutionContext>)getThreadPoolExecutionContext:(nonnull NSString *)name
{
  return nil;
}

/**
*Get main execution context (generally where tasks that should never get blocked are executed)
*@return ExecutionContext object
*/
- (nullable id<LGExecutionContext>)getMainExecutionContext
{
  return nil;
}

/**
*Get lock to handle multithreading
*@return Lock object
*/
- (nullable id<LGLock>)newLock
{
  return nil;
}

@end