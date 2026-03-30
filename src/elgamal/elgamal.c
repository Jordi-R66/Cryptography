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

	generateSafePrimeParallel(bits, 12, q, p);
	pub->barrettMu_p = getBarrettMu(*p);
	priv->barrettMu_p = copyIntegerToNew(pub->barrettMu_p);

	*a = generateRandomInvertible(*p);
	*x = generateRandomCappedNumber(*q);
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

#pragma endregion
