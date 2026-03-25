# ==========================================
# 1. Bibliothèque : myOwnCLib
# ==========================================

my_own_c_lib_sources = files(
	'myOwnCLib/variableSizeInt/customInteger.c',
	'myOwnCLib/strings/customStrings.c',
	'myOwnCLib/memory/memfuncs.c',
	'myOwnCLib/endianness/endianness.c'
)

# Le dossier d'inclusion pointe sur la racine de la lib
my_own_c_lib_inc = include_directories('myOwnCLib')

my_own_c_lib_static = static_library('myOwnCLib',
	my_own_c_lib_sources,
	include_directories: my_own_c_lib_inc
)

# Variable globale exposée au projet principal
my_own_c_lib_dep = declare_dependency(
	link_with: my_own_c_lib_static,
	include_directories: my_own_c_lib_inc
)

# ==========================================
# 2. Future Bibliothèque (Exemple pour plus tard)
# ==========================================
# future_lib_sources = files('futureLib/src1.c', ...)
# future_lib_inc = include_directories('futureLib')
# future_lib_static = static_library('futureLib', future_lib_sources, include_directories: future_lib_inc)
# future_lib_dep = declare_dependency(link_with: future_lib_static, include_directories: future_lib_inc)