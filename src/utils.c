#include "common_headers/utils.h"
#include <pthread.h>
#include <sys/random.h>
#include <errno.h>

// Structure pour passer les arguments aux threads
typedef struct {
	SizeT bits;
	CustomInteger result;
	bool found;
	pthread_mutex_t* mutex;
} PrimeGenThreadArgs;

typedef struct {
	SizeT bits;
	CustomInteger result_q;
	CustomInteger result_p;
	bool found;
	pthread_mutex_t* mutex;
} SafePrimeGenArgs;

Word modWord(CustomInteger a, Word b) {
	if (b == 0) return 0;

	DoubleWord remainder = 0;

	for (SizeT i = a.size; i > 0; i--) {
		remainder = ((remainder << (WORD_SIZE * 8)) | (DoubleWord)a.value[i - 1]) % b;
	}

	return (Word)remainder;
}

bool isQuickCriblePassed(CustomInteger candidate) {
	static const Word smallPrimes[] = {
		3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 
		79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 
		163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 
		241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 
		337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 
		431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 
		521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 
		617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 
		719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 
		823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 
		929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997
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

void* safePrimeWorker(void* vargs) {
	SafePrimeGenArgs* args = (SafePrimeGenArgs*)vargs;

	CustomInteger One = allocIntegerFromValue(1, false, true);
	CustomInteger Two = allocIntegerFromValue(2, false, true);

	SizeT i = 0;

	while (1) {
		printf("(%zu) %zu\n", args->bits, i++);
		CustomInteger q = generateRandomInt(args->bits);
		int retry_count = 0;

		while (retry_count < 20000) {
			pthread_mutex_lock(args->mutex);

			if (args->found) {
				pthread_mutex_unlock(args->mutex);
				freeInteger(&q);
				freeInteger(&One); freeInteger(&Two);
				return NULL;
			}
			pthread_mutex_unlock(args->mutex);

			if (!isQuickCriblePassed(q)) {
				goto next_candidate;
			}

			// Calcul de p = 2q + 1
			CustomInteger temp = multiplyKaratsuba(Two, q);
			CustomInteger p = addInteger(temp, One);
			freeInteger(&temp);

			if (!isQuickCriblePassed(p)) {
				freeInteger(&p);
				goto next_candidate;
			}

			if (!isProbablyPrime(q, 5)) {
				freeInteger(&p);
				goto next_candidate;
			}

			if (!isProbablyPrime(p, 5)) {
				freeInteger(&p);
				goto next_candidate;
			}

			// --- SUCCÈS ---
			pthread_mutex_lock(args->mutex);
			if (!args->found) {
				args->result_q = q;
				args->result_p = p;
				args->found = true;
			} else {
				freeInteger(&q);
				freeInteger(&p);
			}
			pthread_mutex_unlock(args->mutex);

			freeInteger(&One); freeInteger(&Two);
			return NULL;

		next_candidate:
			retry_count++;
			CustomInteger next_q = addInteger(q, Two);
			freeInteger(&q);
			q = next_q;
		}

		freeInteger(&q); // Au bout de 20 000 échecs, on relance un tirage complet
	}
}

void* primeWorker(void* vargs) {
	PrimeGenThreadArgs* args = (PrimeGenThreadArgs*)vargs;

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

void generateSafePrimeParallel(SizeT bits, int numThreads, CustomIntegerPtr out_q, CustomIntegerPtr out_p) {
	pthread_t threads[numThreads];
	SafePrimeGenArgs args;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	args.bits = bits;
	args.found = false;
	args.mutex = &mutex;

	for (int i = 0; i < numThreads; i++) {
		pthread_create(&threads[i], NULL, safePrimeWorker, &args);
	}

	for (int i = 0; i < numThreads; i++) {
		pthread_join(threads[i], NULL);
	}

	*out_q = args.result_q;
	*out_p = args.result_p;
}

CustomInteger generatePrimeParallel(SizeT bits, int numThreads) {
	pthread_t threads[numThreads];
	PrimeGenThreadArgs args;
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

// Calcule l'inverse modulaire d'un nombre impair modulo 2^(WORD_SIZE * 8)
Word inverseModWord(Word x) {
	Word inv = x; // Estimation initiale

	for (SizeT i = 0; i < WORD_SIZE; i++) {
		inv = inv * (2 - x * inv);
	}

	return inv;
}

CustomInteger montgomeryReduce(CustomInteger T, CustomInteger M, Word m_prime) {
	SizeT requiredCapacity = T.size > M.size * 2 ? T.size + 1 : M.size * 2 + 1;
	CustomInteger result = allocInteger(requiredCapacity);
	
	copyMemory(T.value, result.value, T.size * sizeof(Word));
	result.size = requiredCapacity; 

	for (SizeT i = 0; i < M.size; i++) {
		Word u = result.value[i] * m_prime;

		DoubleWord carry = 0;
		for (SizeT j = 0; j < M.size; j++) {
			DoubleWord temp = (DoubleWord)result.value[i + j] +
				(DoubleWord)u * (DoubleWord)M.value[j] +
				carry;
			result.value[i + j] = (Word)(temp & WORD_MAX_VAL);
			carry = temp >> (WORD_SIZE * 8);
		}

		SizeT k = i + M.size;
		while (carry > 0) {
			if (k >= result.capacity) {
				reallocInteger(&result, result.capacity + 1);
				result.size = result.capacity;
			}
			DoubleWord temp = (DoubleWord)result.value[k] + carry;
			result.value[k] = (Word)(temp & WORD_MAX_VAL);
			carry = temp >> (WORD_SIZE * 8);
			k++;
		}
	}

	SizeT wordsToCopy = (result.size > M.size) ? (result.size - M.size) : 0;
	
	CustomInteger finalRes = allocInteger(wordsToCopy > 0 ? wordsToCopy + 1 : 2);
	finalRes.size = wordsToCopy > 0 ? wordsToCopy : 1;

	if (wordsToCopy > 0) {
		for (SizeT i = 0; i < wordsToCopy; i++) {
			finalRes.value[i] = result.value[i + M.size];
		}
	} else {
		finalRes.value[0] = 0;
	}

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
	Word m_prime = (Word)0 - inverseModWord(mod.value[0]);
	SizeT N = mod.size;

	CustomInteger R2_mod = allocIntegerFromValue(1, false, true);
	SizeT max_shifts = 2 * N * (WORD_SIZE * 8);

	for (SizeT k = 0; k < max_shifts; k++) {
		// 1. Décalage de 1 bit vers la gauche (équivaut à * 2)
		BitshiftPtr(&R2_mod, 1, LEFT, true); 

		// 2. Si R2_mod >= mod, on soustrait mod
		if (compareIntegers(R2_mod, mod) != LESS) {
			CustomInteger temp = subtractInteger(R2_mod, mod);
			freeInteger(&R2_mod);
			R2_mod = temp;
		}
	}

	CustomInteger base_prod = multiplyKaratsuba(base, R2_mod);
	CustomInteger base_mont = montgomeryReduce(base_prod, mod, m_prime);
	freeInteger(&base_prod);

	SizeT maxBits = 0;
	if (exp.size > 0) {
		SizeT msWordIdx = exp.size;
		while (msWordIdx > 0 && exp.value[msWordIdx - 1] == 0) msWordIdx--;

		if (msWordIdx > 0) {
			Word topWord = exp.value[msWordIdx - 1];
			sSizeT msBit = ((WORD_SIZE * 8 ) - 1);
			while (msBit >= 0 && !((topWord >> msBit) & 1)) msBit--;
			maxBits = (msWordIdx - 1) * (WORD_SIZE * 8) + msBit + 1;
		}
	}

	#define MONT_WINDOW_SIZE 5
	#define MONT_TABLE_SIZE (1 << (MONT_WINDOW_SIZE - 1)) // 16 valeurs

	CustomInteger precomputed[MONT_TABLE_SIZE];
	precomputed[0] = copyIntegerToNew(base_mont);

	CustomInteger sq_prod = squareInteger(base_mont);
	CustomInteger base2 = montgomeryReduce(sq_prod, mod, m_prime);
	freeInteger(&sq_prod);

	for (int k = 1; k < MONT_TABLE_SIZE; k++) {
		CustomInteger prod = multiplyKaratsuba(precomputed[k - 1], base2);
		precomputed[k] = montgomeryReduce(prod, mod, m_prime);
		freeInteger(&prod);
	}
	freeInteger(&base2);

	CustomInteger One = allocIntegerFromValue(1, false, true);
	CustomInteger res_prod = multiplyKaratsuba(One, R2_mod);
	CustomInteger result_mont = montgomeryReduce(res_prod, mod, m_prime);
	freeInteger(&res_prod);
	freeInteger(&R2_mod);

	SizeT i = maxBits;

	while (i > 0) {
		if (getBit(exp, i - 1) == 0) {
			CustomInteger sq = squareInteger(result_mont);
			CustomInteger newRes = montgomeryReduce(sq, mod, m_prime);
			freeInteger(&result_mont);
			freeInteger(&sq);
			result_mont = newRes;
			i--;
		} else {
			SizeT j = (i > MONT_WINDOW_SIZE) ? i - MONT_WINDOW_SIZE : 0;
			while (getBit(exp, j) == 0) j++;

			Word windowVal = 0;
			for (SizeT k = i; k > j; k--) {
				windowVal = (windowVal << 1) | getBit(exp, k - 1);
			}

			for (SizeT k = i; k > j; k--) {
				CustomInteger sq = squareInteger(result_mont);
				CustomInteger newRes = montgomeryReduce(sq, mod, m_prime);
				freeInteger(&result_mont);
				freeInteger(&sq);
				result_mont = newRes;
			}

			CustomInteger prod = multiplyKaratsuba(result_mont, precomputed[windowVal / 2]);
			CustomInteger newRes = montgomeryReduce(prod, mod, m_prime);
			freeInteger(&result_mont);
			freeInteger(&prod);
			result_mont = newRes;

			i = j;
		}
	}

	CustomInteger finalOutput = montgomeryReduce(result_mont, mod, m_prime);

	freeInteger(&One);
	freeInteger(&base_mont);
	freeInteger(&result_mont);
	for (int k = 0; k < MONT_TABLE_SIZE; k++) {
		freeInteger(&precomputed[k]);
	}

	return finalOutput;
}

size_t getRandom(void* buffer, size_t length) {
	FILE* f = fopen("/dev/urandom", "r");
	size_t ret = fread(buffer, 1, length, f);
	fclose(f);

	return ret;
}

/**
 * @brief Generates a random integer between [0; limit[
 * 
 * @param limit 
 * @return CustomInteger 
 */
CustomInteger generateRandomCappedNumber(CustomInteger limit) {
	if (isZero(limit)) {
		return allocIntegerFromValue(0, false, true);
	}

	CustomInteger result = allocInteger(limit.size);
	result.size = limit.size;

	while (1) {
		// 1. On tire des octets 100% aléatoires purs
		size_t bytes_read = getRandom(result.value, limit.size * sizeof(Word));
		if (bytes_read == 0) {
			fprintf(stderr, "Fatal Error : getRandom() failed\n");
			exit(EXIT_FAILURE);
		}

		// 2. On masque les bits excédentaires pour ne pas dépasser inutilement
		Word topWordLimit = limit.value[limit.size - 1];
		sSizeT msBit = ((WORD_SIZE * 8 ) - 1);
		while (msBit >= 0 && !((topWordLimit >> msBit) & 1)) {
			msBit--;
		}

		if (msBit < ((WORD_SIZE * 8 ) - 1)) {
			Word mask = WORD_MAX_VAL >> ((WORD_SIZE * 8) - 1 - msBit);
			result.value[limit.size - 1] &= mask;
		}

		reallocToFitInteger(&result);

		// 3. Rejection Sampling : Si on est strictement inférieur, on a gagné !
		// Sinon on recommence la boucle (ce qui supprime totalement le Modulo Bias)
		if (compareIntegers(result, limit) == LESS) {
			break;
		}

		// Reset de la taille pour le prochain appel à getRandom
		result.size = limit.size; 
	}

	return result;
}

CustomInteger generateRandomInvertible(CustomInteger n) {
	CustomInteger One = allocIntegerFromValue(1, false, true);
	CustomInteger candidate;

	while (1) {
		// On utilise notre nouvelle fonction parfaitement bornée !
		candidate = generateRandomCappedNumber(n);

		if (isZero(candidate)) {
			freeInteger(&candidate);
			continue;
		}

		CustomInteger gcd = steinGcdInteger(candidate, n);

		if (equalsInteger(gcd, One)) {
			freeInteger(&gcd);
			break; 
		}

		freeInteger(&gcd);
		freeInteger(&candidate);
	}

	freeInteger(&One);
	return candidate;
}

CustomInteger generateRandomBase(CustomInteger n) {
	CustomInteger One = allocIntegerFromValue(1, false, true);
	CustomInteger Two = allocIntegerFromValue(2, false, true);
	CustomInteger n_minus_two = subtractInteger(n, Two);

	CustomInteger a = allocInteger(n.size);
	a.size = n.size;

	while (1) {
		size_t bytes_read = getRandom(a.value, n.size * sizeof(Word));

		if (bytes_read == 0) {
			fprintf(stderr, "Erreur fatale : getRandom() a échoué\n");
			exit(EXIT_FAILURE);
		}

		Word topWordN = n.value[n.size - 1];
		sSizeT msBit = ((WORD_SIZE * 8 ) - 1);
		while (msBit >= 0 && !((topWordN >> msBit) & 1)) {
			msBit--;
		}

		if (msBit < ((WORD_SIZE * 8 ) - 1)) {
			Word mask = WORD_MAX_VAL >> ((WORD_SIZE * 8) - 1 - msBit);
			a.value[n.size - 1] &= mask;
		}

		SizeT tempSize = n.size;
		while (tempSize > 1 && a.value[tempSize - 1] == 0) {
			tempSize--;
		}
		a.size = tempSize;

		if (greaterThanInteger(a, One) && (lessThanInteger(a, n_minus_two) || equalsInteger(a, n_minus_two))) {
			break;
		}

		a.size = n.size;
	}

	freeInteger(&One);
	freeInteger(&Two);
	freeInteger(&n_minus_two);

	reallocToFitInteger(&a);
	return a;
}

// Fonction blindée contre les interruptions système (GDB / Signaux)
void fillRandomBytes(void* buffer, SizeT length) {
	uint8_t* ptr = (uint8_t*)buffer;
	SizeT total_read = 0;

	while (total_read < length) {
		ssize_t bytes = getrandom(ptr + total_read, length - total_read, 0);
		
		if (bytes < 0) {
			// Si le syscall est interrompu par GDB ou l'OS, on l'ignore et on recommence
			if (errno == EINTR) {
				continue; 
			}
			fprintf(stderr, "Fatal Error : getrandom() failed\n");
			exit(EXIT_FAILURE);
		}
		
		total_read += (SizeT)bytes;
	}
}

CustomInteger generateRandomInt(SizeT bits) {
	SizeT nbWords = (bits + ((WORD_SIZE * 8 ) - 1)) / (WORD_SIZE * 8);
	CustomInteger result = allocInteger(nbWords);
	result.size = nbWords;

	// 1. Appel du nouveau wrapper blindé
	fillRandomBytes(result.value, nbWords * sizeof(Word));

	SizeT extraBits = (nbWords * WORD_SIZE * 8) - bits;;
	if (extraBits > 0) {
		result.value[nbWords - 1] &= (WORD_MAX_VAL >> extraBits);
	}

	sSizeT msBitIndex = (bits - 1) % (WORD_SIZE * 8);

	// 2. FIX CRITIQUE : Utilisation de 1U (Unsigned) pour éviter l'Undefined Behavior
	result.value[nbWords - 1] |= (((Word)1) << msBitIndex);

	result.value[0] |= 1;

	reallocToFitInteger(&result);

	return result;
}

bool isProbablyPrime(CustomInteger n, Word k) {
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
	for (Word i = 0; i < k; i++) {
		CustomInteger a = generateRandomBase(n);
		CustomInteger x = modPowMontgomery(a, d, n);

		freeInteger(&a);

		if (equalsInteger(x, One) || equalsInteger(x, n_minus_1)) {
			freeInteger(&x);
			continue;
		}

		bool innerLoopPassed = false;

		for (SizeT r = 1; r < s; r++) {
			//printf("[DEBUG] x_sq computed here:\n");
			CustomInteger x_sq = modPowMontgomery(x, Two, n);
			printInteger(x_sq, HEX, false);
			freeInteger(&x);
			x = x_sq;

			if (equalsInteger(x, One)) {
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
