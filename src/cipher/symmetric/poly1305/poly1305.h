#pragma once

#include "../../common/includes.h"

#pragma region Data

#define POLY1305_KEY_SIZE 32
#define POLY1305_MAC_SIZE 16
#define POLY1305_BLOCK_SIZE 16

typedef struct Poly1305_Context {
	uint32 r[4];
	uint32 s[4];
	uint32 a[5];
	Byte buffer[16];
	uint64 buffer_pos;
} Poly1305_Context, *Poly1305_Context_Ptr, P1305Ctx, *P1305CtxPtr;

#define POLY1305_CONTEXT_SIZE sizeof(Poly1305_Context)

#pragma endregion

void Poly1305_Init(P1305CtxPtr ctx, const Byte key[32]);
void Poly1305_Update(P1305CtxPtr ctx, const Byte* data, SizeT length);
void Poly1305_Final(P1305CtxPtr ctx, Byte mac[16]);
