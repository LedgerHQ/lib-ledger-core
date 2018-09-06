package com.ledger.java;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/** Helper class for commonly used crypto algorithms */
public class HashAlgorithmHelperImpl extends co.ledger.core.HashAlgorithmHelper {

    /**
     *RACE Integrity Primitives Evaluation Message Digest (used in Bitcoin)
     *@param data in bytes, message to hash
     *@return 160 bits hashed message
     */
    public byte[] ripemd160(byte[] data) {
        //TODO: not implemented
    	return data;
    }

    /**
     *Secure Hash Algorithm (used in Bitcoin)
     *@param data in bytes, message to hash
     *@return 256 bits hashed message
     */
    public byte[] sha256(byte[] data)  {
        byte [] tmpData = data;
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            tmpData = digest.digest(tmpData);
        } catch (NoSuchAlgorithmException e) {
            System.out.println("No such algorithm: SHA-256");
        }

    	return tmpData;
    }

    /**
     *Hash algorithm used in ethereum
     *@param data in bytes, message to hash
     *@return 256 bits hashed message
     */
    public byte[] keccak256(byte[] data) {
        //TODO: not implemented
        return data;
    }
}
