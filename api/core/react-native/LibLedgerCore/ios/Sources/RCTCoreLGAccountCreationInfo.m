// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#import "RCTCoreLGAccountCreationInfo.h"
#import "LGAccountCreationInfo.h"

@implementation RCTCoreLGAccountCreationInfo

//Export module
RCT_EXPORT_MODULE(RCTCoreLGAccountCreationInfo)

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

+ (BOOL)requiresMainQueueSetup
{
    return NO;
}
RCT_REMAP_METHOD(release, release:(NSDictionary *)currentInstance withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject)
{
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGAccountCreationInfo::release, first argument should be an instance of LGAccountCreationInfo", nil);
    }
    [self.objcImplementations removeObjectForKey:currentInstance[@"uid"]];
    resolve(@(YES));
}
RCT_REMAP_METHOD(log, logWithResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject)
{
    NSMutableArray *uuids = [[NSMutableArray alloc] init];
    for (id key in self.objcImplementations)
    {
        [uuids addObject:key];
    }
    NSDictionary *result = @{@"value" : uuids};
    resolve(result);
}
RCT_REMAP_METHOD(flush, flushWithResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject)
{
    [self.objcImplementations removeAllObjects];
    resolve(@(YES));
}
RCT_REMAP_METHOD(init, initWithIndex:(int)index
                              owners:(nonnull NSArray<NSString *> *)owners
                         derivations:(nonnull NSArray<NSString *> *)derivations
                          publicKeys:(nonnull NSArray<NSData *> *)publicKeys
                          chainCodes:(nonnull NSArray<NSData *> *)chainCodes withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {


    LGAccountCreationInfo * finalResult = [[LGAccountCreationInfo alloc] initWithIndex:index owners:owners derivations:derivations publicKeys:publicKeys chainCodes:chainCodes];
    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGAccountCreationInfo *rctImpl = (RCTCoreLGAccountCreationInfo *)[self.bridge moduleForName:@"CoreLGAccountCreationInfo"];
    [rctImpl.objcImplementations setObject:finalResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGAccountCreationInfo", @"uid" : uuid };
    if (result)
    {
        resolve(result);
    }
}

RCT_REMAP_METHOD(getIndex, getIndex:(NSDictionary *)currentInstance withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)rejecter)
{
    LGAccountCreationInfo *objcImpl = (LGAccountCreationInfo *)[self.objcImplementations objectForKey:currentInstance[@"uid"]];
    NSDictionary *result = @{@"value" : @((int)objcImpl.index)};
    resolve(result);
}

RCT_REMAP_METHOD(getOwners, getOwners:(NSDictionary *)currentInstance withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)rejecter)
{
    LGAccountCreationInfo *objcImpl = (LGAccountCreationInfo *)[self.objcImplementations objectForKey:currentInstance[@"uid"]];
    NSDictionary *result = @{@"value" : objcImpl.owners};
    resolve(result);
}

RCT_REMAP_METHOD(getDerivations, getDerivations:(NSDictionary *)currentInstance withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)rejecter)
{
    LGAccountCreationInfo *objcImpl = (LGAccountCreationInfo *)[self.objcImplementations objectForKey:currentInstance[@"uid"]];
    NSDictionary *result = @{@"value" : objcImpl.derivations};
    resolve(result);
}

RCT_REMAP_METHOD(getPublicKeys, getPublicKeys:(NSDictionary *)currentInstance withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)rejecter)
{
    LGAccountCreationInfo *objcImpl = (LGAccountCreationInfo *)[self.objcImplementations objectForKey:currentInstance[@"uid"]];
    NSDictionary *result = @{@"value" : objcImpl.publicKeys};
    resolve(result);
}

RCT_REMAP_METHOD(getChainCodes, getChainCodes:(NSDictionary *)currentInstance withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)rejecter)
{
    LGAccountCreationInfo *objcImpl = (LGAccountCreationInfo *)[self.objcImplementations objectForKey:currentInstance[@"uid"]];
    NSDictionary *result = @{@"value" : objcImpl.chainCodes};
    resolve(result);
}

@end
