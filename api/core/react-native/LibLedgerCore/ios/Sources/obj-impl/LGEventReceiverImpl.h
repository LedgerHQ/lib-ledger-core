#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>
#import "../objc/LGEventReceiver.h"

@interface LGEventReceiverImpl : NSObject<LGEventReceiver>
-(instancetype) initWithResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject;
@property (nonatomic) RCTPromiseResolveBlock resolve;
@property (nonatomic) RCTPromiseRejectBlock reject;
@end
