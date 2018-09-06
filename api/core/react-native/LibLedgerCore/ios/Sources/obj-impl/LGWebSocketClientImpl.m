#import "LGWebSocketClientImpl.h"
#import "LGWebSocketConnection.h"
@implementation LGWebSocketClientImpl


- (void)connect:(nonnull NSString *)url connection:(nullable LGWebSocketConnection *)connection
{
    //TODO: review this implementation
    int32_t connectionId = [connection getConnectionId];
    [connection onConnect:connectionId];
}

- (void)send:(nullable LGWebSocketConnection *)connection data:(nonnull NSString *)data
{
    //TODO: review this implementation
    [connection onMessage:data];
}

- (void)disconnect:(nullable LGWebSocketConnection *)connection
{
    //TODO: review this implementation
    [connection onClose];
}

@end
