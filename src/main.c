#include "common/utils.h"
#include "chacha20/chacha20.h"
#include <time.h>

int main(void) {
	Byte key[32] = { 0 };
	Byte nonce[12] = { 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00 };
	Byte output[64] = { 0 };

	for (Byte i = 0; i < 32; i++) { key[i] = i; }

	// 2. Initialisation
	Chacha20_State myState = initState(key, nonce);

	// 4. Génération
	keystreamFactory(&myState, output);

	for (Byte i = 0; i < 64; i++) {printf("%02X ", output[i]);}
	printf("\n");

	return 0;
}
