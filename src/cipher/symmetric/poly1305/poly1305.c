#include "poly1305.h"
#include "memory/memfuncs.h"
#include "../chacha20/chacha20.h"

#include "poly1305.h"

#pragma region Heart

void Poly1305_Init(P1305CtxPtr ctx, const Byte key[POLY1305_KEY_SIZE]) {
	setMemory(ctx, 0, POLY1305_CONTEXT_SIZE);

	for (uint8 i = 0; i < 4; i++) {
		ctx->r[i] = buildInt32LE(&key[i * 4]) & (i == 0 ? 0x0fffffff : 0x0ffffffc);
	}

	for (uint8 i = 0; i < 4; i++) {
		ctx->s[i] = buildInt32LE(&key[16 + i * 4]);
	}
}

static void Poly1305_ProcessBlock(P1305CtxPtr ctx, const Byte block[16], uint8 is_padded) {
	uint32 h0 = ctx->a[0], h1 = ctx->a[1], h2 = ctx->a[2], h3 = ctx->a[3], h4 = ctx->a[4];
	uint32 r0 = ctx->r[0], r1 = ctx->r[1], r2 = ctx->r[2], r3 = ctx->r[3];

	uint32 pad_bit = is_padded == false ? 1 : 0;

	uint64 carry = 0;
	carry += (uint64)h0 + buildInt32LE(&block[0]);	h0 = (uint32)carry; carry >>= 32;
	carry += (uint64)h1 + buildInt32LE(&block[4]);	h1 = (uint32)carry; carry >>= 32;
	carry += (uint64)h2 + buildInt32LE(&block[8]);	h2 = (uint32)carry; carry >>= 32;
	carry += (uint64)h3 + buildInt32LE(&block[12]);	h3 = (uint32)carry; carry >>= 32;

	carry += (uint64)h4 + pad_bit;					h4 = (uint32)carry;

	uint32 s1 = r1 + (r1 >> 2);
	uint32 s2 = r2 + (r2 >> 2);
	uint32 s3 = r3 + (r3 >> 2);

	uint64 d0 = (uint64)h0 * r0 + (uint64)h1 * s3 + (uint64)h2 * s2 + (uint64)h3 * s1;
	uint64 d1 = (uint64)h0 * r1 + (uint64)h1 * r0 + (uint64)h2 * s3 + (uint64)h3 * s2 + (uint64)h4 * s1;
	uint64 d2 = (uint64)h0 * r2 + (uint64)h1 * r1 + (uint64)h2 * r0 + (uint64)h3 * s3 + (uint64)h4 * s2;
	uint64 d3 = (uint64)h0 * r3 + (uint64)h1 * r2 + (uint64)h2 * r1 + (uint64)h3 * r0 + (uint64)h4 * s3;
	uint64 d4 = (uint64)h4 * r0;

	carry = d0 >> 32;						h0 = (uint32)d0;
	d1 += carry;	carry = d1 >> 32;		h1 = (uint32)d1;
	d2 += carry;	carry = d2 >> 32;		h2 = (uint32)d2;
	d3 += carry;	carry = d3 >> 32;		h3 = (uint32)d3;
	d4 += carry;	h4 = (uint32)d4;

	uint32 c_out = h4 >> 2;
	h4 &= 3;

	carry = (uint64)h0 + c_out * 5; 	h0 = (uint32)carry; carry >>= 32;
	carry = (uint64)h1 + carry;     	h1 = (uint32)carry; carry >>= 32;
	carry = (uint64)h2 + carry;     	h2 = (uint32)carry; carry >>= 32;
	carry = (uint64)h3 + carry;     	h3 = (uint32)carry; carry >>= 32;
	carry = (uint64)h4 + carry;     	h4 = (uint32)carry;

	ctx->a[0] = h0; ctx->a[1] = h1; ctx->a[2] = h2; ctx->a[3] = h3; ctx->a[4] = h4;
}

void Poly1305_Update(P1305CtxPtr ctx, const Byte* data, SizeT length) {
	SizeT i = 0;

	while (i < length) {
		SizeT space_left = 16 - ctx->buffer_pos;
		SizeT to_copy = (length - i < space_left) ? (length - i) : space_left;

		copyMemory((ptr)(data + i), (ptr)(ctx->buffer + ctx->buffer_pos), to_copy);
		ctx->buffer_pos += to_copy;
		i += to_copy;

		if (ctx->buffer_pos == 16) {
			Poly1305_ProcessBlock(ctx, ctx->buffer, 0);
			ctx->buffer_pos = 0;
		}
	}
}

void Poly1305_Final(P1305CtxPtr ctx, Byte mac[POLY1305_MAC_SIZE]) {
	if (ctx->buffer_pos > 0) {
		ctx->buffer[ctx->buffer_pos] = 0x01;

		for (SizeT i = ctx->buffer_pos + 1; i < 16; i++) {
			ctx->buffer[i] = 0;
		}

		Poly1305_ProcessBlock(ctx, ctx->buffer, 1);
	}

	uint32 h0 = ctx->a[0], h1 = ctx->a[1], h2 = ctx->a[2], h3 = ctx->a[3], h4 = ctx->a[4];

	uint64 c = (uint64)h0 + 5;
	uint32 g0 = (uint32)c; c >>= 32;
	c += h1; uint32 g1 = (uint32)c; c >>= 32;
	c += h2; uint32 g2 = (uint32)c; c >>= 32;
	c += h3; uint32 g3 = (uint32)c; c >>= 32;
	c += h4; uint32 g4 = (uint32)c;

	uint32 mask = ~((g4 >> 2) - 1);

	h0 = (h0 & ~mask) | (g0 & mask);
	h1 = (h1 & ~mask) | (g1 & mask);
	h2 = (h2 & ~mask) | (g2 & mask);
	h3 = (h3 & ~mask) | (g3 & mask);
	h4 = (h4 & ~mask) | ((g4 & 3) & mask);

	uint64 carry = (uint64)h0 + ctx->s[0];	h0 = (uint32)carry; carry >>= 32;
	carry += (uint64)h1 + ctx->s[1];		h1 = (uint32)carry; carry >>= 32;
	carry += (uint64)h2 + ctx->s[2];		h2 = (uint32)carry; carry >>= 32;
	carry += (uint64)h3 + ctx->s[3];		h3 = (uint32)carry;

	breakInt32LE(h0, &mac[0]);
	breakInt32LE(h1, &mac[4]);
	breakInt32LE(h2, &mac[8]);
	breakInt32LE(h3, &mac[12]);

	setMemory(ctx, 0, POLY1305_CONTEXT_SIZE);
}

#pragma endregion