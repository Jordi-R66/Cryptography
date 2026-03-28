#include "common_headers/utils.h"
#include <time.h>

int main(void) {
	CustomInteger p, q, n;

	time_t start, stop;

	start = time(NULL);
	p = generatePrimeParallel(4096, 6);
	//q = generatePrimeParallel(2048, 12);
	stop = time(NULL);
	n = generateRandomInvertible(p);
	reallocToFitInteger(&n);

	printf("p=");
	printInteger(p, DEC, false);
	//printInteger(q, DEC, false);

	printf("Computed in %ld seconds\n", stop - start);
	// 137 secondes pour 2 premiers de 4096 bits sur Ryzen 5 7533HS jusqu'à 4.4 GHz et 75°C avec 1 seul thread
	// 42 secondes pour 2 premiers de 4096 bits sur Ryzen 5 7533HS jusqu'à 4.4 GHz et 82°C avec 12 threads

	printf("a=");
	printInteger(n, DEC, false);

	Euclide temp = ExtendedStein(n, p);

	printf("pgcd=");
	printInteger(temp.gcd, DEC, false);

	printf("u=");
	printInteger(temp.u, DEC, false);

	printf("v=");
	printInteger(temp.v, DEC, false);

	freeInteger(&p);
	//freeInteger(&q);
	freeInteger(&n);

	return 0;
}
