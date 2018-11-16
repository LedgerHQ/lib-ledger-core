#import <Foundation/Foundation.h>
#import "../objc/LGThreadDispatcher.h"

@interface LGThreadDispatcherImpl : NSObject<LGThreadDispatcher>
@property(nonatomic, strong) NSMutableDictionary *contexts;
@end
