project('Cryptography', 'c',
	version: '0.1',
	default_options: [
		'c_std=c11', 
		'warning_level=3',
		'buildtype=release',	# Applique -O3 en tâche de fond
		'b_lto=true'			# Active le Link-Time Optimization (très puissant pour lier la lib et le main)
	]
)

cc = meson.get_compiler('c')

# ==========================================
# Optimisations agressives spécifiques à GCC
# ==========================================
optim_flags = [
	'-march=native',		# Génère des instructions exclusives à votre processeur actuel
	'-mtune=native',		# Ordonnance le code pour optimiser les pipelines de ce même processeur
	'-funroll-loops',		# Déroule les boucles (augmente la taille du binaire, mais accélère l'exécution)
	'-fomit-frame-pointer'	# Libère un registre supplémentaire
]

# On s'assure que le compilateur accepte chaque flag avant de l'ajouter
foreach arg : optim_flags
	if cc.has_argument(arg)
		add_project_arguments(arg, language: 'c')
	endif
endforeach

# ==========================================
# Inclusion des sous-projets et sources
# ==========================================

# 1. Chargement des librairies internes (déclenche libs/meson.build)
subdir('libs')

# 2. Définition des sources de l'application
app_sources = files(
	'src/main.c'
)

app_inc = include_directories('src/headers')

# 3. Compilation de l'exécutable
executable('crypto_app',
	app_sources,
	include_directories: app_inc,
	dependencies: my_own_c_lib_dep, # Liaison avec la lib compilée dans subdir('libs')
	link_args: [
		'-s',
		'--static'
	],
	install: true
)