#import <Foundation/Foundation.h>

#import "../../objc/LGExecutionContext.h"

@interface LGExecutionContextImpl : NSObject<LGExecutionContext>
//@property(nonatomic) BOOL isSerialized;
@property(nonatomic) dispatch_queue_t queue;
- (instancetype)initContext:(BOOL)isSerialized;
@end
