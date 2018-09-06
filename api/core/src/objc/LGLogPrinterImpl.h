#import <Foundation/Foundation.h>
#import "../../objc/LGLogPrinter.h"
#import "LGExecutionContextImpl.h"

@interface LGLogPrinterImpl : NSObject<LGLogPrinter>
@property(nonatomic, strong) LGExecutionContextImpl *context;
@end
