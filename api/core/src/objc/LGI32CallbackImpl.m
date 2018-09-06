#import "LGI32CallbackImpl.h"

@implementation LGI32CallbackImpl

/**
* Method triggered when main task complete
* @params result optional of type T, non null if main task failed
* @params error optional of type Error, non null if main task succeeded
*/
- (void)onCallback:(nullable NSNumber *)result
error:(nullable LGError *)error
{}

@end