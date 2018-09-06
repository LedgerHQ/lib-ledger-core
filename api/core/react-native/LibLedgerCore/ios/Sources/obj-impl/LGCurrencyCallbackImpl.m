#import "LGCurrencyCallbackImpl.h"

@implementation LGCurrencyCallbackImpl

/**
* Method triggered when main task complete
* @params result optional of type T, non null if main task failed
* @params error optional of type Error, non null if main task succeeded
*/
- (void)onCallback:(nullable LGCurrency *)result
error:(nullable LGError *)error
{}

@end