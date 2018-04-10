#import "LGLogPrinterImpl.h"

@implementation LGLogPrinterImpl
-(void)printError:(nonnull NSString *)message
{}
-(void)printInfo:(nonnull NSString *)message
{}
-(void)printDebug:(nonnull NSString *)message
{}
-(void)printWarning:(nonnull NSString *)message
{}
-(void)printApdu:(nonnull NSString *)message
{}
-(void)printCriticalError:(nonnull NSString *)message
{}
-(nullable id<LGExecutionContext>)getContext
{
  return nil;
}
@end