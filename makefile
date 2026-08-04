CC = gcc
OBJ_DIR = build
MOCL_DIR ?= libs/myOwnCLib
MODE ?= prod

INCLUDES = -Isrc/common -I$(MOCL_DIR)
CFLAGS_COMMON = -std=c17 -Wall -Wextra -masm=intel -march=native -mtune=native $(INCLUDES)
CFLAGS_PROD = $(CFLAGS_COMMON) -O3 -funroll-loops -fomit-frame-pointer -flto
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g -fsanitize=address
LDLIBS = -lpthread

ifeq ($(MODE), debug)
    CFLAGS = $(CFLAGS_DEBUG)
    LDFLAGS = 
    AR = ar
else
    CFLAGS = $(CFLAGS_PROD)
    LDFLAGS = -flto
    AR = gcc-ar
endif

COMMON_SRCS = $(MOCL_DIR)/variableSizeInt/customInteger.c \
              $(MOCL_DIR)/strings/customStrings.c \
              $(MOCL_DIR)/memory/memfuncs.c \
              $(MOCL_DIR)/endianness/endianness.c \
              src/cipher/asymmetric/common/utils.c

CHACHA20_SRC = src/cipher/symmetric/chacha20/chacha20.c
POLY1305_SRC = src/cipher/symmetric/poly1305/poly1305.c
CC20P1305_SRC = src/cipher/symmetric/chacha20poly1305/chacha20poly1305.c
ELGAMAL_SRC = src/cipher/asymmetric/elgamal/elgamal.c
SHA256_SRC = src/hash/sha256.c

COMMON_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))
CHACHA20_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CHACHA20_SRC))
POLY1305_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(POLY1305_SRC))
CC20P1305_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CC20P1305_SRC))
ELGAMAL_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(ELGAMAL_SRC))
SHA256_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SHA256_SRC))

APP_FILE_CIPHER_SRCS = src/apps/file_cipher/main.c
APP_FILE_CIPHER_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(APP_FILE_CIPHER_SRCS))

APP_HASH_FILE_SRCS = src/apps/hash_file/main.c
APP_HASH_FILE_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(APP_HASH_FILE_SRCS))

LIB_OBJS = $(COMMON_OBJS) $(CHACHA20_OBJ) $(POLY1305_OBJ) $(CC20P1305_OBJ) $(ELGAMAL_OBJ) $(SHA256_OBJ)
TARGET_LIB = $(OBJ_DIR)/libcryptography.a

.PHONY: all clean lib poly1305 cc20p1305 chacha20 elgamal test_main test_crible app_file_cipher app_hash_file

all: lib test_main test_crible

lib: $(TARGET_LIB)

$(TARGET_LIB): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

chacha20: $(CHACHA20_OBJ) $(COMMON_OBJS)
elgamal: $(ELGAMAL_OBJ) $(COMMON_OBJS) $(SHA256_OBJ)
cc20p1305: $(CC20P1305_OBJ) $(COMMON_OBJS)
poly1305: $(POLY1305_OBJ) $(COMMON_OBJS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@mkdir -p asm/
	$(CC) $(CFLAGS) -S $< -o asm/$(notdir $(<:.c=.s))

test_main: $(OBJ_DIR)/src/main.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

test_crible: $(OBJ_DIR)/src/crible.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

app_file_cipher: $(APP_FILE_CIPHER_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

app_hash_file: $(APP_HASH_FILE_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

clean:
	rm -rf $(OBJ_DIR)/ asm/
	rm -f *.out