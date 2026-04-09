#include "chacha20.h"

#pragma region Tools

inline uint32 buildInt32LE(const Byte bytes[4]) {
	uint32 output = 0;

	output |= (uint32)bytes[0];
	output |= ((uint32)bytes[1]) << 8;
	output |= ((uint32)bytes[2]) << 16;
	output |= ((uint32)bytes[3]) << 24;

	return output;
}

inline void breakInt32LE(uint32 a, Byte bytes[4]) {
	for (uint8 i = 0; i < 4; i++) {
		bytes[i] = 0xFF & (Byte)(a >> (i * 8));
	}
}

inline uint32 ROTL32(uint32 a, uint32 n) {
	uint32 output;

	output = a << n;
	output |= a >> (32 - n);

	return output;
}

#pragma endregion

#pragma region Miscs

inline void QuarterRound(uint32* a, uint32* b, uint32* c, uint32* d) {
	*a += *b;
	*d ^= *a;
	*d = ROTL32(*d, 16);

	*c += *d;
	*b ^= *c;
	*b = ROTL32(*b, 12);

	*a += *b;
	*d ^= *a;
	*d = ROTL32(*d, 8);

	*c += *d;
	*b ^= *c;
	*b = ROTL32(*b, 7);
}

#pragma endregion

#pragma region Heart

Chacha20_State initState(const Byte key[32], const Byte nonce[12]) {
	Chacha20_State output = { .MagicWord = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574} };

	// Key Recomposition
	for (SizeT i = 0; i < 32; i += 4) {
		output.raw[4 + (i >> 2)] = buildInt32LE(&key[i]);
	}

	// Nonce Recomposition
	for (SizeT i = 0; i < 12; i += 4) {
		output.raw[13 + (i >> 2)] = buildInt32LE(&nonce[i]);
	}

	return output;
}

Chacha20_State keystreamFactory(Chacha20_State previousState);

#pragma endregion