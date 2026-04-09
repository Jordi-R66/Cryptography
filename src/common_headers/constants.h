#pragma once

typedef enum KnownCiphers {
	UNKNOWN = 0,
	EL_GAMAL = 1
} KnownCiphers;

typedef enum KeyType {
	UNKNOWN = 0,
	SECRET_KEY = 1,
	PUBLIC_KEY = 2,
	PRIVATE_KEY = 3
} KeyType;
