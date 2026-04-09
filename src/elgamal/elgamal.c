#include "elgamal.h"
#include "constants.h"

#pragma region Generation and Deletion

EGKeyPair generateEGKeyPair(SizeT bits) {
	EGKeyPair output = {};
	EGPrivateKey* priv = &output.priv;
	EGPublicKey* pub = &output.pub;

	CustomIntegerPtr x, p, q, a, h;

	x = &priv->x;

	a = &pub->a;
	p = &pub->p;
	q = &pub->q;
	h = &pub->h;

	printf("Génération des 2 premiers...\n");
	generateSafePrimeParallel(bits, 12, q, p);
	printf("Calcul de mu pour p...\n");
	pub->barrettMu_p = getBarrettMu(*p);
	priv->barrettMu_p = copyIntegerToNew(pub->barrettMu_p);

	printf("Génération de a...\n");
	*a = generateRandomInvertible(*p);
	printf("Génération de x...\n");
	*x = generateRandomCappedNumber(*q);
	printf("Génération de h...\n");
	*h = modPowInteger(*a, *x, *p);

	return output;
}

void freeEGPublicKey(EGPublicKey* pub) {
	freeInteger(&pub->p);
	freeInteger(&pub->a);
	freeInteger(&pub->q);
	freeInteger(&pub->h);
	freeInteger(&pub->barrettMu_p);
}

void freeEGPrivateKey(EGPrivateKey* priv) {
	freeInteger(&priv->x);
	freeInteger(&priv->barrettMu_p);
}

void freeEGKeyPair(EGKeyPair* pair) {
	freeEGPublicKey(&pair->pub);
	freeEGPrivateKey(&pair->priv);
}

#pragma endregion


EGCiphered cipherData(CustomInteger data, EGPublicKey pub) {
	EGCiphered output;

	CustomInteger r = allocIntegerFromValue(0, false, true), mask, temp;

	while (isZero(r)) {
		freeInteger(&r);
		r = generateRandomCappedNumber(pub.q);
	}

	output.tempKey = modPowInteger(pub.a, r, pub.p);
	mask = modPowInteger(pub.h, r, pub.p);
	temp = multiplyKaratsuba(data, mask);

	output.c = barrettReduce(temp, pub.p, pub.barrettMu_p);

	freeInteger(&mask);
	freeInteger(&temp);
	freeInteger(&r);

	return output;
}

CustomInteger decipherData(EGCiphered ciphered, EGKeyPair keyPair) {
	CustomInteger One = allocIntegerFromValue(1, false, true);

	CustomInteger p_minus_1 = subtractInteger(keyPair.pub.p, One);
	CustomInteger exp = subtractInteger(p_minus_1, keyPair.priv.x);

	CustomInteger mask_inv = modPowInteger(ciphered.tempKey, exp, keyPair.pub.p);

	CustomInteger temp = multiplyKaratsuba(ciphered.c, mask_inv);

	CustomInteger m = barrettReduce(temp, keyPair.pub.p, keyPair.priv.barrettMu_p);

	freeInteger(&One);
	freeInteger(&p_minus_1);
	freeInteger(&exp);
	freeInteger(&mask_inv);
	freeInteger(&temp);

	return m;
}

#pragma region IO

void printEGPublicKey(EGPublicKey* pub, Base base) {
	printf(" === ELGAMAL PUBLIC KEY === \n");
	printf("p = ");
	printInteger(pub->p, base, false);
	printf("a = ");
	printInteger(pub->a, base, false);
	printf("q = ");
	printInteger(pub->q, base, false);
	printf("h = ");
	printInteger(pub->h, base, false);
	printf("mu = ");
	printInteger(pub->barrettMu_p, base, false);
}

void printEGPrivateKey(EGPrivateKey* priv, Base base) {
	printf(" === ELGAMAL PRIVATE KEY === \n");
	printf("x = ");
	printInteger(priv->x, base, false);
	printf("mu = ");
	printInteger(priv->barrettMu_p, base, false);
}

void printEGKeyPair(EGKeyPair* pair, Base base) {
	printEGPublicKey(&pair->pub, base);
	printEGPrivateKey(&pair->priv, base);
}

void exportEGPublicKey(EGPublicKey* pubkey, FILE* fp, bool closeAfterWriting) {
	AlgoId algo = (AlgoId)EL_GAMAL;
	KeyT keyType = (KeyT)PUBLIC_KEY;

	CustomIntegerPtr ints[] = {
		&pubkey->p,
		&pubkey->a,
		&pubkey->q,
		&pubkey->h,
		&pubkey->barrettMu_p
	};

	if (fp != NULL) {
		fwrite(&algo, sizeof(AlgoId), 1, fp);
		fwrite(&keyType, sizeof(KeyT), 1, fp);

		for (SizeT i = 0; i < (sizeof(ints) / sizeof(CustomIntegerPtr)); i++) {
			CustomIntegerPtr custInt = ints[i];

			uint64 sizeInBytes = custInt->size * WORD_SIZE;
			fwrite(&sizeInBytes, sizeof(uint64), 1, fp);

			fwrite(custInt->value, WORD_SIZE, custInt->size, fp);
		}
	}

	if (closeAfterWriting) {
		fclose(fp);
	}
}

void exportEGPrivateKey(EGPrivateKey* privkey, FILE* fp, bool closeAfterWriting) {
	AlgoId algo = (AlgoId)EL_GAMAL;
	KeyT keyType = (KeyT)PRIVATE_KEY;

	CustomIntegerPtr ints[] = {
		&privkey->x,
		&privkey->barrettMu_p
	};

	if (fp != NULL) {
		fwrite(&algo, sizeof(AlgoId), 1, fp);
		fwrite(&keyType, sizeof(KeyT), 1, fp);

		for (SizeT i = 0; i < (sizeof(ints) / sizeof(CustomIntegerPtr)); i++) {
			CustomIntegerPtr custInt = ints[i];

			uint64 sizeInBytes = custInt->size * WORD_SIZE;
			fwrite(&sizeInBytes, sizeof(uint64), 1, fp);

			fwrite(custInt->value, WORD_SIZE, custInt->size, fp);
		}
	}

	if (closeAfterWriting) {
		fclose(fp);
	}
}

EGPublicKey importEGPublicKey(FILE* fp, bool closeAfterReading) {
	EGPublicKey output = { 0 };

	AlgoId algo; KeyT keyType;

	CustomIntegerPtr ints[] = {
		&output.p,
		&output.a,
		&output.q,
		&output.h,
		&output.barrettMu_p
	};

	if (fp != NULL) {
		fread(&algo, sizeof(AlgoId), 1, fp);
		fread(&keyType, sizeof(KeyT), 1, fp);

		if (algo == EL_GAMAL && keyType == PUBLIC_KEY) {
			for (SizeT i = 0; i < (sizeof(ints) / sizeof(CustomIntegerPtr)); i++) {
				CustomIntegerPtr custInt = ints[i];

				uint64 sizeInBytes;
				SizeT sizeInWords = 0;
				fwrite(&sizeInBytes, sizeof(uint64), 1, fp);

				sizeInWords = (((SizeT)sizeInBytes % WORD_SIZE) > 0) + ((SizeT)sizeInBytes / WORD_SIZE);

				*custInt = allocInteger(sizeInWords);
				custInt->size = custInt->capacity;

				custInt->size = (SizeT)fwrite(custInt->value, WORD_SIZE, sizeInWords, fp);
			}

			if (closeAfterReading) {
				fclose(fp);
			}
		}
	}

	return output;
}

EGPrivateKey importEGPrivateKey(FILE* fp, bool closeAfterReading) {
	EGPrivateKey output = { 0 };

	AlgoId algo; KeyT keyType;

	CustomIntegerPtr ints[] = {
		&output.x,
		&output.barrettMu_p
	};

	if (fp != NULL) {
		fread(&algo, sizeof(AlgoId), 1, fp);
		fread(&keyType, sizeof(KeyT), 1, fp);

		if (algo == EL_GAMAL && keyType == PRIVATE_KEY) {
			for (SizeT i = 0; i < (sizeof(ints) / sizeof(CustomIntegerPtr)); i++) {
				CustomIntegerPtr custInt = ints[i];

				uint64 sizeInBytes;
				SizeT sizeInWords = 0;
				fwrite(&sizeInBytes, sizeof(uint64), 1, fp);

				sizeInWords = (((SizeT)sizeInBytes % WORD_SIZE) > 0) + ((SizeT)sizeInBytes / WORD_SIZE);

				*custInt = allocInteger(sizeInWords);
				custInt->size = custInt->capacity;

				custInt->size = (SizeT)fwrite(custInt->value, WORD_SIZE, sizeInWords, fp);
			}

			if (closeAfterReading) {
				fclose(fp);
			}
		}
	}

	return output;
}

#pragma endregion
