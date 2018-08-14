// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#import "RCTCoreLGQueryFilter.h"


@implementation RCTCoreLGQueryFilter
//Export module
RCT_EXPORT_MODULE(RCTCoreLGQueryFilter)

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

RCT_REMAP_METHOD(accountEq,accountEqwithParams:(nonnull NSString *)accountUid withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter accountEq:accountUid];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::accountEq", nil);
    }

}

RCT_REMAP_METHOD(accountNeq,accountNeqwithParams:(nonnull NSString *)accountUid withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter accountNeq:accountUid];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::accountNeq", nil);
    }

}

RCT_REMAP_METHOD(dateLte,dateLtewithParams:(nonnull NSDate *)time withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter dateLte:time];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::dateLte", nil);
    }

}

RCT_REMAP_METHOD(dateLt,dateLtwithParams:(nonnull NSDate *)time withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter dateLt:time];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::dateLt", nil);
    }

}

RCT_REMAP_METHOD(dateGt,dateGtwithParams:(nonnull NSDate *)time withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter dateGt:time];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::dateGt", nil);
    }

}

RCT_REMAP_METHOD(dateGte,dateGtewithParams:(nonnull NSDate *)time withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter dateGte:time];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::dateGte", nil);
    }

}

RCT_REMAP_METHOD(dateEq,dateEqwithParams:(nonnull NSDate *)time withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter dateEq:time];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::dateEq", nil);
    }

}

RCT_REMAP_METHOD(dateNeq,dateNeqwithParams:(nonnull NSDate *)time withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter dateNeq:time];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::dateNeq", nil);
    }

}

RCT_REMAP_METHOD(containsRecipient,containsRecipientwithParams:(nonnull NSString *)recipientAddress withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter containsRecipient:recipientAddress];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::containsRecipient", nil);
    }

}

RCT_REMAP_METHOD(containsSender,containsSenderwithParams:(nonnull NSString *)senderAddress withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter containsSender:senderAddress];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::containsSender", nil);
    }

}

RCT_REMAP_METHOD(currencyEq,currencyEqwithParams:(nonnull NSString *)currencyName withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter currencyEq:currencyName];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::currencyEq", nil);
    }

}

RCT_REMAP_METHOD(operationUidEq,operationUidEqwithParams:(nonnull NSString *)operationUid withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter operationUidEq:operationUid];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::operationUidEq", nil);
    }

}

RCT_REMAP_METHOD(operationUidNeq,operationUidNeqwithParams:(nonnull NSString *)operationUid withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter operationUidNeq:operationUid];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::operationUidNeq", nil);
    }

}

RCT_REMAP_METHOD(trustEq,trustEqwithParams:(LGTrustLevel)trust withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter trustEq:trust];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::trustEq", nil);
    }

}

RCT_REMAP_METHOD(trustNeq,trustNeqwithParams:(LGTrustLevel)trust withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter trustNeq:trust];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::trustNeq", nil);
    }

}

RCT_REMAP_METHOD(feesEq,feesEqwithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter feesEq:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::feesEq", nil);
    }

}

RCT_REMAP_METHOD(feesNeq,feesNeqwithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter feesNeq:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::feesNeq", nil);
    }

}

RCT_REMAP_METHOD(feesGte,feesGtewithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter feesGte:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::feesGte", nil);
    }

}

RCT_REMAP_METHOD(feesGt,feesGtwithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter feesGt:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::feesGt", nil);
    }

}

RCT_REMAP_METHOD(feesLte,feesLtewithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter feesLte:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::feesLte", nil);
    }

}

RCT_REMAP_METHOD(feesLt,feesLtwithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter feesLt:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::feesLt", nil);
    }

}

RCT_REMAP_METHOD(amountEq,amountEqwithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter amountEq:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::amountEq", nil);
    }

}

RCT_REMAP_METHOD(amountNeq,amountNeqwithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter amountNeq:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::amountNeq", nil);
    }

}

RCT_REMAP_METHOD(amountGte,amountGtewithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter amountGte:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::amountGte", nil);
    }

}

RCT_REMAP_METHOD(amountGt,amountGtwithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter amountGt:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::amountGt", nil);
    }

}

RCT_REMAP_METHOD(amountLte,amountLtewithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter amountLte:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::amountLte", nil);
    }

}

RCT_REMAP_METHOD(amountLt,amountLtwithParams:(NSDictionary *)amount withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    RCTCoreLGAmount *rctParam_amount = (RCTCoreLGAmount *)[self.bridge moduleForName:@"CoreLGAmount"];
    LGAmount *objcParam_0 = (LGAmount *)[rctParam_amount.objcImplementations objectForKey:amount[@"uid"]];
    LGQueryFilter * objcResult = [LGQueryFilter amountLt:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::amountLt", nil);
    }

}

RCT_REMAP_METHOD(blockHeightEq,blockHeightEqwithParams:(int64_t)blockHeight withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter blockHeightEq:blockHeight];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::blockHeightEq", nil);
    }

}

RCT_REMAP_METHOD(blockHeightNeq,blockHeightNeqwithParams:(int64_t)blockHeight withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter blockHeightNeq:blockHeight];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::blockHeightNeq", nil);
    }

}

RCT_REMAP_METHOD(blockHeightGte,blockHeightGtewithParams:(int64_t)blockHeight withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter blockHeightGte:blockHeight];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::blockHeightGte", nil);
    }

}

RCT_REMAP_METHOD(blockHeightGt,blockHeightGtwithParams:(int64_t)blockHeight withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter blockHeightGt:blockHeight];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::blockHeightGt", nil);
    }

}

RCT_REMAP_METHOD(blockHeightLte,blockHeightLtewithParams:(int64_t)blockHeight withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter blockHeightLte:blockHeight];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::blockHeightLte", nil);
    }

}

RCT_REMAP_METHOD(blockHeightLt,blockHeightLtwithParams:(int64_t)blockHeight withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter blockHeightLt:blockHeight];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::blockHeightLt", nil);
    }

}

RCT_REMAP_METHOD(blockHeightIsNull,blockHeightIsNullWithResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter blockHeightIsNull];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::blockHeightIsNull", nil);
    }

}

RCT_REMAP_METHOD(operationTypeEq,operationTypeEqwithParams:(LGOperationType)operationType withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter operationTypeEq:operationType];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::operationTypeEq", nil);
    }

}

RCT_REMAP_METHOD(operationTypeNeq,operationTypeNeqwithParams:(LGOperationType)operationType withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    LGQueryFilter * objcResult = [LGQueryFilter operationTypeNeq:operationType];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::operationTypeNeq", nil);
    }

}

RCT_REMAP_METHOD(opAnd,opAnd:(NSDictionary *)currentInstance withParams:(NSDictionary *)filter withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGQueryFilter::opAnd, first argument should be an instance of LGQueryFilter", nil);
    }
    LGQueryFilter *currentInstanceObj = [self.objcImplementations objectForKey:currentInstance[@"uid"]];
    if (!currentInstanceObj)
    {
        NSString *error = [NSString stringWithFormat:@"Error while calling LGQueryFilter::opAnd, instance of uid %@ not found", currentInstance[@"uid"]];
        reject(@"impl_call_error", error, nil);
    }
    RCTCoreLGQueryFilter *rctParam_filter = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    LGQueryFilter *objcParam_0 = (LGQueryFilter *)[rctParam_filter.objcImplementations objectForKey:filter[@"uid"]];
    LGQueryFilter * objcResult = [currentInstanceObj opAnd:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::opAnd", nil);
    }

}

RCT_REMAP_METHOD(opOr,opOr:(NSDictionary *)currentInstance withParams:(NSDictionary *)filter withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGQueryFilter::opOr, first argument should be an instance of LGQueryFilter", nil);
    }
    LGQueryFilter *currentInstanceObj = [self.objcImplementations objectForKey:currentInstance[@"uid"]];
    if (!currentInstanceObj)
    {
        NSString *error = [NSString stringWithFormat:@"Error while calling LGQueryFilter::opOr, instance of uid %@ not found", currentInstance[@"uid"]];
        reject(@"impl_call_error", error, nil);
    }
    RCTCoreLGQueryFilter *rctParam_filter = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    LGQueryFilter *objcParam_0 = (LGQueryFilter *)[rctParam_filter.objcImplementations objectForKey:filter[@"uid"]];
    LGQueryFilter * objcResult = [currentInstanceObj opOr:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::opOr", nil);
    }

}

RCT_REMAP_METHOD(opAndNot,opAndNot:(NSDictionary *)currentInstance withParams:(NSDictionary *)filter withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGQueryFilter::opAndNot, first argument should be an instance of LGQueryFilter", nil);
    }
    LGQueryFilter *currentInstanceObj = [self.objcImplementations objectForKey:currentInstance[@"uid"]];
    if (!currentInstanceObj)
    {
        NSString *error = [NSString stringWithFormat:@"Error while calling LGQueryFilter::opAndNot, instance of uid %@ not found", currentInstance[@"uid"]];
        reject(@"impl_call_error", error, nil);
    }
    RCTCoreLGQueryFilter *rctParam_filter = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    LGQueryFilter *objcParam_0 = (LGQueryFilter *)[rctParam_filter.objcImplementations objectForKey:filter[@"uid"]];
    LGQueryFilter * objcResult = [currentInstanceObj opAndNot:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::opAndNot", nil);
    }

}

RCT_REMAP_METHOD(opOrNot,opOrNot:(NSDictionary *)currentInstance withParams:(NSDictionary *)filter withResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject) {
    if (!currentInstance[@"uid"] || !currentInstance[@"type"])
    {
        reject(@"impl_call_error", @"Error while calling RCTCoreLGQueryFilter::opOrNot, first argument should be an instance of LGQueryFilter", nil);
    }
    LGQueryFilter *currentInstanceObj = [self.objcImplementations objectForKey:currentInstance[@"uid"]];
    if (!currentInstanceObj)
    {
        NSString *error = [NSString stringWithFormat:@"Error while calling LGQueryFilter::opOrNot, instance of uid %@ not found", currentInstance[@"uid"]];
        reject(@"impl_call_error", error, nil);
    }
    RCTCoreLGQueryFilter *rctParam_filter = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    LGQueryFilter *objcParam_0 = (LGQueryFilter *)[rctParam_filter.objcImplementations objectForKey:filter[@"uid"]];
    LGQueryFilter * objcResult = [currentInstanceObj opOrNot:objcParam_0];

    NSString *uuid = [[NSUUID UUID] UUIDString];
    RCTCoreLGQueryFilter *rctImpl_objcResult = (RCTCoreLGQueryFilter *)[self.bridge moduleForName:@"CoreLGQueryFilter"];
    [rctImpl_objcResult.objcImplementations setObject:objcResult forKey:uuid];
    NSDictionary *result = @{@"type" : @"CoreLGQueryFilter", @"uid" : uuid };

    if(result)
    {
        resolve(result);
    }
    else
    {
        reject(@"impl_call_error", @"Error while calling LGQueryFilter::opOrNot", nil);
    }

}
@end
