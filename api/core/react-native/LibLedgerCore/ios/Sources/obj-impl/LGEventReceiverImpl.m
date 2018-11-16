#import "LGEventReceiverImpl.h"
#import "../objc/LGEvent.h"
#import "../objc/LGEventCode.h"
#import "../objc/LGDynamicObject.h"
@implementation LGEventReceiverImpl
@synthesize resolve = _resolve;
@synthesize reject = _reject;
-(instancetype) initWithResolver:(RCTPromiseResolveBlock)resolve rejecter:(RCTPromiseRejectBlock)reject {
    self = [super init];
    if (self) {
        self.resolve = resolve;
        self.reject = reject;
    }
    return self;
}

/**
*Method triggered when an event occurs
*@param event, Event object that triggers this method
*/
- (void)onEvent:(nullable LGEvent *)event
{
    LGEventCode eventCode = [event getCode];
    LGDynamicObject *payload = [event getPayload];
    if (eventCode == LGEventCodeUndefined || eventCode == LGEventCodeSynchronizationFailed) {
        
        NSString *error = [[NSString alloc] init];
        if (payload) {
            error = [payload getString:@"EV_SYNC_ERROR_MESSAGE"];
        } else {
            error = @"Syncronization Failed";
            
        }
        //self.reject(error, @"Error while calling LGEventReceiverImpl::getIndex", nil);
        return;
        //TODO: Call to rejecter
    } else if (eventCode == LGEventCodeSynchronizationSucceed || eventCode == LGEventCodeSynchronizationSucceedOnPreviouslyEmptyAccount) {
        //TODO: Call to resolver
        self.resolve([payload getString:@"result"]);
        return;
    }
    return;
}
@end
