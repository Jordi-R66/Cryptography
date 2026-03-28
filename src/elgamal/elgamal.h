#pragma once

#include "../common_headers/includes.h"
#include "../common_headers/utils.h"

#pragma pack()

typedef struct {
	CustomInteger x;
} EGPrivateKey;

typedef struct {
	CustomInteger p, a, q, h;
} EGPublicKey;

typedef struct {
	EGPrivateKey priv;
	EGPublicKey pub;
} EGKeyPair;

#define EG_PUBLIC_KEY_SIZE sizeof(EGPublicKey);
#define EG_PRIVATE_KEY_SIZE sizeof(EGPrivateKey);
#define EG_KEYPAIR_SIZE sizeof(EGKeyPair)

#pragma pack(1)

EGKeyPair generateEGKeyPair(SizeT bits);

void freeEGPublicKey(EGPublicKey* pub);
void freeEGPrivateKey(EGPrivateKey* priv);
void freeEGKeyPair(EGKeyPair* pair);

void printEGPublicKey(EGPublicKey* pub, Base base);
void printEGPrivateKey(EGPrivateKey* priv, Base base);
void printEGKeyPair(EGKeyPair* pair, Base base);
