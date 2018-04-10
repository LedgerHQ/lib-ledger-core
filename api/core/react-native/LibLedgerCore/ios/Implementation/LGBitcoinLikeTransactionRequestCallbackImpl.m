#import "LGBitcoinLikeTransactionRequestCallbackImpl.h"

@implementation LGBitcoinLikeTransactionRequestCallbackImpl

/**
* Method triggered when main task complete
* @params result optional of type T, non null if main task failed
* @params error optional of type Error, non null if main task succeeded
*/
- (void)onCallback:(nullable LGBitcoinLikeTransactionRequest *)result
error:(nullable LGError *)error
{}

@end