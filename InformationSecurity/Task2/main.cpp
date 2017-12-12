#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char *argv[])
{
    std::ifstream f_enc("image_430x401.enc", std::ifstream::in);
    std::ifstream f_dec("test_430x401.raw", std::ifstream::in);
    std::ifstream secret("secret_430x401.enc", std::ifstream::in);
    std::ofstream solution("solutionTask2_430x401.png", std::ofstream::out);

    int pos = 0;
    char keyByte = 0;
    char encByte = 0;
    char decByte = 0;
    char secByte = 0;

    while (secret.get(secByte) && f_enc.get(encByte) && f_dec.get(decByte) && pos < 517290) {
        keyByte = (secByte ^ encByte) ^ decByte;
        solution << keyByte;
        ++pos;
    }

    f_enc.close();
    f_dec.close();
    secret.close();
    solution.close();

    return 0;
}