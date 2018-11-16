package com.ledger.java;
import java.util.Random;
/** Class to generate random numbers */
public class RandomNumberGeneratorImpl extends co.ledger.core.RandomNumberGenerator {
    private Random rand;
    public RandomNumberGeneratorImpl() {
        this.rand = new Random();
    }
    /**
     * Generates random bytes.
     * @params size number of bytes to generate
     * @return 'size' random bytes
     */
    public byte[] getRandomBytes(int size) {
        byte [] result = new byte[size];
        this.rand.nextBytes(result);
    	return result;
    }

    /**
     * Generates random 32 bits integer.
     * @return random 32 bits integer
     */
    public int getRandomInt() {
    	return this.rand.nextInt();
    }

    /**
     * Generates random 64 bits integer.
     * @return random 64 bits integer
     */
    public long getRandomLong() {
    	return this.rand.nextLong();
    }

    /**
     * Generates random byte.
     * @return random byte
     */
    public byte getRandomByte() {
    	return this.getRandomBytes(1)[0];
    }
}
