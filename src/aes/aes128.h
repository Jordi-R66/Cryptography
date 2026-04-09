#pragma once

#include "../common/includes.h"
#include "../common/utils.h"
#include "aes_common.h"

#define AES128_BLOCK_SIZE 16 // 128 bits = 8 * 16 octets

#pragma pack()

typedef struct {
	Byte vals[AES128_BLOCK_SIZE];
} AES128_Block;

typedef AES128_Block AES128_Key;

#pragma pack(1)

AES128_Key generateAes128Key();
AES128_Block encryptAes128Block(AES128_Block block, AES128_Key key);
AES128_Block decryptAes128Block(AES128_Block block, AES128_Key key);