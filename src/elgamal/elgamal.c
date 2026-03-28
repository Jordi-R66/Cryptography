#include "elgamal.h"

EGKeyPair generateEGKeyPair(SizeT bits) {
	EGKeyPair output = {};
	EGPrivateKey* priv = &output.priv;
	EGPublicKey* pub = &output.pub;

	CustomIntegerPtr x, p, q, a, h;

	CustomInteger One, Two, temp;

	One = allocIntegerFromValue(1, false, true);
	Two = allocIntegerFromValue(2, false, true);

	x = &priv->x;

	a = &pub->a;
	p = &pub->p;
	q = &pub->q;
	h = &pub->h;

	bool runLoop = true;

	while (runLoop) {
		*q = generatePrime(bits);
		temp = multiplyInteger(Two, *q);
		*p = addInteger(temp, One);

		freeInteger(&temp);

		runLoop = isProbablyPrime(*p, 5);

		if (!runLoop) {
			freeInteger(q);
			freeInteger(a);
		}
	}

	*a = generateRandomInvertible(*p);
	*x = generateRandomCappedNumber(*q);
	*h = modPowInteger(*a, *x, *p);

	freeInteger(&One);
	freeInteger(&Two);

	return output;
}

void freeEGPublicKey(EGPublicKey* pub) {
	freeInteger(&pub->p);
	freeInteger(&pub->a);
	freeInteger(&pub->q);
	freeInteger(&pub->h);
}

void freeEGPrivateKey(EGPrivateKey* priv) {
	freeInteger(&priv->x);
}

void freeEGKeyPair(EGKeyPair* pair) {
	freeEGPublicKey(&pair->pub);
	freeEGPrivateKey(&pair->priv);
}

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
}

void printEGPrivateKey(EGPrivateKey* priv, Base base) {
	printf(" === ELGAMAL PRIVATE KEY === \n");
	printf("x = ");
	printInteger(priv->x, base, false);
}

void printEGKeyPair(EGKeyPair* pair, Base base) {
	printEGPublicKey(&pair->pub, base);
	printEGPrivateKey(&pair->priv, base);
}
