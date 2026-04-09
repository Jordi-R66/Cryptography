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

#pragma enregion