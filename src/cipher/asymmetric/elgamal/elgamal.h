#pragma once

#include "includes.h"
#include "../common/utils.h"

#pragma pack()

typedef struct {
	CustomInteger x, barrettMu_p, barrettMu_p1;
} EGPrivateKey;

typedef struct {
	CustomInteger p, a, q, h, barrettMu_p, barrettMu_p1;
} EGPublicKey;

typedef struct {
	EGPrivateKey priv;
	EGPublicKey pub;
} EGKeyPair;

typedef struct {
	CustomInteger tempKey, c;
} EGCiphered;

typedef struct {
	CustomInteger r, s;
} EGSignature;

#define EG_PRIVATE_KEY_SIZE sizeof(EGPrivateKey)
#define EG_PUBLIC_KEY_SIZE sizeof(EGPublicKey)
#define EG_SIGNATURE_SIZE sizeof(EGSignature)
#define EG_KEYPAIR_SIZE sizeof(EGKeyPair)

#pragma pack(1)

EGKeyPair generateEGKeyPair(SizeT bits);

EGCiphered cipherData(CustomInteger data, EGPublicKey pub);
CustomInteger decipherData(EGCiphered ciphered, EGKeyPair keyPair);

EGSignature signData(CustomInteger hash, EGKeyPair keyPair);
bool verifySignature(CustomInteger hash, EGSignature sig, EGPublicKey pub);

void freeEGPrivateKey(EGPrivateKey* priv);
void freeEGPublicKey(EGPublicKey* pub);
void freeEGSignature(EGSignature* sig);
void freeEGKeyPair(EGKeyPair* pair);

void printEGPublicKey(EGPublicKey* pub, Base base);
void printEGPrivateKey(EGPrivateKey* priv, Base base);
void printEGKeyPair(EGKeyPair* pair, Base base);

void exportEGPublicKey(EGPublicKey* pubkey, FILE* fp, bool closeAfterWriting);
void exportEGPrivateKey(EGPrivateKey* privkey, FILE* fp, bool closeAfterWriting);
EGPublicKey importEGPublicKey(FILE* fp, bool closeAfterReading);
EGPrivateKey importEGPrivateKey(FILE* fp, bool closeAfterReading);