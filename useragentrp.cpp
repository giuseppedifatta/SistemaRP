#include "useragentrp.h"

UserAgentRP::UserAgentRP(QObject *parent) : QObject(parent)
{
    ipUrna = "192.168.19.130";
}

string UserAgentRP::deriveKeyFromPass(string password)
{
    string derivedKey;
    try {

        // KDF parameters
        unsigned int iterations = 15000;
        char purpose = 0; // unused by Crypto++

        // 32 bytes of derived material. Used to key the cipher.
        //   16 bytes are for the key, and 16 bytes are for the iv.
        SecByteBlock derived(AES::MAX_KEYLENGTH);

        // KDF function
        PKCS5_PBKDF2_HMAC<SHA256> kdf;
        kdf.DeriveKey(derived.data(), derived.size(), purpose, (byte*)password.data(), password.size(), NULL, 0, iterations);




        // Encode derived
        HexEncoder hex(new StringSink(derivedKey));
        hex.Put(derived.data(), derived.size());
        hex.MessageEnd();

        // Print stuff
        cout << "pass: " << password << endl;
        //cout << "derived key: " << derivedKey << endl;

    }
    catch(CryptoPP::Exception& ex)
    {
        cerr << ex.what() << endl;
    }
    return derivedKey;
}

string UserAgentRP::getPassword() const
{
    return password;
}

void UserAgentRP::setPassword(const string &value)
{
    password = value;
}

