#include <stdio.h>
#include <string.h>
#include "asymmetric/elgamal/elgamal.h"
#include "symmetric/chacha20/chacha20.h"
#include "symmetric/chacha20poly1305/chacha20poly1305.h" // Nouvel import AEAD

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

	printf("Texte dechiffre :\n%s\n\n", decrypted_storage);
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
	printf("\n\n");
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

	printf("\n\n");

	FILE* fp = fopen("chacha20.key", "w");

	exportCC20Key(&key, fp, true);
}

void testChiffrementElGamalCleCheChe20() {
	printf("=== Test ElGamal + ChaCha20 : Generation Cles + nonce + Chiffrement ElGamal Cle + Nonce + Dechiffrement ===\n\n");

	printf("Generation de la paire El Gamal\n");
	EGKeyPair paireEG = generateEGKeyPair(3072);
	CC20Key cc20_key;

	printf("Generation de la cle CC20\n");
	CC20KeyGen(&cc20_key);

	// Traduction de la donnée en CustomInteger
	SizeT wordsNeeded = (CHACHA20_KEY_SIZE + WORD_SIZE - 1) / WORD_SIZE;
	CustomInteger temp = allocInteger(wordsNeeded);

	setMemory(temp.value, 0, temp.capacity * WORD_SIZE);

	copyMemory(&cc20_key, temp.value, CHACHA20_KEY_SIZE);
	temp.size = temp.capacity;

	printf("Chiffrement de la cle avec El Gamal\n");
	EGCiphered ciphered_key = cipherData(temp, paireEG.pub);

	FILE* tempKey_fp;
	FILE* c_fp;

	tempKey_fp = fopen("temp_key.hex", "w");
	c_fp = fopen("c.hex", "w");

	printf("Ecriture du chiffre et de sa cle temporaire dans des fichiers\n");
	writeToFile(&ciphered_key.tempKey, tempKey_fp, true);
	writeToFile(&ciphered_key.c, c_fp, true);

	EGCiphered fromFile;

	tempKey_fp = fopen("temp_key.hex", "r");
	c_fp = fopen("c.hex", "r");

	printf("Lecture du chiffre et de sa cle temporaire dans des fichiers\n");
	fromFile.tempKey = readFromFile(tempKey_fp, true);
	fromFile.c = readFromFile(c_fp, true);

	CustomInteger deciphered = decipherData(fromFile, paireEG);

	CC20Key cle_nonce;
	copyMemory(deciphered.value, &cle_nonce, CHACHA20_KEY_SIZE);

	uint8* cn = (uint8*)&cle_nonce,
		* c = (uint8*)&cc20_key;

	printf("Verification d'erreurs\n");
	for (SizeT i = 0; i < CHACHA20_KEY_SIZE; i++) {
		printf("[%zu]\t%02X\n", i, cn[i] ^ c[i]);
	}

	printf("Sauvegarde des clés\n\n");

	FILE* fp_pub, * fp_priv;

	fp_pub = fopen("elgamal.pub.key", "wb");
	fp_priv = fopen("elgamal.priv.key", "wb");

	exportEGPublicKey(&paireEG.pub, fp_pub, true);
	exportEGPrivateKey(&paireEG.priv, fp_priv, true);

	printf("Liberation de la memoire\n\n");
	freeInteger(&ciphered_key.c);
	freeInteger(&ciphered_key.tempKey);
	freeInteger(&fromFile.c);
	freeInteger(&fromFile.tempKey);
	freeEGKeyPair(&paireEG);
	freeInteger(&temp);
	freeInteger(&deciphered);
}

void testChaCha20Poly1305() {
	printf("=== Test ChaCha20-Poly1305 (Protocole AEAD complet) ===\n\n");

	const Byte key[CHACHA20POLY1305_KEY_SIZE] = {
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
		0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
		0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
		0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f
	};

	const Byte nonce[CHACHA20POLY1305_NONCE_SIZE] = {
		0x07, 0x00, 0x00, 0x00, 0x40, 0x41, 0x42, 0x43,
		0x44, 0x45, 0x46, 0x47
	};

	const char* message = "Donnees secretes pour le proxy eduroam !";
	SizeT pt_len = strlen(message);

	const char* aad = "IP:192.168.1.1;PORT:80"; // Entêtes en clair à authentifier
	SizeT aad_len = strlen(aad);

	Byte ciphertext[128] = { 0 };
	Byte decrypted[128] = { 0 };
	Byte tag[CHACHA20POLY1305_TAG_SIZE] = { 0 };

	// --- ETAPE 1 : CHIFFREMENT ET SCELLEMENT ---
	printf("1. Chiffrement et generation du Tag MAC...\n");
	ChaCha20Poly1305_Encrypt(
		key, nonce,
		(const Byte*)aad, aad_len,
		(const Byte*)message, pt_len,
		ciphertext,
		tag
	);

	printf("Tag obtenu (Hex) : ");
	for (int i = 0; i < CHACHA20POLY1305_TAG_SIZE; i++) {
		printf("%02x", tag[i]);
	}
	printf("\n\n");

	// --- ETAPE 2 : DECHIFFREMENT VALIDE ---
	printf("2. Test de dechiffrement avec un paquet intact...\n");
	int result = ChaCha20Poly1305_Decrypt(
		key, nonce,
		(const Byte*)aad, aad_len,
		ciphertext, pt_len,
		tag,
		decrypted
	);

	if (result == 0) {
		decrypted[pt_len] = '\0';
		printf("-> SUCCES : Intégrité validée. Texte recupere : '%s'\n\n", decrypted);
	}
	else {
		printf("-> ERREUR : Echec inattendu de l'authentification.\n\n");
	}

	// --- ETAPE 3 : TENTATIVE D'ATTAQUE (Bit-Flipping) ---
	printf("3. Simulation d'une attaque sur le reseau (Alteration du paquet)...\n");
	ciphertext[5] ^= 0x01; // On modifie un seul bit dans le texte chiffré !

	setMemory(decrypted, 0, sizeof(decrypted)); // On vide le buffer de déchiffrement

	result = ChaCha20Poly1305_Decrypt(
		key, nonce,
		(const Byte*)aad, aad_len,
		ciphertext, pt_len,
		tag,
		decrypted
	);

	if (result == -1) {
		printf("-> SUCCES DU BLOCAGE : Falsification detectee, le paquet a ete rejete !\n");
		printf("-> Buffer de dechiffrement : %s (Reste vide pour securite)\n\n", decrypted[0] == 0 ? "VIDE" : "COMPROMIS");
	}
	else {
		printf("-> DANGER : L'attaque est passee inaperçue !\n\n");
	}
}

void testLectureEcritureBlocSecurise() {
	printf("=== Test Fichiers : Lecture & Ecriture de Blocs Securises ===\n\n");

	// 1. Préparation des données factices
	const char* fake_message = "Ceci est un faux texte chiffre pour tester la manipulation de fichiers.";
	SizeT ct_len = strlen(fake_message);
	const Byte* fake_ciphertext = (const Byte*)fake_message;

	Byte fake_tag[CHACHA20POLY1305_TAG_SIZE] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
	};

	const char* filename = "test_secure_block.hex";

	// ==========================================
	// PHASE 1 : ÉCRITURE
	// ==========================================
	printf("1. Tentative d'ecriture de %zu octets dans '%s'...\n", ct_len, filename);
	FILE* fp_out = fopen(filename, "wb"); // Mode binaire obligatoire !

	if (fp_out != NULL) {
		bool write_ok = writeSecureBlock(fp_out, fake_ciphertext, ct_len, fake_tag);
		fclose(fp_out);

		if (write_ok) {
			printf("-> SUCCES : Bloc ecrit correctement.\n\n");
		} else {
			printf("-> ERREUR : Echec de l'ecriture du bloc.\n\n");
			return;
		}
	} else {
		printf("-> ERREUR : Impossible d'ouvrir le fichier en ecriture.\n\n");
		return;
	}

	// ==========================================
	// PHASE 2 : LECTURE
	// ==========================================
	printf("2. Tentative de lecture depuis '%s'...\n", filename);
	FILE* fp_in = fopen(filename, "rb");

	if (fp_in != NULL) {
		// On alloue un buffer avec calloc, selon ta préférence
		SizeT max_buffer_size = 1024;
		Byte* read_buffer = (Byte*)calloc(max_buffer_size, sizeof(Byte));
		Byte read_tag[CHACHA20POLY1305_TAG_SIZE] = { 0 };

		if (read_buffer != NULL) {
			int64 bytes_read = readSecureBlock(fp_in, read_buffer, max_buffer_size, read_tag);
			fclose(fp_in);

			if (bytes_read != -1) {
				printf("-> SUCCES : %ld octets lus.\n\n", (long)bytes_read);

				// ==========================================
				// PHASE 3 : VÉRIFICATION D'INTÉGRITÉ
				// ==========================================
				printf("3. Verification de l'integrite des donnees...\n");

				bool size_ok = (bytes_read == (int64)ct_len);

				bool content_ok = true;
				for (SizeT i = 0; i < ct_len; i++) {
					if (read_buffer[i] != fake_ciphertext[i]) content_ok = false;
				}

				bool tag_ok = true;
				for (int i = 0; i < CHACHA20POLY1305_TAG_SIZE; i++) {
					if (read_tag[i] != fake_tag[i]) tag_ok = false;
				}

				if (size_ok && content_ok && tag_ok) {
					printf("-> RESULTAT : PARFAIT ! Les donnees lues sont strictement identiques aux donnees ecrites.\n");
				} else {
					printf("-> RESULTAT : ECHEC ! Les donnees ont ete corrompues pendant l'I/O.\n");
				}

			} else {
				printf("-> ERREUR : Echec de readSecureBlock (fichier corrompu ou depassement de buffer).\n");
			}

			// Libération de la mémoire allouée par calloc
			free(read_buffer);

		} else {
			printf("-> ERREUR : Allocation memoire (calloc) a echoue.\n");
			fclose(fp_in);
		}
	} else {
		printf("-> ERREUR : Impossible d'ouvrir le fichier en lecture.\n");
	}
	printf("\n");
}

void testElGamalMultiCipherAndSignature() {
	printf("=== Test ElGamal : Multi-Cipher & Signature with Hashing ===\n\n");

	bool test_status = false;
	EGKeyPair pair = generateEGKeyPair(3072);

	const char* messages[3] = {
		"First block of highly sensitive production data.",
		"Second chunk containing financial transactions.",
		"Third partition for backup synchronization tokens."
	};
	SizeT msg_count = 3;

	CustomInteger msg_int = allocIntegerFromValue(0, false, true);
	EGCiphered ciphered = { 0 };
	CustomInteger deciphered = allocIntegerFromValue(0, false, true);
	bool cipher_success = true;

	for (SizeT i = 0; i < msg_count; i++) {
		freeInteger(&msg_int);
		freeInteger(&ciphered.tempKey);
		freeInteger(&ciphered.c);
		freeInteger(&deciphered);

		SizeT words = (strlen(messages[i]) + WORD_SIZE - 1) / WORD_SIZE;
		msg_int = allocInteger(words);
		setMemory(msg_int.value, 0, msg_int.capacity * WORD_SIZE);
		copyMemory((ptr)messages[i], msg_int.value, strlen(messages[i]));
		msg_int.size = msg_int.capacity;

		ciphered = cipherData(msg_int, pair.pub);
		deciphered = decipherData(ciphered, pair);

		if (compareAbs(msg_int, deciphered) != EQUALS) {
			cipher_success = false;
		}
	}

	Byte fake_sha256_digest[32] = {
		0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
		0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0xbc,
		0xb2, 0x27, 0x3d, 0x81, 0xd9, 0x07, 0xaf, 0xc8,
		0x47, 0xa1, 0x42, 0x4b, 0x9b, 0xbe, 0x07, 0x45
	};

	SizeT hash_words = (32 + WORD_SIZE - 1) / WORD_SIZE;
	CustomInteger hash_int = allocInteger(hash_words);
	setMemory(hash_int.value, 0, hash_int.capacity * WORD_SIZE);
	copyMemory(fake_sha256_digest, hash_int.value, 32);
	hash_int.size = hash_int.capacity;

	EGSignature signature = signData(hash_int, pair);
	bool signature_valid = verifySignature(hash_int, signature, pair.pub);

	if (cipher_success && signature_valid) {
		test_status = true;
	}

	if (test_status) {
		printf("-> SUCCESS: All blocks ciphered/deciphered and signature validated.\n\n");
	} else {
		printf("-> FAILURE: Integration test issues detected.\n\n");
	}

	freeInteger(&msg_int);
	freeInteger(&ciphered.tempKey);
	freeInteger(&ciphered.c);
	freeInteger(&deciphered);
	freeInteger(&hash_int);
	freeEGSignature(&signature);
	freeEGKeyPair(&pair);
}

int main() {
	//testChiffrementElGamalCleCheChe20();
	testElGamalMultiCipherAndSignature();

	//printf("--------------------------------------------------\n\n");

	testChaCha20Poly1305();
	testSauvegardeLectureCle();
	testLectureEcritureBlocSecurise();

	return 0;
}