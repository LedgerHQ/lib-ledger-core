#import <Foundation/Foundation.h>

@interface LGGetEthreumLikeWalletCallbackImpl : NSObject
-(void)onSuccess:(nullable LGEthereumLikeWallet *)wallet
        isCreated:(BOOL)isCreated;
-(void)onError:(nonnull LGError *)error;
@end