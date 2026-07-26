#include "elgamal.h"

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

	CustomInteger p1 = { 0 }, One = allocIntegerFromValue(1, false, true);

	printf("Génération des 2 premiers...\n");
	generateSafePrimeParallel(bits, 12, q, p);
	printf("Calcul de mu pour p...\n");
	pub->barrettMu_p = getBarrettMu(*p);
	priv->barrettMu_p = copyIntegerToNew(pub->barrettMu_p);

	p1 = subtractInteger(*p, One);
	pub->barrettMu_p1 = getBarrettMu(p1);
	priv->barrettMu_p1 = copyIntegerToNew(pub->barrettMu_p1);

	freeInteger(&One);
	freeInteger(&p1);

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
	freeInteger(&pub->barrettMu_p1);
}

void freeEGPrivateKey(EGPrivateKey* priv) {
	freeInteger(&priv->x);
	freeInteger(&priv->barrettMu_p);
	freeInteger(&priv->barrettMu_p1);
}

void freeEGKeyPair(EGKeyPair* pair) {
	freeEGPublicKey(&pair->pub);
	freeEGPrivateKey(&pair->priv);
}

void freeEGSignature(EGSignature* sig) {
	freeInteger(&sig->r);
	freeInteger(&sig->s);
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

EGSignature signData(CustomInteger hash, EGKeyPair keyPair) {
	EGSignature output = { 0 };

	CustomInteger One = allocIntegerFromValue(1, false, true);
	CustomInteger p_minus_1 = subtractInteger(keyPair.pub.p, One);

	CustomInteger k = allocIntegerFromValue(0, false, true);
	CustomInteger gcd_val = allocIntegerFromValue(0, false, true);

	bool k_found = false;
	while (!k_found) {
		freeInteger(&k);
		freeInteger(&gcd_val);
		k = generateRandomCappedNumber(p_minus_1);
		gcd_val = gcdInteger(k, p_minus_1);
		if (equalsInteger(gcd_val, One) && !isZero(k)) {
			k_found = true;
		}
	}

	output.r = modPowInteger(keyPair.pub.a, k, keyPair.pub.p);

	CustomInteger k_inv = modularInverse(k, p_minus_1);
	CustomInteger xr = multiplyKaratsuba(keyPair.priv.x, output.r);
	CustomInteger xr_reduced = barrettReduce(xr, p_minus_1, keyPair.priv.barrettMu_p1);

	CustomInteger target = subtractInteger(hash, xr_reduced);
	if (target.isNegative) {
		CustomInteger temp_target = addInteger(target, p_minus_1);
		freeInteger(&target);
		target = temp_target;
	}

	CustomInteger factor = multiplyKaratsuba(k_inv, target);
	output.s = barrettReduce(factor, p_minus_1, keyPair.priv.barrettMu_p1);

	freeInteger(&One);
	freeInteger(&p_minus_1);
	freeInteger(&k);
	freeInteger(&gcd_val);
	freeInteger(&k_inv);
	freeInteger(&xr);
	freeInteger(&xr_reduced);
	freeInteger(&target);
	freeInteger(&factor);

	return output;
}

bool verifySignature(CustomInteger hash, EGSignature sig, EGPublicKey pub) {
	bool is_valid = false;

	CustomInteger Zero = allocIntegerFromValue(0, false, true);

	if (greaterThanInteger(sig.r, Zero) && lessThanInteger(sig.r, pub.p) &&
		greaterThanInteger(sig.s, Zero) && lessThanInteger(sig.s, pub.p)) {

		CustomInteger left = modPowInteger(pub.a, hash, pub.p);

		CustomInteger yr = modPowInteger(pub.h, sig.r, pub.p);
		CustomInteger rs = modPowInteger(sig.r, sig.s, pub.p);
		CustomInteger right_prod = multiplyKaratsuba(yr, rs);
		CustomInteger right = barrettReduce(right_prod, pub.p, pub.barrettMu_p);

		if (equalsInteger(left, right)) {
			is_valid = true;
		}

		freeInteger(&left);
		freeInteger(&yr);
		freeInteger(&rs);
		freeInteger(&right_prod);
		freeInteger(&right);
	}

	freeInteger(&Zero);

	return is_valid;
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
	printf("mu(p) = ");
	printInteger(pub->barrettMu_p, base, false);
	printf("mu(p-1) = ");
	printInteger(pub->barrettMu_p1, base, false);
}

void printEGPrivateKey(EGPrivateKey* priv, Base base) {
	printf(" === ELGAMAL PRIVATE KEY === \n");
	printf("x = ");
	printInteger(priv->x, base, false);
	printf("mu(p) = ");
	printInteger(priv->barrettMu_p, base, false);
	printf("mu(p-1) = ");
	printInteger(priv->barrettMu_p1, base, false);
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
		&pubkey->barrettMu_p,
		&pubkey->barrettMu_p1
	};

	if (fp != NULL) {
		fwrite(&algo, sizeof(AlgoId), 1, fp);
		fwrite(&keyType, sizeof(KeyT), 1, fp);

		for (SizeT i = 0; i < (sizeof(ints) / sizeof(CustomIntegerPtr)); i++) {
			CustomIntegerPtr custInt = ints[i];

			uint64 sizeInBytes = custInt->size * WORD_SIZE;
			fwrite(&sizeInBytes, sizeof(uint64), 1, fp);

			printInteger(*custInt, HEX, false);
			fwrite(custInt->value, WORD_SIZE, custInt->size, fp);
		}

		if (closeAfterWriting) {
			fclose(fp);
		}
	}

}

void exportEGPrivateKey(EGPrivateKey* privkey, FILE* fp, bool closeAfterWriting) {
	AlgoId algo = (AlgoId)EL_GAMAL;
	KeyT keyType = (KeyT)PRIVATE_KEY;

	CustomIntegerPtr ints[] = {
		&privkey->x,
		&privkey->barrettMu_p,
		&privkey->barrettMu_p1
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

		if (closeAfterWriting) {
			fclose(fp);
		}
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
		&output.barrettMu_p,
		&output.barrettMu_p1
	};

	if (fp != NULL) {
		fread(&algo, sizeof(AlgoId), 1, fp);
		fread(&keyType, sizeof(KeyT), 1, fp);

		if (algo == EL_GAMAL && keyType == PUBLIC_KEY) {
			for (SizeT i = 0; i < (sizeof(ints) / sizeof(CustomIntegerPtr)); i++) {
				CustomIntegerPtr custInt = ints[i];

				uint64 sizeInBytes;
				SizeT sizeInWords = 0;
				fread(&sizeInBytes, sizeof(uint64), 1, fp);

				sizeInWords = (((SizeT)sizeInBytes % WORD_SIZE) > 0) + ((SizeT)sizeInBytes / WORD_SIZE);

				*custInt = allocInteger(sizeInWords);
				custInt->size = custInt->capacity;

				custInt->size = (SizeT)fread(custInt->value, WORD_SIZE, sizeInWords, fp);
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
		&output.barrettMu_p,
		&output.barrettMu_p1
	};

	if (fp != NULL) {
		fread(&algo, sizeof(AlgoId), 1, fp);
		fread(&keyType, sizeof(KeyT), 1, fp);

		if (algo == EL_GAMAL && keyType == PRIVATE_KEY) {
			for (SizeT i = 0; i < (sizeof(ints) / sizeof(CustomIntegerPtr)); i++) {
				CustomIntegerPtr custInt = ints[i];

				uint64 sizeInBytes;
				SizeT sizeInWords = 0;
				fread(&sizeInBytes, sizeof(uint64), 1, fp);

				sizeInWords = (((SizeT)sizeInBytes % WORD_SIZE) > 0) + ((SizeT)sizeInBytes / WORD_SIZE);

				*custInt = allocInteger(sizeInWords);
				custInt->size = custInt->capacity;

				custInt->size = (SizeT)fread(custInt->value, WORD_SIZE, sizeInWords, fp);
			}

			if (closeAfterReading) {
				fclose(fp);
			}
		}
	}

	return output;
}

#pragma endregion
