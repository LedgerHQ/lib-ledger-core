package co.ledger.core;

/** Helper class for commonly used crypto algorithms */
public class HashAlgorithmHelperImpl extends HashAlgorithmHelper {
    /**
     *RACE Integrity Primitives Evaluation Message Digest (used in Bitcoin)
     *@param data in bytes, message to hash
     *@return 160 bits hashed message
     */
    public byte[] ripemd160(byte[] data) {
    	return null;
    }

    /**
     *Secure Hash Algorithm (used in Bitcoin)
     *@param data in bytes, message to hash
     *@return 256 bits hashed message
     */
    public byte[] sha256(byte[] data) {
    	return null;
    }

    /**
     *Hash algorithm used in ethereum
     *@param data in bytes, message to hash
     *@return 256 bits hashed message
     */
    public byte[] keccak256(byte[] data) {
    	return null;
    }
}
