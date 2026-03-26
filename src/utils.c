#include "common_headers/utils.h"
#include <pthread.h>

// Structure pour passer les arguments aux threads
typedef struct {
	SizeT bits;
	CustomInteger result;
	bool found;
	pthread_mutex_t* mutex;
} ThreadArgs;


Word modWord(CustomInteger a, Word b) {
	if (b == 0) return 0;

	DoubleWord remainder = 0;

	for (SizeT i = a.size; i > 0; i--) {
		remainder = ((remainder << 32) | (DoubleWord)a.value[i - 1]) % b;
	}

	return (Word)remainder;
}

bool isQuickCriblePassed(CustomInteger candidate) {
	Word smallPrimes[] = {
		3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 
		59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 
		113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 
		179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 
		239, 241, 251
	};
	int numSmallPrimes = sizeof(smallPrimes) / sizeof(smallPrimes[0]);

	for (int i = 0; i < numSmallPrimes; i++) {
		// modWord est l'optimisation que nous avons vue pour éviter Knuth ici
		Word remainder = modWord(candidate, smallPrimes[i]); 
		if (remainder == 0) {
			// Si le candidat est le nombre premier lui-même (ex: 8 bits)
			if (candidate.size == 1 && candidate.value[0] == smallPrimes[i]) {
				return true;
			}
			return false;
		}
	}
	return true;
}

void* primeWorker(void* vargs) {
	ThreadArgs* args = (ThreadArgs*)vargs;

	while (1) {
		// 1. Vérifier si un autre thread a déjà trouvé
		pthread_mutex_lock(args->mutex);
		if (args->found) {
			pthread_mutex_unlock(args->mutex);
			return NULL;
		}
		pthread_mutex_unlock(args->mutex);

		// 2. Tenter une génération (Crible + Miller-Rabin)
		CustomInteger candidate = generateRandomInt(args->bits);

		// (Applique ici ton petit crible et ton isProbablyPrime)
		if (isQuickCriblePassed(candidate) && isProbablyPrime(candidate, 5)) {
			pthread_mutex_lock(args->mutex);
			if (!args->found) {
				args->result = candidate; // On ne copie pas, on transfert le pointeur
				args->found = true;
			} else {
				freeInteger(&candidate);
			}
			pthread_mutex_unlock(args->mutex);
			return NULL;
		}
		
		freeInteger(&candidate);
	}
}

CustomInteger generatePrimeParallel(SizeT bits, int numThreads) {
	pthread_t threads[numThreads];
	ThreadArgs args;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	args.bits = bits;
	args.found = false;
	args.mutex = &mutex;

	for (int i = 0; i < numThreads; i++) {
		pthread_create(&threads[i], NULL, primeWorker, &args);
	}

	for (int i = 0; i < numThreads; i++) {
		pthread_join(threads[i], NULL);
	}

	return args.result;
}

// Calcule l'inverse modulaire d'un nombre impair modulo 2^32
Word inverseMod32(Word x) {
	Word inv = x; // Estimation initiale
	inv = inv * (2 - x * inv);
	inv = inv * (2 - x * inv);
	inv = inv * (2 - x * inv);
	inv = inv * (2 - x * inv);
	return inv;
}

CustomInteger montgomeryReduce(CustomInteger T, CustomInteger M, Word m_prime) {
	CustomInteger result = allocInteger(T.size > M.size * 2 ? T.size + 1 : M.size * 2 + 1);
	copyMemory(T.value, result.value, T.size * sizeof(Word));
	result.size = T.size;

	for (SizeT i = 0; i < M.size; i++) {
		Word u = result.value[i] * m_prime;

		DoubleWord carry = 0;
		for (SizeT j = 0; j < M.size; j++) {
			DoubleWord temp = (DoubleWord)result.value[i + j] +
				(DoubleWord)u * (DoubleWord)M.value[j] +
				carry;
			result.value[i + j] = (Word)(temp & 0xFFFFFFFF);
			carry = temp >> 32;
		}

		SizeT k = i + M.size;
		while (carry > 0) {
			if (k >= result.capacity) reallocInteger(&result, result.capacity + 1);
			DoubleWord temp = (DoubleWord)result.value[k] + carry;
			result.value[k] = (Word)(temp & 0xFFFFFFFF);
			carry = temp >> 32;
			if (k >= result.size) result.size = k + 1;
			k++;
		}
	}

	CustomInteger finalRes = allocInteger(M.size + 1);
	for (SizeT i = 0; i < result.size - M.size; i++) {
		finalRes.value[i] = result.value[i + M.size];
	}
	finalRes.size = result.size > M.size ? result.size - M.size : 1;

	freeInteger(&result);

	reallocToFitInteger(&finalRes);

	if (compareAbs(finalRes, M) != LESS) {
		CustomInteger tempRes = subtractInteger(finalRes, M);
		freeInteger(&finalRes);
		finalRes = tempRes;
	}

	return finalRes;
}

CustomInteger modPowMontgomery(CustomInteger base, CustomInteger exp, CustomInteger mod) {
	Word m_prime = (Word)0 - inverseMod32(mod.value[0]);
	SizeT N = mod.size;

	CustomInteger R2 = allocIntegerFromValue(1, false, true);
	BitshiftPtr(&R2, 2 * N * 32, LEFT, true);
	CustomInteger R2_mod = modInteger(R2, mod);
	freeInteger(&R2);

	CustomInteger base_prod = multiplyInteger(base, R2_mod);
	CustomInteger base_mont = montgomeryReduce(base_prod, mod, m_prime);
	freeInteger(&base_prod);

	CustomInteger One = allocIntegerFromValue(1, false, true);
	CustomInteger res_prod = multiplyInteger(One, R2_mod);
	CustomInteger result_mont = montgomeryReduce(res_prod, mod, m_prime);
	freeInteger(&res_prod);
	freeInteger(&R2_mod);

	SizeT maxBits = 0;
	if (exp.size > 0) {
		SizeT msWordIdx = exp.size;
		while (msWordIdx > 0 && exp.value[msWordIdx - 1] == 0) {
			msWordIdx--;
		}

		if (msWordIdx > 0) {
			Word topWord = exp.value[msWordIdx - 1];
			int msBit = 31;
			while (msBit >= 0 && !((topWord >> msBit) & 1)) {
				msBit--;
			}

			maxBits = (msWordIdx - 1) * 32 + msBit + 1;
		}
	}

	for (SizeT i = 0; i < maxBits; i++) {
		if (getBit(exp, i) == 1) {
			CustomInteger prod = multiplyInteger(result_mont, base_mont);
			CustomInteger newRes = montgomeryReduce(prod, mod, m_prime);
			freeInteger(&prod);
			freeInteger(&result_mont);
			result_mont = newRes;
		}

		if (i < maxBits - 1) {
			CustomInteger sq = multiplyInteger(base_mont, base_mont);
			CustomInteger newBase = montgomeryReduce(sq, mod, m_prime);
			freeInteger(&sq);
			freeInteger(&base_mont);
			base_mont = newBase;
		}
	}

	CustomInteger finalOutput = montgomeryReduce(result_mont, mod, m_prime);

	freeInteger(&One);
	freeInteger(&base_mont);
	freeInteger(&result_mont);

	return finalOutput;
}

CustomInteger generateRandomBase(CustomInteger n) {
	CustomInteger One = allocIntegerFromValue(1, false, true);
	CustomInteger Two = allocIntegerFromValue(2, false, true);
	CustomInteger n_minus_two = subtractInteger(n, Two);

	// On alloue 'a' à la même taille que 'n'
	CustomInteger a = allocInteger(n.size);
	a.size = n.size;

	FILE* urandom = fopen("/dev/urandom", "r");
	if (!urandom) {
		fprintf(stderr, "Erreur lecture /dev/urandom\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		// On remplit 'a' avec de l'aléatoire pur
		fread(a.value, sizeof(Word), n.size, urandom);

		// --- AJOUT : Masquage pour éviter le rejet infini ---
		Word topWordN = n.value[n.size - 1];
		int msBit = 31;
		while (msBit >= 0 && !((topWordN >> msBit) & 1)) {
			msBit--;
		}

		// Si le mot de poids fort n'utilise pas ses 32 bits, on masque a
		if (msBit < 31) {
			Word mask = (1U << (msBit + 1)) - 1;
			a.value[n.size - 1] &= mask;
		}
		// ----------------------------------------------------

		// Simule un reallocToFit local
		SizeT tempSize = n.size;
		while (tempSize > 1 && a.value[tempSize - 1] == 0) {
			tempSize--;
		}
		a.size = tempSize;

		// On vérifie : 2 <= a <= n-2
		if (greaterThanInteger(a, One) && (lessThanInteger(a, n_minus_two) || equalsInteger(a, n_minus_two))) {
			break; // C'est gagné !
		}

		a.size = n.size;
	}

	fclose(urandom);
	freeInteger(&One);
	freeInteger(&Two);
	freeInteger(&n_minus_two);

	reallocToFitInteger(&a);
	return a;
}

CustomInteger generateRandomInt(SizeT bits) {
	SizeT nbWords = (bits + 31) / 32;
	CustomInteger result = allocInteger(nbWords);

	result.size = nbWords;

	FILE* urandom = fopen("/dev/urandom", "r");

	if (!urandom) {
		freeInteger(&result);
		fprintf(stderr, "Couldn't open /dev/urandom\n");
		exit(EXIT_FAILURE);
	}

	fread(result.value, sizeof(Word), nbWords, urandom);
	fclose(urandom);

	SizeT extraBits = (nbWords << 5) - bits;
	if (extraBits > 0) {
		result.value[nbWords - 1] &= (0xFFFFFFFF >> extraBits);
	}

	SizeT msBitIndex = (bits - 1) % 32;
	result.value[nbWords - 1] |= (1 << msBitIndex);

	result.value[0] |= 1;

	reallocToFitInteger(&result);

	return result;
}

bool isProbablyPrime(CustomInteger n, uint32 k) {
	bool isPrime = true;
	CustomInteger One = allocIntegerFromValue(1, false, true);
	CustomInteger Two = allocIntegerFromValue(2, false, true);
	CustomInteger n_minus_1 = subtractInteger(n, One);

	// Étape A : Factoriser n - 1 = 2^s * d
	CustomInteger d = copyIntegerToNew(n_minus_1);
	SizeT s = 0;

	while (d.size > 0 && (d.value[0] & 1) == 0) {
		BitshiftPtr(&d, 1, RIGHT, true);
		s++;
	}

	// Étape B : Boucle de k tests
	for (uint32 i = 0; i < k; i++) {
		CustomInteger a = generateRandomBase(n);
		CustomInteger x = modPowMontgomery(a, d, n);

		freeInteger(&a);

		if (equalsInteger(x, One) || equalsInteger(x, n_minus_1)) {
			freeInteger(&x);
			continue;
		}

		bool innerLoopPassed = false;

		for (SizeT r = 1; r < s; r++) {
			CustomInteger x_sq = modPowMontgomery(x, Two, n);
			freeInteger(&x);
			x = x_sq;

			if (equalsInteger(x, One)) {
				freeInteger(&x);
				isPrime = false;
				break;
			}

			if (equalsInteger(x, n_minus_1)) {
				innerLoopPassed = true;
				break;
			}
		}

		if (!innerLoopPassed && !equalsInteger(x, n_minus_1)) {
			isPrime = false;
		}

		freeInteger(&x);

		if (!isPrime) {
			break;
		}
	}

	freeInteger(&One);
	freeInteger(&Two);
	freeInteger(&n_minus_1);
	freeInteger(&d);

	return isPrime;
}

CustomInteger generatePrime(SizeT bits) {
	// Liste des premiers petits nombres premiers
	Word smallPrimes[] = {
		3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
		59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109,
		113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
		179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
		239, 241, 251
	};
	int numSmallPrimes = sizeof(smallPrimes) / sizeof(smallPrimes[0]);

	while (true) {
		CustomInteger candidate = generateRandomInt(bits);
		bool isDivisible = false;

		// 1. Crible rapide (Trial Division)
		for (int i = 0; i < numSmallPrimes; i++) {
			Word remainder = modWord(candidate, smallPrimes[i]);
			if (remainder == 0) {
				isDivisible = !(candidate.size == 1 && candidate.value[0] == smallPrimes[i]);
				break;
			}
		}

		if (isDivisible) {
			freeInteger(&candidate);
			continue; // Échec du crible, on recommence
		}

		if (isProbablyPrime(candidate, 5)) {
			return candidate;
		} else {
			freeInteger(&candidate);
		}
	}
}
