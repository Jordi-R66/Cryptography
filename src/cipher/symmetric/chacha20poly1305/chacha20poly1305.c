#include "chacha20poly1305.h"
#include "../chacha20/chacha20.h"
#include "../poly1305/poly1305.h"

#pragma region Utils

static void pad16(P1305CtxPtr poly_ctx, SizeT len) {
	SizeT remainder = len % 16;

	if (remainder != 0) {
		SizeT pad_len = 16 - remainder;
		Byte pad[15] = { 0 };
		Poly1305_Update(poly_ctx, pad, pad_len);
	}
}

static void serialize_length(uint64 len, Byte output[8]) {
	for (uint8 i = 0; i < 8; i++) {
		output[i] = 0xFF & (Byte)(len >> (i * 8));
	}
}

#pragma endregion

#pragma region Heart

void ChaCha20Poly1305_Encrypt(const Byte key[CHACHA20POLY1305_KEY_SIZE], const Byte nonce[CHACHA20POLY1305_NONCE_SIZE], const Byte* aad, SizeT aad_len, const Byte* plaintext, SizeT pt_len, Byte* ciphertext, Byte tag[CHACHA20POLY1305_TAG_SIZE]) {
	Chacha20_Context cc_ctx;
	Poly1305_Context poly_ctx;
	Byte poly_key_block[64];

	initState(&cc_ctx, key, nonce);
	cc_ctx.state.counter = 0;

	keystreamFactory(&cc_ctx.state, poly_key_block);
	Poly1305_Init(&poly_ctx, poly_key_block);

	setMemory(poly_key_block, 0, 64);

	cc_ctx.state.counter = 1;
	cc_ctx.keystream_pos = 64;

	if (plaintext != ciphertext && pt_len > 0) {
		copyMemory((ptr)plaintext, (ptr)ciphertext, pt_len);
	}

	if (pt_len > 0) {
		de_cipher(&cc_ctx, ciphertext, pt_len);
	}

	if (aad != NULL && aad_len > 0) {
		Poly1305_Update(&poly_ctx, aad, aad_len);
		pad16(&poly_ctx, aad_len);
	}

	if (pt_len > 0) {
		Poly1305_Update(&poly_ctx, ciphertext, pt_len);
		pad16(&poly_ctx, pt_len);
	}

	Byte len_block[16];
	serialize_length(aad_len, &len_block[0]);
	serialize_length(pt_len, &len_block[8]);
	Poly1305_Update(&poly_ctx, len_block, 16);

	Poly1305_Final(&poly_ctx, tag);
	setMemory(&cc_ctx, 0, CHACHA20_CTX_SIZE);
}


int ChaCha20Poly1305_Decrypt(
	const Byte key[CHACHA20POLY1305_KEY_SIZE],
	const Byte nonce[CHACHA20POLY1305_NONCE_SIZE],
	const Byte* aad, SizeT aad_len,
	const Byte* ciphertext, SizeT ct_len,
	const Byte tag[CHACHA20POLY1305_TAG_SIZE],
	Byte* plaintext
) {
	CC20Ctx cc_ctx;
	P1305Ctx poly_ctx;
	Byte poly_key_block[64];
	Byte expected_tag[16];

	initState(&cc_ctx, key, nonce);
	cc_ctx.state.counter = 0;

	keystreamFactory(&cc_ctx.state, poly_key_block);
	Poly1305_Init(&poly_ctx, poly_key_block);
	setMemory(poly_key_block, 0, 64);

	if (aad != NULL && aad_len > 0) {
		Poly1305_Update(&poly_ctx, aad, aad_len);
		pad16(&poly_ctx, aad_len);
	}

	if (ct_len > 0) {
		Poly1305_Update(&poly_ctx, ciphertext, ct_len);
		pad16(&poly_ctx, ct_len);
	}

	Byte len_block[16];
	serialize_length(aad_len, &len_block[0]);
	serialize_length(ct_len, &len_block[8]);
	Poly1305_Update(&poly_ctx, len_block, 16);

	Poly1305_Final(&poly_ctx, expected_tag);

	uint8 diff = 0;
	for (uint8 i = 0; i < 16; i++) {
		diff |= (expected_tag[i] ^ tag[i]);
	}

	if (diff != 0) {
		setMemory(&cc_ctx, 0, CHACHA20_CTX_SIZE);
		return -1;
	}

	cc_ctx.state.counter = 1;
	cc_ctx.keystream_pos = 64;

	if (plaintext != ciphertext && ct_len > 0) {
		for (SizeT i = 0; i < ct_len; i++) {
			plaintext[i] = ciphertext[i];
		}
	}

	if (ct_len > 0) {
		de_cipher(&cc_ctx, plaintext, ct_len);
	}

	setMemory(&cc_ctx, 0, CHACHA20_CTX_SIZE);
	return 0;
}

#pragma endregion

bool writeSecureBlock(FILE* fp, const Byte* ciphertext, SizeT ct_len, const Byte tag[CHACHA20POLY1305_TAG_SIZE]) {
	bool success = false;

	if (fp != NULL && ciphertext != NULL) {
		success = (fwrite(&ct_len, SIZET_SIZE, 1, fp) == 1);

		if (success && ct_len > 0) {
			success = (fwrite(ciphertext, 1, ct_len, fp) == ct_len);
		}

		if (success) {
			success = (fwrite(tag, 1, CHACHA20POLY1305_TAG_SIZE, fp) == CHACHA20POLY1305_TAG_SIZE);
		}
	}

	return success;
}

int64 readSecureBlock(FILE* fp, Byte* out_buffer, SizeT max_buffer_size, Byte tag[CHACHA20POLY1305_TAG_SIZE]) {
	int64 result = -1;
	SizeT ct_len = 0;
	bool success = false;

	if (fp != NULL && out_buffer != NULL) {
		if (fread(&ct_len, sizeof(SizeT), 1, fp) == 1) {

			if (ct_len <= max_buffer_size) {
				success = true;

				if (ct_len > 0) {
					if (fread(out_buffer, 1, ct_len, fp) != ct_len) {
						success = false;
					}
				}

				if (success) {
					if (fread(tag, 1, CHACHA20POLY1305_TAG_SIZE, fp) != CHACHA20POLY1305_TAG_SIZE) {
						success = false;
					}
				}
			}
		}
	}

	if (success) {
		result = (int64)ct_len;
	} else {
		if (out_buffer != NULL && max_buffer_size > 0) {
			setMemory(out_buffer, 0, max_buffer_size);
		}
	}

	return result;
}