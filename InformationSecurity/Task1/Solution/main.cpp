#include <string>

#include "caesar_cipher.h"


int main(int argc, char *argv[])
{
    caesar_cipher::caesar_cipher cc;

    std::string key = cc.identify_key("description.enc", "description.txt");
    cc.decryption(  key,
                    "task.enc",
                    "solutionTask1.txt" );
    return 0;
}