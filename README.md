# raylib-intro — Mise en place de l'environnement

Durée estimée : **10-15 minutes** (dont ~5 min de téléchargement).

---

## 1. Installer le compilateur C++ et CMake

Ouvre **PowerShell en tant qu'administrateur** et colle cette commande :

```powershell
winget install Microsoft.VisualStudio.2022.BuildTools
```

L'installeur graphique s'ouvre. Coche **"Développement Desktop en C++"** puis clique
sur **Installer**. (~3 Go, inclut le compilateur MSVC, CMake et Ninja.)

> Si tu as déjà Visual Studio 2022 Community ou Professional installé, tu n'as rien
> à faire : les outils sont déjà là.

---

## 2. Installer les extensions VS Code

Ouvre VS Code et installe ces deux extensions (icône Extensions dans la barre latérale) :

| Extension | Identifiant |
|-----------|-------------|
| C/C++ | `ms-vscode.cpptools` |
| CMake Tools | `ms-vscode.cmake-tools` |

---

## 3. Ouvrir le projet

1. **File → Open Folder…** → sélectionne le dossier `raylib-intro`.
2. VS Code détecte automatiquement le `CMakeLists.txt`.
3. Une invite apparaît en bas à droite : **"Sélectionner un kit"** →
   choisis **"Visual Studio Build Tools 2022 - amd64"**.
4. CMake Tools configure le projet tout seul (barre de progression en bas).

> La première configuration télécharge Raylib depuis GitHub (~10 Mo).
> C'est normal, ça ne se produit qu'une seule fois.

---

## 4. Compiler et lancer

| Action | Raccourci |
|--------|-----------|
| Compiler | `F7` |
| Compiler + Lancer | `Shift+F5` |
| Lancer sans recompiler | `Ctrl+F5` |

Le bouton **▶ Lancer** en bas de la fenêtre VS Code fonctionne aussi.

---

## 5. Ce que fait la démo

Une fenêtre 800×600 s'ouvre avec une balle qui rebondit :

- **Clic gauche** → téléporte la balle sous le curseur
- **Espace** → change la couleur aléatoirement
- **Échap** → ferme la fenêtre

Le code source est dans `src/main.cpp`, commenté pour guider la découverte du C++.

---

## Résolution de problèmes courants

**raylib 6.0 exige CMake 3.25+** → si VS Code utilise encore un ancien CMake,
ajoute ce réglage dans `.vscode/settings.json` :
`"cmake.cmakePath": "C:/Program Files/CMake/bin/cmake.exe"`.

**"No CMake kit selected"** → clique sur la barre d'état en bas de VS Code,
là où c'est marqué "No Kit Selected", et sélectionne le kit Build Tools.

**Erreur réseau lors du FetchContent** → vérifie ta connexion ; le proxy d'entreprise
peut bloquer GitHub. Dans ce cas, télécharge Raylib manuellement :
<https://github.com/raysan5/raylib/releases/tag/5.5>
et adapte le `CMakeLists.txt` pour pointer vers le dossier local.

**L'exe se lance mais la fenêtre ne s'affiche pas** → supprime l'option
`-mwindows` dans `CMakeLists.txt` (elle masque la console, utile en prod
mais parfois gênante pour déboguer).
