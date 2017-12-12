#ifndef __CAESAR_CIPHER_H__
#define __CAESAR_CIPHER_H__

#include <string>

class caesar_cipher {
private:
    std::string find_patern_in_string(const std::string &key);

public:
    void encryption(const std::string &key,
                    const std::string &in,
                    const std::string &out = "");

    void decryption(const std::string &key,
                    const std::string &in,
                    const std::string &out = "");

    std::string identify_key(const std::string &enc, const std::string &dec);
};

#endif /* __CAESAR_CIPHER_H__ */