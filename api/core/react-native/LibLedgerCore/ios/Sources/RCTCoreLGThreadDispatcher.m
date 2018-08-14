// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from thread_dispatcher.djinni

#import "RCTCoreLGThreadDispatcher.h"


@implementation RCTCoreLGThreadDispatcher
//Export module
RCT_EXPORT_MODULE(RCTCoreLGThreadDispatcher)

@synthesize bridge = _bridge;

-(instancetype)init
{
    self = [super init];
    //Init Objc implementation
    if(self)
    {
        self.objcImplementations = [[NSMutableDictionary alloc] init];
    }
    return self;
}

/**
 *Get an execution context where tasks are executed sequentially
 *@param name, string, name of execution context to retrieve
 *@return ExecutionContext object
 */
RCT_REMAP_METHOD(getSerialExecutionContext,getSerialExecutionContext:(NSDictionary *)currentInstance withParams:(nonnull NSString *)name withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGThreadDispatcher::getSerialExecutionContext, first argument should be an instance of LGThreadDispatcherImpl", nil);
    }
    LGThreadDispatcherImpl *currentInstanceObj = [self.objcImplementations objectForKey:currentInstance[@"uid"]];
    if (!currentInstanceObj)
    {
        NSString *error = [NSString stringWithFormat:@"Error while calling LGThreadDispatcherImpl::getSerialExecutionContext, instance of uid %@ not found", currentInstance[@"uid"]];
        reject(@"impl_call_error", error, nil);
    }
    id<LGExecutionContext> objcResult = [currentInstanceObj getSerialExecutionContext:name];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGExecutionContext *rctImpl_objcResult = (RCTCoreLGExecutionContext *)[self.bridge moduleForName:@"CoreLGExecutionContext"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGExecutionContext", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGThreadDispatcherImpl::getSerialExecutionContext", nil);
    }

}

/**
 *Get an execution context where tasks are executed in parallel thanks to a thread pool
 *where a system of inter-thread communication was designed
 *@param name, string, name of execution context to retrieve
 *@return ExecutionContext object
 */
RCT_REMAP_METHOD(getThreadPoolExecutionContext,getThreadPoolExecutionContext:(NSDictionary *)currentInstance withParams:(nonnull NSString *)name withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGThreadDispatcher::getThreadPoolExecutionContext, first argument should be an instance of LGThreadDispatcherImpl", nil);
    }
    LGThreadDispatcherImpl *currentInstanceObj = [self.objcImplementations objectForKey:currentInstance[@"uid"]];
    if (!currentInstanceObj)
    {
        NSString *error = [NSString stringWithFormat:@"Error while calling LGThreadDispatcherImpl::getThreadPoolExecutionContext, instance of uid %@ not found", currentInstance[@"uid"]];
        reject(@"impl_call_error", error, nil);
    }
    id<LGExecutionContext> objcResult = [currentInstanceObj getThreadPoolExecutionContext:name];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGExecutionContext *rctImpl_objcResult = (RCTCoreLGExecutionContext *)[self.bridge moduleForName:@"CoreLGExecutionContext"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGExecutionContext", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGThreadDispatcherImpl::getThreadPoolExecutionContext", nil);
    }

}

/**
 *Get main execution context (generally where tasks that should never get blocked are executed)
 *@return ExecutionContext object
 */
RCT_REMAP_METHOD(getMainExecutionContext,getMainExecutionContext:(NSDictionary *)currentInstance WithResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGThreadDispatcher::getMainExecutionContext, first argument should be an instance of LGThreadDispatcherImpl", nil);
    }
    LGThreadDispatcherImpl *currentInstanceObj = [self.objcImplementations objectForKey:currentInstance[@"uid"]];
    if (!currentInstanceObj)
    {
        NSString *error = [NSString stringWithFormat:@"Error while calling LGThreadDispatcherImpl::getMainExecutionContext, instance of uid %@ not found", currentInstance[@"uid"]];
        reject(@"impl_call_error", error, nil);
    }
    id<LGExecutionContext> objcResult = [currentInstanceObj getMainExecutionContext];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGExecutionContext *rctImpl_objcResult = (RCTCoreLGExecutionContext *)[self.bridge moduleForName:@"CoreLGExecutionContext"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGExecutionContext", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGThreadDispatcherImpl::getMainExecutionContext", nil);
    }

}

/**
 *Get lock to handle multithreading
 *@return Lock object
 */
RCT_REMAP_METHOD(newLock,newLock:(NSDictionary *)currentInstance WithResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGThreadDispatcher::newLock, first argument should be an instance of LGThreadDispatcherImpl", nil);
    }
    LGThreadDispatcherImpl *currentInstanceObj = [self.objcImplementations objectForKey:currentInstance[@"uid"]];
    if (!currentInstanceObj)
    {
        NSString *error = [NSString stringWithFormat:@"Error while calling LGThreadDispatcherImpl::newLock, instance of uid %@ not found", currentInstance[@"uid"]];
        reject(@"impl_call_error", error, nil);
    }
    id<LGLock> objcResult = [currentInstanceObj newLock];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGLock *rctImpl_objcResult = (RCTCoreLGLock *)[self.bridge moduleForName:@"CoreLGLock"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGLock", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGThreadDispatcherImpl::newLock", nil);
    }

}
RCT_REMAP_METHOD(new, newWithResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGThreadDispatcherImpl *objcResult = [[LGThreadDispatcherImpl alloc] init];
    NSString *uuid = [[NSUUID UUID] UUIDString];
    [self.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGThreadDispatcherImpl", @"uid" : uuid };
    if (!objcResult || !result)
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGThreadDispatcherImpl::init", nil);
    }
    resolve(result);
}
@end
