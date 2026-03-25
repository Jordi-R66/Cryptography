#pragma once

#include "includes.h"

CustomInteger generateRandomBase(CustomInteger n);
CustomInteger generateRandomInt(SizeT bits);
bool isProbablyPrime(CustomInteger n, uint32 k);
CustomInteger generatePrime(SizeT bits);
CustomInteger generatePrimeParallel(SizeT bits, int numThreads);