#include "utils.h"
#include "elgamal/elgamal.h"
#include <time.h>

int main(void) {
	time_t start, end;

	int final = 0;
	int amount = 100;

	for (int n = 1; n <= amount; n++) {
		start = time(NULL);
		EGKeyPair pair = generateEGKeyPair(3 * 1024);
		end = time(NULL);

		printf("Key pair %d generated in %ld seconds\n", n, end - start);
		final += end - start;

		freeEGKeyPair(&pair);
	}

	printf("Total: %d secs\nAvg: %d secs/pair\n", final, final / amount);

	/*printEGKeyPair(&pair, HEX);

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

	freeEGKeyPair(&pair);*/

	return 0;
}
