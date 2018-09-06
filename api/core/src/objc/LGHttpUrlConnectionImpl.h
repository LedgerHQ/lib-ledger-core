#import <Foundation/Foundation.h>
#import "../../objc/LGHttpUrlConnection.h"

@interface LGHttpUrlConnectionImpl : NSObject <LGHttpUrlConnection>
@property(nonatomic, strong) NSData *data;
@property(nonatomic, strong) NSURLResponse *response;
@property(nonatomic, strong) NSError *error;
-(instancetype) initWithData:(NSData *)data url:(NSURLResponse *)response andError:(NSError *)error;
@end
