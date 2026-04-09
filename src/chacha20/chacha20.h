#pragma once

#include "../common/includes.h"

#pragma region Tools

inline uint32 buildInt32LE(const Byte bytes[4]);
inline void breakInt32LE(uint32 a, Byte bytes[4]);
inline uint32 ROTL32(uint32 a, uint32 n);

#pragma endregion