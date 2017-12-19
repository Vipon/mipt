#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char *argv[])
{
    std::ifstream f_enc("image_430x401.enc", std::ifstream::in);
    std::ifstream secret("secret_430x401.enc", std::ifstream::in);
    std::ofstream solution("solutionTask2_430x401.png", std::ofstream::out);

    int pos = 0;
    char keyByte = 0;
    char encByte = 0;
    char secByte = 0;

    while (secret.get(secByte) && f_enc.get(encByte) && pos < 517290) {
        keyByte = encByte ^ secByte;
        solution << keyByte;
        ++pos;
    }

    f_enc.close();
    secret.close();
    solution.close();

    return 0;
}