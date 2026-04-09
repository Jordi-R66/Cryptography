#include "aes_common.h"

void generateRandomBytes(Byte buffer[], SizeT nBytes) {
	FILE* fp = fopen("/dev/urandom", "r");

	fread(buffer, sizeof(Byte), nBytes, fp);

	fclose(fp);
}