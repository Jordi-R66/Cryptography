# ==========================================
# Configuration du compilateur et chemins
# ==========================================
CC = gcc
OBJ_DIR = build

# Chemin vers myOwnCLib. 
# Surchargeable pour éviter la redondance si utilisé comme sous-module.
# Exemple : make MOCL_DIR=../autre_dossier/myOwnCLib
MOCL_DIR ?= libs/myOwnCLib

# ==========================================
# Gestion des modes (Prod / Debug)
# Utilisation : "make" (prod par défaut) ou "make MODE=debug"
# ==========================================
MODE ?= prod

# Dossiers d'inclusion (mise à jour avec la variable de chemin)
INCLUDES = -Isrc/common -I$(MOCL_DIR)

# Flags communs
CFLAGS_COMMON = -std=c17 -Wall -Wextra -masm=intel -march=native -mtune=native $(INCLUDES)

# Flags spécifiques
CFLAGS_PROD = $(CFLAGS_COMMON) -O3 -funroll-loops -fomit-frame-pointer -flto
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g -fsanitize=address

# Éditeur de liens
LDLIBS = -lpthread

# Application des flags selon le mode choisi
ifeq ($(MODE), debug)
    CFLAGS = $(CFLAGS_DEBUG)
    LDFLAGS = 
else
    CFLAGS = $(CFLAGS_PROD)
    LDFLAGS = -flto
endif

# ==========================================
# Fichiers sources et objets communs (libs)
# ==========================================
# Utilisation de $(MOCL_DIR) pour pointer vers la bonne instance de la librairie
COMMON_SRCS = $(MOCL_DIR)/variableSizeInt/customInteger.c \
              $(MOCL_DIR)/strings/customStrings.c \
              $(MOCL_DIR)/memory/memfuncs.c \
              $(MOCL_DIR)/endianness/endianness.c \
              src/cipher/asymmetric/common/utils.c

# ==========================================
# Algorithmes (Cibles modulaires)
# ==========================================
CHACHA20_SRC = src/cipher/symmetric/chacha20/chacha20.c
POLY1305_SRC = src/cipher/symmetric/poly1305/poly1305.c
CC20P1305_SRC = src/cipher/symmetric/chacha20poly1305/chacha20poly1305.c
ELGAMAL_SRC = src/cipher/asymmetric/elgamal/elgamal.c

SHA256_SRC = src/hash/sha256.c

# Variables objets déduites
COMMON_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))
CHACHA20_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CHACHA20_SRC))
POLY1305_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(POLY1305_SRC))
CC20P1305_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CC20P1305_SRC))
ELGAMAL_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(ELGAMAL_SRC))

SHA256_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SHA256_SRC))


APPS_SRCS = src/apps/file_cipher/main.c
APPS_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(APPS_SRCS))

# ==========================================
# Règles principales
# ==========================================
.PHONY: all clean poly1305 cc20p1305 chacha20 elgamal test_main test_crible app_file_cipher

# Règle par défaut
all: test_main test_crible

# --- Compilation des algorithmes indépendants ---
chacha20: $(CHACHA20_OBJ) $(COMMON_OBJS)
elgamal: $(ELGAMAL_OBJ) $(COMMON_OBJS) $(SHA256_OBJ)
cc20p1305: $(CC20P1305_OBJ) $(COMMON_OBJS)
poly1305: $(POLY1305_OBJ) $(COMMON_OBJS)

# Règle générique
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@mkdir -p asm/
	$(CC) $(CFLAGS) -S $< -o asm/$(notdir $(<:.c=.s))

# ==========================================
# Exécutables de tests (main.c et crible.c)
# ==========================================
# CORRECTION CRITIQUE : Ajout de $(POLY1305_OBJ) et $(CC20P1305_OBJ) pour le linker
test_main: $(OBJ_DIR)/src/main.o $(COMMON_OBJS) $(CHACHA20_OBJ) $(POLY1305_OBJ) $(CC20P1305_OBJ) $(ELGAMAL_OBJ) $(SHA256_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

test_crible: $(OBJ_DIR)/src/crible.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

app_file_cipher: $(APPS_OBJS) $(COMMON_OBJS) $(CHACHA20_OBJ) $(POLY1305_OBJ) $(CC20P1305_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

# ==========================================
# Nettoyage
# ==========================================
clean:
	rm -rf $(OBJ_DIR)/ asm/
	rm -f *.out