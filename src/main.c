#include "utils.h"
#include "elgamal/elgamal.h"
#include <time.h>

int main(void) {
	time_t start, end;

	start = time(NULL);
	SizeT keySize = 1 * 1024;
	EGKeyPair pair = generateEGKeyPair(keySize);
	end = time(NULL);

	printf("%zu bits Key pair generated in %ld seconds\n", keySize, end - start);

	printEGKeyPair(&pair, HEX);

	FILE* fp_pub_out = fopen("eg_pub.key", "w");
	FILE* fp_priv_out = fopen("eg_priv.key", "w");

	exportEGPrivateKey(&pair.priv, fp_priv_out, true);
	exportEGPublicKey(&pair.pub, fp_pub_out, true);

	FILE* fp_pub_in = fopen("eg_pub.key", "r");
	FILE* fp_priv_in = fopen("eg_priv.key", "r");

	EGKeyPair paire = { .priv = importEGPrivateKey(fp_priv_in, true), .pub = importEGPublicKey(fp_pub_in, true) };

	printEGKeyPair(&paire, HEX);

	/*
	CustomInteger message = generateRandomInt(32);
	EGCiphered ciphered = cipherData(message, pair.pub);
	CustomInteger deciphered = decipherData(ciphered, pair);

	printf("Message (clear) = ");
	printInteger(message, HEX, false);
	printf("temp key = ");
	printInteger(ciphered.tempKey, HEX, false);
	printf("Message (ciphered) = ");
	printInteger(ciphered.c, HEX, false);
	printf("Message (deciphered) = ");
	printInteger(deciphered, HEX, false);

	freeInteger(&message);
	freeInteger(&ciphered.c);
	freeInteger(&ciphered.tempKey);
	freeInteger(&deciphered);
	*/

	freeEGKeyPair(&pair);
	freeEGKeyPair(&paire);

	return 0;
}
