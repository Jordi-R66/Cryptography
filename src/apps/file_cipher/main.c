#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../symmetric/chacha20/chacha20.h"
#include "../../symmetric/chacha20poly1305/chacha20poly1305.h"

#define BUFFER_SIZE 256

uint32 nonce[3] = { 0 };
uint8* nonce8 = (uint8*)nonce;

void readNonce(char* string) {
	char temp[3] = { 0 };

	for (int i = 0; i < 24; i += 2) {
		copyMemory(&string[i], &temp[0], 2);

		nonce8[i/2] = (uint8)(strtoul(temp, NULL, 16) & 0xFF);
	}
}

/*
SYNTAXE DES COMMANDES :
./a.out -g output.key -> GÉNÉRER UNE CLÉ CHACHA20
./a.out -n output.nonce -> GÉNÉRER UN NONCE
./a.out -c key nonce_as_hex file_in file_out -> CHIFFRER UN FICHIER AVEC LA CLÉ EN SPÉCIFIANT UN NONCE (12 OCTETS)
*/
int main(int argc, char** argv) {
	int output = 0;

	if (argc > 2) {
		char* action = argv[1];
		char* key_path = argv[2];

		bool generateKeyMode = strcmp(action, "-g") == 0;
		bool cipherMode = strcmp(action, "-c") == 0;
		bool nonceMode = strcmp(action, "-n") == 0;

		if (generateKeyMode && (argc == 3)) {
			CC20Key key = { 0 };

			FILE* fp = fopen(key_path, "w");
			CC20KeyGen(&key);
			exportCC20Key(&key, fp, true);

			printf("ChaCha20 key generated and exported!\n");
		} else if (nonceMode && argc == 3) {
			CC20NonceGen(nonce, true);
		} else if (cipherMode && (argc == 6)) {
			char* nonce_str = argv[3];

			char* file_in = argv[4];
			char* file_out = argv[5];

			FILE* fp_key = fopen(key_path, "r");

			CC20Key key = importCC20Key(fp_key, true);
			readNonce(nonce_str);

			copyMemory(nonce8, key.nonce, 12);
			setMemory(nonce8, 0, 12);

			Byte buffer[BUFFER_SIZE] = {0};
			bool keepReading = true;

			CC20Ctx ctx;

			FILE* fp_in = fopen(file_in, "r");
			FILE* fp_out = fopen(file_out, "w");

			initState(&ctx, (const Byte*)key.key, (const Byte*)key.nonce);
			while (keepReading) {
				SizeT bytesRead = fread(buffer, 1, BUFFER_SIZE, fp_in);
				keepReading = bytesRead >= BUFFER_SIZE;

				de_cipher(&ctx, buffer, bytesRead);
				fwrite(buffer, 1, bytesRead, fp_out);
				setMemory(buffer, 0, bytesRead);
			}

			fclose(fp_in);
			fclose(fp_out);
		} else {
			printf("Please check syntax\n");
		}
	}

	return output;
}