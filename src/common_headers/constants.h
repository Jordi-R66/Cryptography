#pragma once

#include "includes.h"

typedef enum KnownCiphers {
	UNKNOWN_CIPHER = 0,
	EL_GAMAL = 1
} KnownCiphers;

typedef enum KeyType {
	UNKNOWN_KEYTYPE = 0,
	SECRET_KEY = 1,
	PUBLIC_KEY = 2,
	PRIVATE_KEY = 3
} KeyType;

typedef uint8 AlgoId;
typedef uint8 KeyT;