#include "chacha20.h"
#include "constants.h"

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

void CC20KeyGen(CC20Key* key) {
	FILE* fp = fopen("/dev/urandom", "r");

	if (fp != NULL) {
		fread(key, CHACHA20_KEY_SIZE, 1, fp);
		fclose(fp);
	}
}

void exportCC20Key(CC20Key* key, FILE* fp, bool closeAfterWriting) {
	AlgoId algo = (AlgoId)CHACHA20;
	KeyT keyType = (KeyT)SECRET_KEY;

	if (fp != NULL) {
		fwrite(&algo, sizeof(AlgoId), 1, fp);
		fwrite(&keyType, sizeof(KeyT), 1, fp);

		fwrite(key, CHACHA20_KEY_SIZE, 1, fp);

		if (closeAfterWriting) {
			fclose(fp);
		}
	}
}

CC20Key importCC20Key(FILE* fp, bool closeAfterReading) {
	CC20Key output;

	AlgoId algo;
	KeyT keyType;

	const SizeT filesize = sizeof(AlgoId) + sizeof(KeyT) + CHACHA20_KEY_SIZE;

	if (fp != NULL) {
		// Mesure de la taille du fichier
		fseek(fp, 0, SEEK_END);
		SizeT size = (SizeT)ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (size == filesize) {
			fread(&algo, sizeof(AlgoId), 1, fp);
			fread(&keyType, sizeof(KeyT), 1, fp);

			if (algo == CHACHA20 && keyType == SECRET_KEY) {
				fread(&output, CHACHA20_KEY_SIZE, 1, fp);
			}
		}

		if (closeAfterReading) {
			fclose(fp);
		}
	}

	return output;
}

#pragma endregion

#pragma region Heart

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

void initState(const Chacha20_Context_Ptr ctx, const Byte key[32], const Byte nonce[12]) {
	ctx->state = (Chacha20_State){ .MagicWord = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574} };
	ctx->keystream_pos = 64;

	// Key Recomposition
	for (SizeT i = 0; i < 32; i += 4) {
		ctx->state.raw[4 + (i >> 2)] = buildInt32LE(&key[i]);
	}

	// Nonce Recomposition
	for (SizeT i = 0; i < 12; i += 4) {
		ctx->state.raw[13 + (i >> 2)] = buildInt32LE(&nonce[i]);
	}

	ctx->state.counter = 1;

	setMemory(ctx->keystream, 0, 64);
}

void keystreamFactory(Chacha20_State* previousState, Byte outputBlock[64]) {
	Chacha20_State working_state = *previousState;
	//Byte tempArr[4] = { 0 };

	for (uint8 i = 0; i < 10; i++) {
		QuarterRound(&working_state.raw[0], &working_state.raw[4], &working_state.raw[8], &working_state.raw[12]);
		QuarterRound(&working_state.raw[1], &working_state.raw[5], &working_state.raw[9], &working_state.raw[13]);
		QuarterRound(&working_state.raw[2], &working_state.raw[6], &working_state.raw[10], &working_state.raw[14]);
		QuarterRound(&working_state.raw[3], &working_state.raw[7], &working_state.raw[11], &working_state.raw[15]);

		QuarterRound(&working_state.raw[0], &working_state.raw[5], &working_state.raw[10], &working_state.raw[15]);
		QuarterRound(&working_state.raw[1], &working_state.raw[6], &working_state.raw[11], &working_state.raw[12]);
		QuarterRound(&working_state.raw[2], &working_state.raw[7], &working_state.raw[8], &working_state.raw[13]);
		QuarterRound(&working_state.raw[3], &working_state.raw[4], &working_state.raw[9], &working_state.raw[14]);
	}

	for (uint8 i = 0; i < 16; i++) {
		working_state.raw[i] += previousState->raw[i];
		breakInt32LE(working_state.raw[i], &outputBlock[i * 4]);
	}
}

void de_cipher(const Chacha20_Context_Ptr ctx, Byte* buffer, SizeT length) {
	for (SizeT i = 0; i < length; i++) {
		if (ctx->keystream_pos == 64) {
			keystreamFactory(&ctx->state, ctx->keystream);
			ctx->state.counter++;
			ctx->keystream_pos = 0;
		}

		buffer[i] ^= ctx->keystream[ctx->keystream_pos++];
	}
}

#pragma endregion