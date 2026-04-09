#include "aes128.h"

AES128_Key generateAes128Key() {
	AES128_Key output = { {0} };

	generateRandomBytes(output.vals, AES128_BLOCK_SIZE);

	return output;
}

AES128_Block encryptAes128Block(AES128_Block block, AES128_Key key);
AES128_Block decryptAes128Block(AES128_Block block, AES128_Key key);