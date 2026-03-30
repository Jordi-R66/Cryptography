# ==========================================
# Configuration du compilateur et des flags
# ==========================================
CC = gcc

# Flags de compilation (-I pour les dossiers d'inclusion)
CFLAGS = -std=c17 -Wall -Wextra -O3 \
		-march=native -mtune=native -funroll-loops -fomit-frame-pointer \
		-Isrc/common_headers -Ilibs/myOwnCLib

# Flags de l'éditeur de liens (Linker)
LDFLAGS = -flto

# Librairies externes (pthread pour ton utils.c)
LDLIBS = -lpthread

# ==========================================
# Fichiers sources
# ==========================================
APP_SRCS = src/main.c src/utils.c

EL_GAMAL_SRCS = src/elgamal/elgamal.c

LIB_SRCS = libs/myOwnCLib/variableSizeInt/customInteger.c \
			libs/myOwnCLib/strings/customStrings.c \
			libs/myOwnCLib/memory/memfuncs.c \
			libs/myOwnCLib/endianness/endianness.c

# Concaténation de toutes les sources
SRCS = $(APP_SRCS) $(EL_GAMAL_SRCS) $(LIB_SRCS)

# Transformation de la liste des .c en .o (fichiers objets)
OBJS = $(SRCS:.c=.o)

# Nom de l'exécutable final
TARGET = crypto_app.out

# ==========================================
# Règles de compilation
# ==========================================
.PHONY: all clean

# Règle par défaut
all: $(TARGET)

# Édition de liens (création de l'exécutable)
# Ajouter les paramètres -g -fsanitize=address pour débug la mémoire en cas de fuite, ou détecter des risques de fuite
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	$(CC) -S

# Compilation des fichiers .c en fichiers .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJS) $(TARGET)