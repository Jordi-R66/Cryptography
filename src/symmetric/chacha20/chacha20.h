#pragma once

#include "includes.h"

#pragma region Data

typedef union Chacha20_State {
	struct {
		uint32 MagicWord[4];
		uint32 key[8];
		uint32 counter;
		uint32 nonce[3];
	};
	uint32 raw[16];
} Chacha20_State;

typedef struct {
	Chacha20_State state;
	Byte keystream[64];
	SizeT keystream_pos;
} Chacha20_Context, *Chacha20_Context_Ptr;

#define CHACHA20_STATE_SIZE sizeof(Chacha20_State)
#define CHACHA20_CTX_SIZE sizeof(Chacha20_Context)

#pragma endregion

#pragma region Tools

uint32 buildInt32LE(const Byte bytes[4]);
void breakInt32LE(uint32 a, Byte bytes[4]);
uint32 ROTL32(uint32 a, uint32 n);

#pragma endregion

#pragma region Miscs

void QuarterRound(uint32* a, uint32* b, uint32* c, uint32* d);

#pragma endregion

#pragma region Heart

void initState(const Chacha20_Context_Ptr ctx, const Byte key[32], const Byte nonce[12]);

void keystreamFactory(Chacha20_State* previousState, Byte outputBlock[64]);

void de_cipher(const Chacha20_Context_Ptr ctx, Byte* buffer, SizeT length);

#pragma endregion