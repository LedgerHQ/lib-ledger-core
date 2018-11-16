#import "LGRandomNumberGeneratorImpl.h"

@implementation LGRandomNumberGeneratorImpl

/**
* Generates random bytes.
* @params size number of bytes to generate
* @return 'size' random bytes
*/
- (nonnull NSData *)getRandomBytes:(int32_t)size
{
    NSMutableData* data = [NSMutableData dataWithLength:size];
    int result = SecRandomCopyBytes(kSecRandomDefault, size, data.mutableBytes);
    if (result != 0) {
        NSLog(@"SecRandomCopyBytes failed for some reason");
    }
    return data;
}

/**
* Generates random 32 bits integer.
* @return random 32 bits integer
*/
- (int32_t)getRandomInt
{
    NSData *data = [self getRandomBytes:sizeof(int32_t)];
    return *(int32_t *)([data bytes]);
}

/**
* Generates random 64 bits integer.
* @return random 64 bits integer
*/
- (int64_t)getRandomLong
{
    NSData *data = [self getRandomBytes:sizeof(int64_t)];
    return *(int64_t *)([data bytes]);
}

/**
* Generates random byte.
* @return random byte
*/
- (int8_t)getRandomByte
{
    NSData *data = [self getRandomBytes:sizeof(int8_t)];
    return *(int8_t *)([data bytes]);
}

@end
