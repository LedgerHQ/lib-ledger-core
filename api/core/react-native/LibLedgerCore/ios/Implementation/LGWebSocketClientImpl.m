#import "LGWebSocketClientImpl.h"

@implementation LGWebSocketClientImpl

- (void)connect:(nonnull NSString *)url
connection:(nullable LGWebSocketConnection *)connection
{}

- (void)send:(nullable LGWebSocketConnection *)connection
data:(nonnull NSString *)data
{}

- (void)disconnect:(nullable LGWebSocketConnection *)connection
{}

@end