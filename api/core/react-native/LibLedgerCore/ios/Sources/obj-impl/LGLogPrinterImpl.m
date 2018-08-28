#import "LGLogPrinterImpl.h"

@implementation LGLogPrinterImpl

-(instancetype)init
{
    self = [super init];
    if (self) {
        BOOL isSerialized = YES;
        self.context = [[LGExecutionContextImpl alloc] initContext:isSerialized];
    }
    return self;
}
-(void)printError:(nonnull NSString *)message
{
    NSLog(@"Error: %@", message);
}

-(void)printInfo:(nonnull NSString *)message
{
    NSLog(@"Info: %@", message);
}

-(void)printDebug:(nonnull NSString *)message
{
    NSLog(@"Debug: %@", message);
}

-(void)printWarning:(nonnull NSString *)message
{
    NSLog(@"Warning: %@", message);
}

-(void)printApdu:(nonnull NSString *)message
{
    NSLog(@"APDU: %@", message);
}

-(void)printCriticalError:(nonnull NSString *)message
{
    NSLog(@"Critical Error: %@", message);
}

-(nullable id<LGExecutionContext>)getContext
{
  return self.context;
}
@end
