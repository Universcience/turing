# `turing`

Implémentation ASCII des machines de Turing, développée dans le cadre de l'exposé **Le pouvoir des algorithmes** du Palais de la Découverte.

## Description

Les [machines de Turing(en)](https://en.wikipedia.org/wiki/Turing_machine) sont un modèle abstrait proposé par Alan Turing permettant de décrire n'importe quelle procédure de calcul. Elles sont unanimement considérées comme un modèle universel du calcul en vertu de la [thèse de Church-Turing(en)](https://en.wikipedia.org/wiki/Church%E2%80%93Turing_thesis), c'est-à-dire que n'importe quelle procédure de calcul imaginable correspond à une machine de Turing capable d'effectuer ce calcul.

Les [castors affairés(en)](https://en.wikipedia.org/wiki/Busy_beaver) sont un sous-ensemble précis des machines de Turing, englobant exclusivement les machines qui ont une activité opérationnelle maximale (à nombre d'instructions égal) selon un certain critère : par exemple, la machine de Turing qui effectue le calcul le plus long avant de cesser de fonctionner.

Ce programme permet de visualiser le fonctionnement d'une machine de Turing jusqu'au terme de son calcul (dans les limites des capacités de l'ordinateur sur lequel il s'exécute...) ; il permet de mettre en pause, d'accélérer, de ralentir, ou de revenir en arrière tout au long du processus de calcul, et d'examiner les instructions de la machine et le contenu de sa bande (c'est-à-dire sa mémoire de travail interne).

Le répertoire `assets` contient la description de plusieurs machines de type castor affairé, fonctionnant sur un alphabet binaire (les symboles `_` et `#` par défaut). Les fichiers `bb3.tm` et `bb4.tm` décrivent les castors affairés qui ont écrit le plus grand nombre de `#` dans leur mémoire interne avant de cesser de fonctionner, en se limitant à 3 et 4 états internes respectivement ; les fichiers `bb5c.tm` et `bb6c.tm` sont des candidats au titre de castor affairé ayant respectivement 5 et 6 états internes... la question de savoir s'il existe une machine plus affairée à nombre d'état égal étant encore non-résolue à l'heure actuelle.

# Déploiement

Le programme fourni est conçu pour un système GNU/Linux ([Debian(fr)](https://www.debian.org/index.fr.html)), mais peut être porté sur tout système supportant la bibliothèque `ncurses`.

Le fichier `Makefile` fourni permet de compiler le programme C. Le répertoire `assets/` contient les descriptions de plusieurs machines de Turing lisibles par le programme.

## Paquetages nécessaires (Debian Stretch)
- `gcc`
- `make`
- `libncurses5-dev`

## Usage
```
git clone https://github.com/universcience/turing
cd turing
make
./turing assets/bb4.tm
```

Le programme démarre en simulant l'exécution de la machine décrite dans le fichier passé en paramètre (ou par défaut, `assets/bb5c.tm`). Ces fichiers `.tm`, propres au logiciel, sont des fichiers au format ASCII dont la syntaxe est décrite à la fin du fichier `turing.h`.

Aussitôt démarré, le programe simule le fonctionnement de la machine de Turing fournie, au rythme de 16 instructions par seconde. Les instructions de la machine sont affichées dans un cadre en haut de l'écran, et son état interne est marqué par une astérisque (`*`). Chaque colonne correspond à un état de la machine, et chaque ligne correspond à symbole présent sous la tête de lecture (représentée par un `V`). Une instruction se compose d'un symbole à écrire sous la tête, d'une direction de déplacement (`<` ou `>`), et du nouvel état interne de la machine (donné par un chiffre, ou `H` pour *Halt*, signifiant la fin du calcul).

Les touches fléchées permettent d'ajuster la vitesse (haut pour doubler la vitesse, bas pour la diviser par deux) et la direction du calcul (gauche pour revenir en arrière, droite pour aller de l'avant). La barre d'espace (ou la touche `p` pour *Pause*) permet de mettre en pause le calcul (indiqué par un marqueur `[PAUSE]`). Les touches `n` (*Next*) et `b` (*Back*) permettent de dérouler le calcul pas à pas, vers l'avant ou l'arrière.

Les touches `<` et `>` permettent de faire défiler la bande d'une case à la fois, pour inspecter son contenu. La touche `c` (*Center*) permet de centrer la vue sur la tête de lecture/écriture, et les touches voisines `w` et `x` (*marks the spot*) permettent respectivement de suivre la tête de lecture automatiquement à chaque fois qu'elle sort de l'écran (mode `[FOLLOW]`), ou de verrouiller le centrage de l'écran sur la tête en permanence (mode `[LOCK]`).

Enfin, la touche `q` (*Quit*) permet de quitter le programme à tout moment.

# Licence

Ceci est un [logiciel libre(fr)](https://www.gnu.org/philosophy/free-sw.fr.html), distribué sous les termes de la licence GNU GPL Affero. Les termes exacts de cette licence sont disponibles dans le fichier `LICENSE`.

En résumé, vous êtes libres de télécharger ce logiciel, de l'utiliser et de le modifier sans restrictions, ainsi que de le redistribuer sous les mêmes termes, avec ou sans modifications. 
