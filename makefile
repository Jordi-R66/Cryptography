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
CFLAGS_PROD = $(CFLAGS_COMMON) -O3 -funroll-loops -fomit-frame-pointer
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g -fsanitize=address

# Éditeur de liens
LDFLAGS = -flto
LDLIBS = -lpthread

# Application des flags selon le mode choisi
ifeq ($(MODE), debug)
	CFLAGS = $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_PROD)
endif

# ==========================================
# Fichiers sources et objets communs (libs)
# ==========================================
# Utilisation de $(MOCL_DIR) pour pointer vers la bonne instance de la librairie
COMMON_SRCS = $(MOCL_DIR)/variableSizeInt/customInteger.c \
			  $(MOCL_DIR)/strings/customStrings.c \
			  $(MOCL_DIR)/memory/memfuncs.c \
			  $(MOCL_DIR)/endianness/endianness.c \
			  src/asymmetric/common/utils.c

# ==========================================
# Algorithmes (Cibles modulaires)
# ==========================================
CHACHA20_SRC = src/symmetric/chacha20/chacha20.c
ELGAMAL_SRC = src/asymmetric/elgamal/elgamal.c

# Variables objets déduites
COMMON_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))
CHACHA20_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CHACHA20_SRC))
ELGAMAL_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(ELGAMAL_SRC))

# ==========================================
# Règles principales
# ==========================================
.PHONY: all clean chacha20 elgamal test_main test_crible

# Règle par défaut
all: test_main test_crible

# --- Compilation des algorithmes indépendants ---
chacha20: $(CHACHA20_OBJ) $(COMMON_OBJS)
elgamal: $(ELGAMAL_OBJ) $(COMMON_OBJS)

# Règle générique
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
	@mkdir -p asm/
	$(CC) $(CFLAGS) -S $< -o asm/$(notdir $(<:.c=.s))

# ==========================================
# Exécutables de tests (main.c et crible.c)
# ==========================================
test_main: src/main.o $(COMMON_OBJS) $(CHACHA20_OBJ) $(ELGAMAL_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

test_crible: src/crible.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.out $^ $(LDLIBS)

# ==========================================
# Nettoyage
# ==========================================
clean:
	rm -rf $(OBJ_DIR)/ asm/
	rm -f *.out
