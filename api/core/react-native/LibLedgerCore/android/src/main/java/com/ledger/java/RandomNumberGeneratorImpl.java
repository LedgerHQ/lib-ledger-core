package co.ledger.core;

/** Class to generate random numbers */
public class RandomNumberGeneratorImpl extends co.ledger.core.RandomNumberGenerator {
    /**
     * Generates random bytes.
     * @params size number of bytes to generate
     * @return 'size' random bytes
     */
    public byte[] getRandomBytes(int size) {
        byte [] result = new byte[0];
    	return result;
    }

    /**
     * Generates random 32 bits integer.
     * @return random 32 bits integer
     */
    public int getRandomInt() {
    	return 0;
    }

    /**
     * Generates random 64 bits integer.
     * @return random 64 bits integer
     */
    public long getRandomLong() {
    	return 0;
    }

    /**
     * Generates random byte.
     * @return random byte
     */
    public byte getRandomByte() {
    	return 0;
    }
}
