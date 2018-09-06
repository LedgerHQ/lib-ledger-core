#import <Foundation/Foundation.h>
#import "../../objc/LGPathResolver.h"

@interface LGPathResolverImpl : NSObject<LGPathResolver>
@property(nonatomic, strong) NSString *rootPath;
@end
