#import <Foundation/Foundation.h>

#import "../../objc/LGEventReceiver.h"
#import <React/RCTBridge.h>

@interface LGEventReceiverImpl : NSObject<LGEventReceiver>
@property(nonatomic) RCTPromiseResolveBlock resolve;
@property(nonatomic) RCTPromiseRejectBlock reject;
@end
