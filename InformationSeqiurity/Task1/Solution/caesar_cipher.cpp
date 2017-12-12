#include <string>
#include <fstream>
#include <iostream>

#include "caesar_cipher.h"

static const char ENC_PREF[] = "enc_";
static const char DEC_PREF[] = "dec_";

static char EncryptByte(const char data, const char key)
{
    return data + key;
}

void
caesar_cipher::encryption(  const std::string &key,
                            const std::string &in,
                            const std::string &out )
{
    std::ifstream f_in(in, std::ifstream::in);
    std::ofstream f_out;

    if (out == "") {
        std::string out_file = std::string(ENC_PREF) + in;
        f_out.open(out_file, std::ifstream::out);
    } else
        f_out.open(out, std::ofstream::out);

    int keyPos = 0;
    char keyByte = 0;
    char dataByte = 0;

    while (f_in.get(dataByte)) {
        keyByte = key[keyPos];
        dataByte = EncryptByte(dataByte, keyByte);
        f_out << dataByte;
        keyPos = (++keyPos) % key.length();
    }

    f_in.close();
    f_out.close();
}

static char DecryptByte(const char data, const char key)
{
    return data - key;
}

void
caesar_cipher::decryption(  const std::string &key,
                            const std::string &in,
                            const std::string &out )
{
    std::ifstream f_in(in, std::ifstream::in);
    std::ofstream f_out;

    if (out == "") {
        std::string out_file = std::string(DEC_PREF) + in;
        f_out.open(out_file, std::ifstream::out);
    } else
        f_out.open(out, std::ofstream::out);

    int keyPos = 0;
    char keyByte = 0;
    char dataByte = 0;

    while (f_in.get(dataByte)) {
        keyByte = key[keyPos];
        dataByte = DecryptByte(dataByte, keyByte);
        f_out << dataByte;
        keyPos = (++keyPos) % key.length();
    }

    f_in.close();
    f_out.close();
}

std::string
caesar_cipher::find_patern_in_string(const std::string &key)
{
    size_t len = key.length();
    std::string sub_key;

    for (size_t i = 1; i < len/2; ++i)
        if (len % i == 0) {
            sub_key = key.substr(0,i);

            for (size_t j = i; i < len; j += i) {
                if (sub_key != key.substr(j,i))
                    break;

                return sub_key;
            }
        }

    return key;
}

std::string
caesar_cipher::identify_key(const std::string &enc, const std::string &dec)
{
    std::ifstream f_enc(enc, std::ifstream::in);
    std::ifstream f_dec(dec, std::ifstream::in);

    int pos = 0;
    char keyByte = 0;
    char encByte = 0;
    char decByte = 0;
    std::string key;

    f_enc >> key;
    f_enc.close();

    while (f_dec.get(decByte))
        key[pos++] -= decByte;

    key = find_patern_in_string(key);

    f_dec.close();

    return key;
}
