#import "LGPathResolverImpl.h"

@implementation LGPathResolverImpl

-(instancetype)init
{
    self = [super init];
    if (self) {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        if ([paths count] == 0) {
            self.rootPath = paths[0];
        }
    }
    return self;
}

/**
* Resolves the path for a SQLite database file.
* @param path The path to resolve.
* @return The resolved path.
*/
- (nonnull NSString *)resolveDatabasePath:(nonnull NSString *)path
{
    NSString *tmpPath = @"database_";
    tmpPath = [tmpPath stringByAppendingString:[path stringByReplacingOccurrencesOfString:@"/" withString:@"__"]];
    NSString *result = [self.rootPath stringByAppendingPathComponent:tmpPath];
    NSLog(@"************resolveDatabasePath: %@",result);
    return result;
}

/**
* Resolves the path of a single log file.
* @param path The path to resolve.
* @return The resolved path.
*/
- (nonnull NSString *)resolveLogFilePath:(nonnull NSString *)path
{
    NSString *tmpPath = @"log_file_";
    tmpPath = [tmpPath stringByAppendingString:[path stringByReplacingOccurrencesOfString:@"/" withString:@"__"]];
    
    NSString *result = [self.rootPath stringByAppendingPathComponent:tmpPath];
    NSLog(@"************resolveLogFilePath: %@",result);
    return result;
}

/**
* Resolves the path for a json file.
* @param path The path to resolve.
* @return The resolved path.
*/
- (nonnull NSString *)resolvePreferencesPath:(nonnull NSString *)path
{
    NSString *tmpPath = @"preferences_";
    tmpPath = [tmpPath stringByAppendingString:[path stringByReplacingOccurrencesOfString:@"/" withString:@"__"]];
    NSString *result = [self.rootPath stringByAppendingPathComponent:tmpPath];
    NSLog(@"************resolvePreferencesPath: %@",result);
    return result;
}

@end
