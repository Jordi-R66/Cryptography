# ==========================================
# Configuration du compilateur et des flags
# ==========================================
CC = gcc

# Flags de compilation (-I pour les dossiers d'inclusion)
CFLAGS = -std=c17 -Wall -Wextra -O3 -flto \
         -march=native -mtune=native -funroll-loops -fomit-frame-pointer \
         -Isrc/common_headers -Ilibs/myOwnCLib

# Flags de l'éditeur de liens (Linker)
LDFLAGS = -s -static -flto

# Librairies externes (pthread pour ton utils.c)
LDLIBS = -lpthread

# ==========================================
# Fichiers sources
# ==========================================
APP_SRCS = src/main.c src/utils.c

LIB_SRCS = libs/myOwnCLib/variableSizeInt/customInteger.c \
           libs/myOwnCLib/strings/customStrings.c \
           libs/myOwnCLib/memory/memfuncs.c \
           libs/myOwnCLib/endianness/endianness.c \
           libs/myOwnCLib/collections/lists/list.c

# Concaténation de toutes les sources
SRCS = $(APP_SRCS) $(LIB_SRCS)

# Transformation de la liste des .c en .o (fichiers objets)
OBJS = $(SRCS:.c=.o)

# Nom de l'exécutable final
TARGET = crypto_app

# ==========================================
# Règles de compilation
# ==========================================
.PHONY: all clean

# Règle par défaut
all: $(TARGET)

# Édition de liens (création de l'exécutable)
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Compilation des fichiers .c en fichiers .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJS) $(TARGET)