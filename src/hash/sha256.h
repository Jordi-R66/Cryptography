#pragma once

#ifndef SHA256_H
#define SHA256_H

#include "../common/includes.h"

/**
 * @file sha256.h
 * @brief SHA-256 algorithm implementation utilizing a streaming context API.
 */

/**
 * @struct Sha256Context
 * @brief Represents the internal state of a SHA-256 computation.
 */
typedef struct {
    uint32 state[8];
    uint32 dataLength[2];
    Byte buffer[64];
    uint32 bufferLength;
} Sha256Context;

/**
 * @brief Initializes the SHA-256 context to its initial state.
 * @param ctx Pointer to the context structure to initialize.
 * @return 1 if initialization was successful, 0 otherwise.
 */
int sha256_init(Sha256Context* ctx);

/**
 * @brief Appends new data to the SHA-256 computation.
 * @param ctx Pointer to the active context structure.
 * @param data Pointer to the input data stream.
 * @param len Size of the input data stream in bytes.
 * @return 1 if the update was successful, 0 otherwise.
 */
int sha256_update(Sha256Context* ctx, const Byte* data, SizeT len);

/**
 * @brief Finalizes the SHA-256 computation and produces the digest.
 * @param ctx Pointer to the active context structure.
 * @param digest Pointer to the buffer where the 32-byte digest will be written.
 * @return 1 if the finalization was successful, 0 otherwise.
 */
int sha256_final(Sha256Context* ctx, Byte* digest);

/**
 * @brief Computes the SHA-256 hash for a given data buffer in a single operation.
 * @param data Pointer to the input data.
 * @param len Size of the input data in bytes.
 * @param digest Pointer to the buffer where the 32-byte digest will be written.
 * @return 1 if the computation was successful, 0 otherwise.
 */
bool computeSha256(const Byte* data, SizeT len, Byte* digest);

#endif