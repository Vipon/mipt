#include <iostream>
#include <string>

char EncryptByte(const char data, const char key);

#ifndef _ENCRYPT_BYTE_IMPL_
char EncryptByte(const char data, const char key)
{
	return data + key;
}
#endif // _ENCRYPT_BYTE_IMPL_

int EncryptFile(FILE* originalFile, FILE* encryptedFile, const std::string& key)
{
	char dataByte;
	char keyByte;
	int keyPos = 0;

	while (true)
	{
		size_t cnt = fread(&dataByte, 1, 1, originalFile);
		if (!cnt)
			break;
		keyByte = key[keyPos];
		dataByte = EncryptByte(dataByte, keyByte);
		cnt = fwrite(&dataByte, 1, 1, encryptedFile);
		keyPos = (++keyPos) % key.length();
	}

	return 0;
}

int EncryptFile(const std::string& originalFile, const std::string& encryptedFile, const std::string& key)
{
	FILE* orig = fopen(originalFile.c_str(), "rb");
	FILE* enc = fopen(encryptedFile.c_str(), "wb");
	int res = 0;

	if (orig && enc)
		res = EncryptFile(orig, enc, key);

	fclose(orig);
	fclose(enc);

	return res;
}

