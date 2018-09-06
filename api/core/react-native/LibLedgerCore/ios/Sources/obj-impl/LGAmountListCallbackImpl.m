#import "LGAmountListCallbackImpl.h"

@implementation LGAmountListCallbackImpl

/**
 * Method triggered when main task complete
 * @params result optional of type list<T>, non null if main task failed
 * @params error optional of type Error, non null if main task succeeded
 */
- (void)onCallback:(nullable NSArray<LGAmount *> *)result
             error:(nullable LGError *)error
{}

@end


