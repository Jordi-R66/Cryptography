#pragma once

#include "../common_headers/includes.h"
#include "../common_headers/utils.h"

#pragma pack()

typedef struct {
	CustomInteger x, barrettMu_p;
} EGPrivateKey;

typedef struct {
	CustomInteger p, a, q, h, barrettMu_p;
} EGPublicKey;

typedef struct {
	EGPrivateKey priv;
	EGPublicKey pub;
} EGKeyPair;

typedef struct {
	CustomInteger tempKey, c;
} EGCiphered;

#define EG_PUBLIC_KEY_SIZE sizeof(EGPublicKey);
#define EG_PRIVATE_KEY_SIZE sizeof(EGPrivateKey);
#define EG_KEYPAIR_SIZE sizeof(EGKeyPair)

#pragma pack(1)

EGKeyPair generateEGKeyPair(SizeT bits);

EGCiphered cipherData(CustomInteger data, EGPublicKey pub);
CustomInteger decipherData(EGCiphered ciphered, EGKeyPair keyPair);

void freeEGPublicKey(EGPublicKey* pub);
void freeEGPrivateKey(EGPrivateKey* priv);
void freeEGKeyPair(EGKeyPair* pair);

void printEGPublicKey(EGPublicKey* pub, Base base);
void printEGPrivateKey(EGPrivateKey* priv, Base base);
void printEGKeyPair(EGKeyPair* pair, Base base);

void exportEGPublicKey(EGPublicKey* pubkey, FILE* fp, bool closeAfterWriting);
void exportEGPrivateKey(EGPrivateKey* privkey, FILE* fp, bool closeAfterWriting);
EGPublicKey importEGPublicKey(FILE* fp, bool closeAfterReading);
EGPrivateKey importEGPrivateKey(FILE* fp, bool closeAfterReading);