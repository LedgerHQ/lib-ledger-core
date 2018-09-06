#import "LGThreadDispatcherImpl.h"
#import "LGExecutionContextImpl.h"

@implementation LGThreadDispatcherImpl

-(instancetype) init {
    self = [super init];
    if (self) {
        self.contexts = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (id<LGExecutionContext>)getExecutionContext:(BOOL)isSerialized withName:(NSString *)name
{
    id<LGExecutionContext> context = [self.contexts objectForKey:name];
    if (context) {
        return context;
    }
    LGExecutionContextImpl *newContext = [[LGExecutionContextImpl alloc] initContext:isSerialized];
    [self.contexts setObject:newContext forKey:name];
    return newContext;
}
/**
*Get an execution context where tasks are executed sequentially
*@param name, string, name of execution context to retrieve
*@return ExecutionContext object
*/
- (nullable id<LGExecutionContext>)getSerialExecutionContext:(nonnull NSString *)name
{
    BOOL isSerialized = YES;
    return [self getExecutionContext:isSerialized withName:name];
}

/**
*Get an execution context where tasks are executed in parallel thanks to a thread pool
*where a system of inter-thread communication was designed
*@param name, string, name of execution context to retrieve
*@return ExecutionContext object
*/
- (nullable id<LGExecutionContext>)getThreadPoolExecutionContext:(nonnull NSString *)name
{
    //TODO: implement thread pool execution context, right now we take a serialized one
    BOOL isSerialized = YES;
    return [self getExecutionContext:isSerialized withName:name];
}

/**
*Get main execution context (generally where tasks that should never get blocked are executed)
*@return ExecutionContext object
*/
- (nullable id<LGExecutionContext>)getMainExecutionContext
{
    BOOL isSerialized = NO;
    return [self getExecutionContext:isSerialized withName:@"__main__"];
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
