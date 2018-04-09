#import <Foundation/Foundation.h>

/** Helper class for commonly used crypto algorithms */
@interface LGHashAlgorithmHelperImpl : NSObject
/**
 *RACE Integrity Primitives Evaluation Message Digest (used in Bitcoin)
 *@param data in bytes, message to hash
 *@return 160 bits hashed message
 */
-(nonnull NSData *)ripemd160:(nonnull NSData *)data;
/**
 *Secure Hash Algorithm (used in Bitcoin)
 *@param data in bytes, message to hash
 *@return 256 bits hashed message
 */
-(nonnull NSData *)sha256:(nonnull NSData *)data;
/**
 *Hash algorithm used in ethereum
 *@param data in bytes, message to hash
 *@return 256 bits hashed message
 */
-(nonnull NSData *)keccak256:(nonnull NSData *)data;
@end