#include <stdio.h>
#include <string.h>
#include "symmetric/chacha20/chacha20.h"

// On définit un tout petit buffer de travail (ex: 16 octets)
#define CHUNK_SIZE 16

void testChiffrementDechiffrement() {
	printf("=== Test ChaCha20 : Chiffrement & Dechiffrement par Chunk ===\n\n");

	const Byte key[32] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
	};
	const Byte nonce[12] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a,
		0x00, 0x00, 0x00, 0x00
	};

	const char* large_message = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
	SizeT total_len = strlen(large_message);

	// Ces tableaux simulent un fichier sur le disque dur ou une réception réseau
	Byte ciphertext_storage[256];
	char decrypted_storage[256];

	// Notre seule et unique zone de mémoire de travail (16 octets !)
	Byte buffer[CHUNK_SIZE];

	Chacha20_Context ctx;

	/* ==========================================
	 * PHASE 1 : CHIFFREMENT (Vers le stockage)
	 * ==========================================
	 */
	initState(&ctx, key, nonce);
	SizeT processed = 0;

	printf("Texte chiffre (Hex) par morceaux de %d octets :\n", CHUNK_SIZE);

	while (processed < total_len) {
		SizeT bytes_left = total_len - processed;
		SizeT current_chunk_size = (bytes_left > CHUNK_SIZE) ? CHUNK_SIZE : bytes_left;

		// 1. Lire depuis la source (simule un fread)
		memcpy(buffer, large_message + processed, current_chunk_size);

		// 2. Chiffrer sur place dans le buffer
		de_cipher(&ctx, buffer, current_chunk_size);

		// 3. Écrire le résultat dans le stockage (simule un fwrite) et afficher
		for (SizeT i = 0; i < current_chunk_size; i++) {
			printf("%02x ", buffer[i]);
			ciphertext_storage[processed + i] = buffer[i];
		}

		processed += current_chunk_size;
	}
	printf("\n\n");

	/* ============================================
	 * PHASE 2 : DÉCHIFFREMENT (Depuis le stockage)
	 * ============================================
	 */
	// RÈGLE D'OR : On DOIT réinitialiser le contexte pour reprendre le flux à zéro !
	initState(&ctx, key, nonce);
	processed = 0;

	while (processed < total_len) {
		SizeT bytes_left = total_len - processed;
		SizeT current_chunk_size = (bytes_left > CHUNK_SIZE) ? CHUNK_SIZE : bytes_left;

		// 1. Lire le texte chiffré depuis le stockage
		memcpy(buffer, ciphertext_storage + processed, current_chunk_size);

		// 2. Déchiffrer sur place (la magie de l'opération XOR)
		de_cipher(&ctx, buffer, current_chunk_size);

		// 3. Sauvegarder le texte en clair reconstitué
		memcpy(decrypted_storage + processed, buffer, current_chunk_size);

		processed += current_chunk_size;
	}

	// Ajout du caractère de fin de chaîne pour pouvoir l'afficher proprement
	decrypted_storage[total_len] = '\0';

	printf("Texte dechiffre :\n%s\n", decrypted_storage);
}

void testSauvegardeLectureCle() {
	printf("=== Test ChaCha20 : Sauvegarde & Lecture cle + nonce ===\n\n");

	CC20Key key1 = {
		.key = {2, 9, 9, 7, 9, 2, 4, 5},
		.nonce = {8, 667428, 11}
	};

	for (SizeT i = 0; i < 11; i++) {
		uint32* tempArr = (uint32*)&key1;
		printf("%08X ", tempArr[i]);
	}
	printf("\n");

	FILE* fp = fopen("chacha20.key", "w");

	exportCC20Key(&key1, fp, true);

	fp = fopen("chacha20.key", "r");

	CC20Key key2 = importCC20Key(fp, true);

	for (SizeT i = 0; i < 11; i++) {
		uint32* tempArr = (uint32*)&key2;
		printf("%08X ", tempArr[i]);
	}
	printf("\n");
}

void testGenerationSauvegardeCle() {
	printf("=== Test ChaCha20 : Generation & Sauvegarde cle + nonce ===\n\n");

	CC20Key key;
	printf("Generation de la cle\n");
	CC20KeyGen(&key);

	printf("Cle generee\n");
	for (SizeT i = 0; i < 11; i++) {
		uint32* tempArr = (uint32*)&key;
		printf("%08X ", tempArr[i]);
	}

	printf("\n");

	FILE* fp = fopen("chacha20.key", "w");

	exportCC20Key(&key, fp, true);
}

int main() {
	testGenerationSauvegardeCle();

	return 0;
}