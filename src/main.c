#include "headers/utils.h"
#include <time.h>

int main(void) {
	CustomInteger p, q, n;

	time_t start, stop;

	start = time(NULL);
	p = generatePrime(4096);
	q = generatePrime(4096);
	stop  = time(NULL);
	n = multiplyInteger(p, q);
	reallocToFitInteger(&n);

	printInteger(p, HEX, false);
	printInteger(q, HEX, false);

	printf("Computed in %ld seconds\n", stop - start);
	// 137 secondes sur Ryzen 5 7533HS jusqu'à 4.4 GHz et 75°C

	printInteger(n, HEX, false);

	freeInteger(&p);
	freeInteger(&q);
	freeInteger(&n);

	return 0;
}