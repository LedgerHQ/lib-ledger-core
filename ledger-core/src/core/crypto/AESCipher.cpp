/*
 *
 * AESCipher
 * ledger-core
 *
 * Created by Pierre Pollastri on 08/12/2016.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <core/crypto/AESCipher.hpp>
#include <core/crypto/PBKDF2.hpp>
#include <core/crypto/AES256.hpp>

namespace ledger {
    namespace core {
        AESCipher::AESCipher(const std::shared_ptr<api::RandomNumberGenerator> &rng, const std::string &password,
                             const std::string &salt, uint32_t iter) {
            _rng = rng;
            _key = PBKDF2::derive(
                    std::vector<uint8_t>(password.data(), password.data() + password.size()),
                    std::vector<uint8_t>(salt.data(), salt.data() + salt.size())
                    , iter, 32);
        }

        void AESCipher::encrypt(std::istream *input, std::ostream *output) {
#if defined(_WIN32) || defined(_WIN64)
#else
            input->seekg(0, input->beg);
            uint32_t maxRead = 254 * AES256::BLOCK_SIZE;
            uint8_t buffer[maxRead];
            do {
                // Read 254 * AES_BLOCK_SIZE bytes (we want at most 0xFF blocks to encrypt we the same IV)
                input->read((char *)buffer, maxRead);
                uint32_t read = input->gcount();

                // Create an IVt
                auto IV = _rng->getRandomBytes(AES256::BLOCK_SIZE);
                assert(IV.size() == AES256::BLOCK_SIZE);

                // Encrypt
                auto encrypted = AES256::encrypt(IV, _key, std::vector<uint8_t>(buffer, buffer + read));
                // Store number of blocks
                uint8_t blocksCount = (encrypted.size() / AES256::BLOCK_SIZE);
                (*output) << blocksCount;
                (*output) << read;
                // Store IV
                output->write((char *)IV.data(), IV.size());
                // Store encrypted data
                output->write((char *)encrypted.data(), encrypted.size());
            } while (!input->eof());
#endif
        }

        void AESCipher::decrypt(std::istream *input, std::ostream *output) {
#if defined(_WIN32) || defined(_WIN64)
#else
            input->seekg(0, input->beg);
            uint32_t maxRead = 255 * AES256::BLOCK_SIZE;
            uint8_t buffer[maxRead];
            do {
                uint8_t blocksCount;
                uint32_t dataSize;
                (*input) >> blocksCount;
                (*input) >> dataSize;

                std::vector<uint8_t> IV;
                IV.resize(AES256::BLOCK_SIZE);
                input->read((char *)IV.data(), IV.size());
                input->read((char *)buffer, blocksCount * AES256::BLOCK_SIZE);
                if (input->gcount() < blocksCount * AES256::BLOCK_SIZE)
                    break;
                auto decrypted = AES256::decrypt(IV, _key, std::vector<uint8_t>(buffer, buffer + (blocksCount * AES256::BLOCK_SIZE)));
                output->write((char *)decrypted.data(), dataSize);
            } while (!input->eof());
#endif
        }

        void AESCipher::encrypt(BytesReader& input, BytesWriter& output) {
            uint32_t maxRead = 254 * AES256::BLOCK_SIZE;
            do {
                // Read 254 * AES_BLOCK_SIZE bytes (we want at most 0xFF blocks to encrypt we the same IV)
                uint32_t available = input.available();
                uint32_t minEncryptedRead = std::min(maxRead,available);
                std::vector<uint8_t> dataToEncrypt = input.read(minEncryptedRead);
                uint32_t read = dataToEncrypt.size();
                // Create an IVt
                auto IV = _rng->getRandomBytes(AES256::BLOCK_SIZE);
                // Encrypt
                auto encrypted = AES256::encrypt(IV, _key, dataToEncrypt);
                // Store number of blocks
                uint8_t blocksCount = (encrypted.size() / AES256::BLOCK_SIZE);
                //Number of blocks of size AES256::BLOCK_SIZE in enrypted data
                output.writeByte(blocksCount);
                //Limit of reading (padding or string terminating before maxRead)
                output.writeVarInt(read);
                // Store IV
                output.writeByteArray(IV);
                // Store encrypted data
                output.writeByteArray(encrypted);
            } while (input.hasNext());
        }

        void AESCipher::decrypt(BytesReader& input, BytesWriter& output) {
            uint32_t maxRead = 255 * AES256::BLOCK_SIZE;
            do {
                //Get number of blocks in encrypted data
                uint8_t blocksCount = input.readNextByte();
                //Size of encrypted (chunk of) data
                uint32_t encryptedDataSize = blocksCount*AES256::BLOCK_SIZE;
                //Get data that we read
                uint32_t dataSize = input.readNextVarInt();
                //Read IV
                std::vector<uint8_t> IV = input.read(AES256::BLOCK_SIZE);
                //Get number of bytes to read
                uint32_t available = input.available();
                if(available < encryptedDataSize) {
                    //Should not read than more available data
                    break;
                }
                //Read encrypted data
                std::vector<uint8_t> encryptedData = input.read(encryptedDataSize);
                //Decrypt
                auto decrypted = AES256::decrypt(IV, _key, encryptedData);
                //Truncate if needed to size of encrypted data that we stored in dataSize bytes
                if(dataSize <  decrypted.size()){
                    decrypted.resize(dataSize);
                }
                //Store decrypted data
                output.writeByteArray(decrypted);
            } while (input.hasNext());
        }
    }
}
