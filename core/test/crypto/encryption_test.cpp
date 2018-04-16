/*
 *
 * encryption_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/12/2016.
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

#include <gtest/gtest.h>
#include <ledger/core/crypto/AES256.hpp>
#include <ledger/core/crypto/PBKDF2.hpp>
#include <vector>
#include <ledger/core/crypto/AESCipher.hpp>
#include <OpenSSLRandomNumberGenerator.hpp>
#include <sstream>
#include <ledger/core/bytes/BytesReader.h>
#include <ledger/core/bytes/BytesWriter.h>

using namespace ledger::core;

static std::string BIG_TEXT = "Sed si ille hac tam eximia fortuna propter utilitatem rei publicae frui non properat, ut omnia illa conficiat, quid ego, senator, facere debeo, quem, etiamsi ille aliud vellet, rei publicae consulere oporteret?\n"
        "\n"
        "Constituendi autem sunt qui sint in amicitia fines et quasi termini diligendi. De quibus tres video sententias ferri, quarum nullam probo, unam, ut eodem modo erga amicum adfecti simus, quo erga nosmet ipsos, alteram, ut nostra in amicos benevolentia illorum erga nos benevolentiae pariter aequaliterque respondeat, tertiam, ut, quanti quisque se ipse facit, tanti fiat ab amicis.\n"
        "\n"
        "Nec vox accusatoris ulla licet subditicii in his malorum quaerebatur acervis ut saltem specie tenus crimina praescriptis legum committerentur, quod aliquotiens fecere principes saevi: sed quicquid Caesaris implacabilitati sedisset, id velut fas iusque perpensum confestim urgebatur impleri.\n"
        "\n"
        "Superatis Tauri montis verticibus qui ad solis ortum sublimius attolluntur, Cilicia spatiis porrigitur late distentis dives bonis omnibus terra, eiusque lateri dextro adnexa Isauria, pari sorte uberi palmite viget et frugibus minutis, quam mediam navigabile flumen Calycadnus interscindit.\n"
        "\n"
        "Has autem provincias, quas Orontes ambiens amnis imosque pedes Cassii montis illius celsi praetermeans funditur in Parthenium mare, Gnaeus Pompeius superato Tigrane regnis Armeniorum abstractas dicioni Romanae coniunxit.\n"
        "\n"
        "Nemo quaeso miretur, si post exsudatos labores itinerum longos congestosque adfatim commeatus fiducia vestri ductante barbaricos pagos adventans velut mutato repente consilio ad placidiora deverti.\n"
        "\n"
        "Ego vero sic intellego, Patres conscripti, nos hoc tempore in provinciis decernendis perpetuae pacis habere oportere rationem. Nam quis hoc non sentit omnia alia esse nobis vacua ab omni periculo atque etiam suspicione belli?\n"
        "\n"
        "Vita est illis semper in fuga uxoresque mercenariae conductae ad tempus ex pacto atque, ut sit species matrimonii, dotis nomine futura coniunx hastam et tabernaculum offert marito, post statum diem si id elegerit discessura, et incredibile est quo ardore apud eos in venerem uterque solvitur sexus.\n"
        "\n"
        "Batnae municipium in Anthemusia conditum Macedonum manu priscorum ab Euphrate flumine brevi spatio disparatur, refertum mercatoribus opulentis, ubi annua sollemnitate prope Septembris initium mensis ad nundinas magna promiscuae fortunae convenit multitudo ad commercanda quae Indi mittunt et Seres aliaque plurima vehi terra marique consueta.\n"
        "\n"
        "Haec dum oriens diu perferret, caeli reserato tepore Constantius consulatu suo septies et Caesaris ter egressus Arelate Valentiam petit, in Gundomadum et Vadomarium fratres Alamannorum reges arma moturus, quorum crebris excursibus vastabantur confines limitibus terrae Gallorum.";

static inline std::vector<uint8_t> vectorize(const std::string& str) {
    return std::vector<uint8_t>(str.data(), str.data() + str.size());
}

static inline std::string strigify(const std::vector<uint8_t>& vector) {
    return std::string((const char *)vector.data());
}

TEST(Encryption, EncryptDecryptWithAES256CBC) {
    auto blocksize = AES256::BLOCK_SIZE;
    auto IV = std::vector<uint8_t>();
    for (auto i = 0; i < blocksize; i++) {
        IV.push_back((uint8_t)i);
    }
    std::string data = "Hello world!";
    std::vector<uint8_t> encryptKey;
    for (auto i = 0; i < 32; i++) {
        encryptKey.push_back(i << 1);
    }

    auto encrypted = AES256::encrypt(IV, encryptKey, vectorize(data));
    EXPECT_NE(encrypted, vectorize(data));
    auto decrypted = strigify(AES256::decrypt(IV, encryptKey, encrypted));
    EXPECT_EQ(data, decrypted);
}

TEST(Encryption, EncryptDecryptWithAES256CBCAndPBKDF2) {
    auto blocksize = AES256::BLOCK_SIZE;
    auto IV = std::vector<uint8_t>();
    for (auto i = 0; i < blocksize; i++) {
        IV.push_back((uint8_t)i);
    }
    std::string data = "Hello world!";
    std::string password = "My supa strong password!";
    std::string salt = "Random salt";
    std::vector<uint8_t> encryptKey = PBKDF2::derive(vectorize(password), vectorize(salt), 10000, 32);

    auto encrypted = AES256::encrypt(IV, encryptKey, vectorize(data));
    EXPECT_NE(encrypted, vectorize(data));
    auto decrypted = strigify(AES256::decrypt(IV, encryptKey, encrypted));
    EXPECT_EQ(data, decrypted);
}

TEST(Encryption, EncryptDecryptWithCipher) {

    auto rng = std::make_shared<OpenSSLRandomNumberGenerator>();

    //Init reader
    std::vector<uint8_t> vec(BIG_TEXT.begin(), BIG_TEXT.end());
    BytesReader input(vec);

    //Encrypt data
    BytesWriter encrypted;
    AESCipher cipher(rng, "A very strong password", "Awesome salt", 10000);
    cipher.encrypt(input, encrypted);
    EXPECT_NE(input.readUntilEnd(), encrypted.toByteArray());

    //Reset cursor to read during final comparaison
    input.reset();

    //Decrypt data
    BytesWriter destination;
    BytesReader encryptedReader(encrypted.toByteArray());
    cipher.decrypt(encryptedReader, destination);
    EXPECT_EQ(destination.toByteArray(), input.readUntilEnd());
}

TEST(Encryption, EncryptDecryptWithCipherHugeText) {
    auto rng = std::make_shared<OpenSSLRandomNumberGenerator>();
    //Init reader
    std::vector<uint8_t> Bigvec;
    for (auto i = 0; i < 10; i++) {
        std::vector<uint8_t> vec(BIG_TEXT.begin(), BIG_TEXT.end());
        Bigvec.insert(Bigvec.end(), vec.begin(), vec.end());
    }

    BytesReader input(Bigvec);

    //Encrypt data
    BytesWriter encrypted;
    AESCipher cipher(rng, "A very strong password", "Awesome salt", 10000);
    cipher.encrypt(input, encrypted);
    EXPECT_NE(input.readUntilEnd(), encrypted.toByteArray());

    //Reset cursor to read during final comparaison
    input.reset();

    //Decrypt data
    BytesWriter destination;
    BytesReader encryptedReader(encrypted.toByteArray());
    cipher.decrypt(encryptedReader, destination);
    auto inputBytes = input.readUntilEnd();
    EXPECT_EQ(destination.toByteArray(), inputBytes);
}