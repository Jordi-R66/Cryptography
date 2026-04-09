#include "common/utils.h"
#include "chacha20/chacha20.h"
#include <time.h>

int main(void) {
	uint32	a = 0x11111111,
			b = 0x01020304,
			c = 0x9b8d6f43,
			d = 0x01234567;

	printf("A : %08X\nB : %08X\nC : %08X\nD : %08X\n\n", a, b, c, d);

	QuarterRound(&a, &b, &c, &d);

	printf("A : %08X\nB : %08X\nC : %08X\nD : %08X\n", a, b, c, d);

	return 0;
}
