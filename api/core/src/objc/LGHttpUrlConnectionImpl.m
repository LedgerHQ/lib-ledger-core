#import "LGHttpUrlConnectionImpl.h"

@implementation LGHttpUrlConnectionImpl

/**
* Gets the HTTP response status code
* @return The HTTP response status code
*/
- (int32_t)getStatusCode
{
  return 0;
}

/**
* Gets the HTTP response status text
* @return The HTTP response status text
*/
- (nonnull NSString *)getStatusText
{
  return nil;
}

/**
* Gets the HTTP response headers
* @return The HTTP response headers
*/
- (nonnull NSDictionary<NSString *, NSString *> *)getHeaders
{
  return nil;
}

/**
* Reads available HTTP response body. This method will be called multiple times until it returns a empty bytes array.
* @returns A chunk of the body data wrapped into a HttpReadBodyResult (for error management)
*/
- (nonnull LGHttpReadBodyResult *)readBody
{
  return nil;
}
@end