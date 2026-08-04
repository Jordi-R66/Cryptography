#include <stdio.h>
#include "../../hash/sha256.h"

/**
 * @file hash_file.c
 * @brief Utility to compute the SHA-256 hash of a given file.
 */

/**
  * @brief Computes the SHA-256 hash of a file by reading it in chunks.
  * @param filepath Path to the target file.
  * @param digest Pointer to the buffer where the 32-byte digest will be written.
  * @return 1 if the computation was successful, 0 otherwise.
  */
int computeSha256File(const char* filepath, Byte* digest) {
	int status = 0;
	int isValidParams = (filepath != NULL) && (digest != NULL);
	FILE* file = NULL;
	Sha256Context ctx;
	Byte buffer[8192];
	SizeT bytesRead = 0;
	bool isInitSuccess = false, isReading = false, isProcessSuccess = false, isFinalSuccess = false, isEOF = false;

	if (isValidParams) {
		file = fopen(filepath, "rb");
	}

	if (file != NULL) {
		isInitSuccess = sha256_init(&ctx);
	}

	if (isInitSuccess) {
		isReading = 1;
		isProcessSuccess = 1;

		while (isReading) {
			bytesRead = fread(buffer, 1, sizeof(buffer), file);

			if (bytesRead > 0) {
				isProcessSuccess = isProcessSuccess & sha256_update(&ctx, buffer, bytesRead);
			}

			isReading = (bytesRead > 0) & isProcessSuccess;
		}

		isEOF = (feof(file) != 0);
		isProcessSuccess = isProcessSuccess & isEOF;
	}

	if (isProcessSuccess) {
		isFinalSuccess = sha256_final(&ctx, digest);
	}

	if (file != NULL) {
		fclose(file);
	}

	if (isFinalSuccess) {
		status = 1;
	}

	return status;
}

/**
 * @brief Main entry point of the program.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char** argv) {
	int exitCode = 1;
	int hasValidArgs = (argc == 2);
	int isComputeSuccess = 0;
	int isPrintNeeded = 0;
	Byte digest[32];
	int i = 0;

	if (hasValidArgs) {
		isComputeSuccess = computeSha256File(argv[1], digest);
	}

	isPrintNeeded = isComputeSuccess;

	if (isPrintNeeded) {
		for (i = 0; i < 32; ++i) {
			printf("%02x", digest[i]);
		}
		printf("  %s\n", argv[1]);
		exitCode = 0;
	}

	if (hasValidArgs & !isComputeSuccess) {
		printf("Error: Failed to compute SHA-256 for file '%s'.\n", argv[1]);
	}

	if (!hasValidArgs) {
		printf("Usage: %s <filepath>\n", argv[0]);
	}

	return exitCode;
}