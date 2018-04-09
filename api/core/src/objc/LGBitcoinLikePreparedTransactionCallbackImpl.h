#import <Foundation/Foundation.h>

/**
 *Callback triggered by main completed task,
 *returns optional result of template type T
 */
@interface LGBitcoinLikePreparedTransactionCallbackImpl : NSObject
-(void)onCallback:(nullable LGBitcoinLikePreparedTransaction *)result
        error:(nullable LGError *)error;
@end

