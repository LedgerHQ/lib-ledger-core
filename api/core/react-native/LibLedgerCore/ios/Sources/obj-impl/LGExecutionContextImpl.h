#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>
#import "../objc/LGExecutionContext.h"

@interface LGExecutionContextImpl : NSObject<LGExecutionContext>
@property (nonatomic) RCTPromiseResolveBlock resolve;
@property (nonatomic) RCTPromiseRejectBlock reject;
@property(nonatomic) dispatch_queue_t queue;
- (instancetype)initContext:(BOOL)isSerialized;
@end
