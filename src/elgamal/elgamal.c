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