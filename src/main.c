#include "utils.h"
#include "elgamal/elgamal.h"
#include <time.h>

int main(void) {
	time_t start, end;
	start = time(NULL);
	EGKeyPair pair = generateEGKeyPair(48);
	end = time(NULL);

	printf("Key pair generated in %ld seconds\n", end - start);

	printEGKeyPair(&pair, HEX);

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

	freeEGKeyPair(&pair);

	return 0;
}
