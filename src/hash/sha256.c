#include "sha256.h"

#define SHA256_BLOCK_SIZE 64
#define SHA256_DIGEST_SIZE 32

static const uint32 K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32 rightRotate(uint32 value, uint32 count) {
	uint32 result = 0;
	int isValid = (count < 32);

	if (isValid) {
		result = (value >> count) | (value << (32 - count));
	}

	return result;
}

static void processBlock(Sha256Context* ctx) {
	int isValid = (ctx != NULL);
	uint32 w[64] = { 0 };
	uint32 i = 0;
	uint32 a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
	uint32 s0 = 0, s1 = 0, maj = 0, t1 = 0, t2 = 0, ch = 0;

	if (isValid) {
		for (i = 0; i < 16; ++i) {
			w[i] = ((uint32)ctx->buffer[i * 4] << 24) |
				((uint32)ctx->buffer[i * 4 + 1] << 16) |
				((uint32)ctx->buffer[i * 4 + 2] << 8) |
				((uint32)ctx->buffer[i * 4 + 3]);
		}

		for (i = 16; i < 64; ++i) {
			s0 = rightRotate(w[i - 15], 7) ^ rightRotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
			s1 = rightRotate(w[i - 2], 17) ^ rightRotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		}

		a = ctx->state[0];
		b = ctx->state[1];
		c = ctx->state[2];
		d = ctx->state[3];
		e = ctx->state[4];
		f = ctx->state[5];
		g = ctx->state[6];
		h = ctx->state[7];

		for (i = 0; i < 64; ++i) {
			s1 = rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25);
			ch = (e & f) ^ (~e & g);
			t1 = h + s1 + ch + K[i] + w[i];
			s0 = rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22);
			maj = (a & b) ^ (a & c) ^ (b & c);
			t2 = s0 + maj;

			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		ctx->state[0] += a;
		ctx->state[1] += b;
		ctx->state[2] += c;
		ctx->state[3] += d;
		ctx->state[4] += e;
		ctx->state[5] += f;
		ctx->state[6] += g;
		ctx->state[7] += h;
	}
}

int sha256_init(Sha256Context* ctx) {
	int status = 0;
	int isValidContext = (ctx != NULL);
	uint32 i = 0;

	if (isValidContext) {
		ctx->state[0] = 0x6a09e667;
		ctx->state[1] = 0xbb67ae85;
		ctx->state[2] = 0x3c6ef372;
		ctx->state[3] = 0xa54ff53a;
		ctx->state[4] = 0x510e527f;
		ctx->state[5] = 0x9b05688c;
		ctx->state[6] = 0x1f83d9ab;
		ctx->state[7] = 0x5be0cd19;

		ctx->dataLength[0] = 0;
		ctx->dataLength[1] = 0;
		ctx->bufferLength = 0;

		for (i = 0; i < SHA256_BLOCK_SIZE; ++i) {
			ctx->buffer[i] = 0;
		}

		status = 1;
	}

	return status;
}

int sha256_update(Sha256Context* ctx, const Byte* data, SizeT len) {
	int status = 0;
	int isValidContext = (ctx != NULL);
	int isUpdateNeeded = (len > 0) && (data != NULL);
	SizeT i = 0;

	if (isValidContext) {
		status = 1;
		if (isUpdateNeeded) {
			for (i = 0; i < len; ++i) {
				ctx->buffer[ctx->bufferLength] = data[i];
				ctx->bufferLength++;

				ctx->dataLength[0] += 8;
				ctx->dataLength[1] += (ctx->dataLength[0] < 8);

				if (ctx->bufferLength == SHA256_BLOCK_SIZE) {
					processBlock(ctx);
					ctx->bufferLength = 0;
				}
			}
		}
	}

	return status;
}

int sha256_final(Sha256Context* ctx, Byte* digest) {
	int status = 0;
	int isValidParameters = (ctx != NULL) && (digest != NULL);
	uint32 currentLength = 0;
	uint32 i = 0;

	if (isValidParameters) {
		currentLength = ctx->bufferLength;
		ctx->buffer[currentLength] = 0x80;

		for (i = currentLength + 1; i < SHA256_BLOCK_SIZE; ++i) {
			ctx->buffer[i] = 0x00;
		}

		if (currentLength >= 56) {
			processBlock(ctx);
			for (i = 0; i < 56; ++i) {
				ctx->buffer[i] = 0x00;
			}
		}

		ctx->buffer[56] = (Byte)((ctx->dataLength[1] >> 24) & 0xFF);
		ctx->buffer[57] = (Byte)((ctx->dataLength[1] >> 16) & 0xFF);
		ctx->buffer[58] = (Byte)((ctx->dataLength[1] >> 8) & 0xFF);
		ctx->buffer[59] = (Byte)((ctx->dataLength[1]) & 0xFF);
		ctx->buffer[60] = (Byte)((ctx->dataLength[0] >> 24) & 0xFF);
		ctx->buffer[61] = (Byte)((ctx->dataLength[0] >> 16) & 0xFF);
		ctx->buffer[62] = (Byte)((ctx->dataLength[0] >> 8) & 0xFF);
		ctx->buffer[63] = (Byte)((ctx->dataLength[0]) & 0xFF);

		processBlock(ctx);

		for (i = 0; i < 8; ++i) {
			digest[i * 4] = (Byte)((ctx->state[i] >> 24) & 0xFF);
			digest[i * 4 + 1] = (Byte)((ctx->state[i] >> 16) & 0xFF);
			digest[i * 4 + 2] = (Byte)((ctx->state[i] >> 8) & 0xFF);
			digest[i * 4 + 3] = (Byte)((ctx->state[i]) & 0xFF);
		}

		status = 1;
	}

	return status;
}

bool computeSha256(const Byte* data, SizeT len, Byte* digest) {
	bool isValidParameters = ((data != NULL) || (len == 0)) && (digest != NULL);
	bool isInitSuccess = false;
	bool isUpdateSuccess = false;
	bool isFinalSuccess = false;
	Sha256Context ctx;

	if (isValidParameters) {
		isInitSuccess = sha256_init(&ctx);
	}

	if (isInitSuccess) {
		isUpdateSuccess = sha256_update(&ctx, data, len);
	}

	if (isUpdateSuccess) {
		isFinalSuccess = sha256_final(&ctx, digest);
	}

	return isFinalSuccess;
}