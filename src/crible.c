#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LIMIT 75000

int main() {
	// Allocation du tableau de booléens pour le crible d'Ératosthène
	bool* is_prime = (bool*)malloc((LIMIT + 1) * sizeof(bool));
	for (int i = 0; i <= LIMIT; i++) {
		is_prime[i] = true;
	}

	is_prime[0] = false;
	is_prime[1] = false;

	for (int p = 2; p * p <= LIMIT; p++) {
		if (is_prime[p]) {
			for (int i = p * p; i <= LIMIT; i += p) {
				is_prime[i] = false;
			}
		}
	}

	FILE* file = fopen("primes_array.txt", "w");
	if (!file) {
		printf("Erreur lors de la création du fichier.\n");
		return 1;
	}

	fprintf(file, "static const Word smallPrimes[] = { ");

	int count = 0;
	for (int p = 2; p <= LIMIT; p++) {
		if (is_prime[p]) {
			fprintf(file, "%d, ", p);
			count++;
		}
	}

	fprintf(file, " };\n");
	//fprintf(file, "// Nombre total de nombres premiers : %d\n", count);

	fclose(file);
	free(is_prime);

	printf("Fichier 'primes_array.txt' généré avec succès ! (%d nombres premiers trouvés)\n", count);

	return 0;
}
