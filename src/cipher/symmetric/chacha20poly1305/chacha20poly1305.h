#pragma once

#include "../../../common/includes.h"

#pragma region Constants

#define CHACHA20POLY1305_KEY_SIZE   32
#define CHACHA20POLY1305_NONCE_SIZE 12
#define CHACHA20POLY1305_TAG_SIZE   16

#pragma endregion

#pragma region Heart

/**
 * Chiffre un message et génère un tag d'authentification (AEAD).
 * 
 * @param key        Clé de 32 octets.
 * @param nonce      Nonce de 12 octets.
 * @param aad        Données additionnelles authentifiées (peuvent être NULL).
 * @param aad_len    Taille des AAD.
 * @param plaintext  Message à chiffrer.
 * @param pt_len     Taille du message.
 * @param ciphertext Buffer de sortie pour le texte chiffré (peut être égal à plaintext).
 * @param tag        Buffer de 16 octets pour recevoir le tag MAC.
 */
void ChaCha20Poly1305_Encrypt(
	const Byte key[CHACHA20POLY1305_KEY_SIZE],
	const Byte nonce[CHACHA20POLY1305_NONCE_SIZE],
	const Byte* aad, SizeT aad_len,
	const Byte* plaintext, SizeT pt_len,
	Byte* ciphertext,
	Byte tag[CHACHA20POLY1305_TAG_SIZE]
);

/**
 * Déchiffre un message et vérifie son intégrité.
 * 
 * @return 0 si l'authentification réussit, -1 si le message a été altéré.
 */
int ChaCha20Poly1305_Decrypt(
	const Byte key[CHACHA20POLY1305_KEY_SIZE],
	const Byte nonce[CHACHA20POLY1305_NONCE_SIZE],
	const Byte* aad, SizeT aad_len,
	const Byte* ciphertext, SizeT ct_len,
	const Byte tag[CHACHA20POLY1305_TAG_SIZE],
	Byte* plaintext
);

#pragma endregion

bool writeSecureBlock(FILE* fp, const Byte* ciphertext, SizeT ct_len, const Byte tag[CHACHA20POLY1305_TAG_SIZE]);
int64 readSecureBlock(FILE* fp, Byte* out_buffer, SizeT max_buffer_size, Byte tag[CHACHA20POLY1305_TAG_SIZE]);