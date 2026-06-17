#include "raylib.h"

// ============================================================
//  Structures
//  En C++, on peut regrouper des données liées dans une struct.
// ============================================================

struct Balle {
    Vector2 position;   // {x, y}
    Vector2 vitesse;    // pixels par seconde
    float   rayon;
    Color   couleur;
};

// ============================================================
//  Fonctions
//  On sépare la logique en petites fonctions réutilisables.
// ============================================================

// Met à jour la position et fait rebondir sur les bords
void mettreAJourBalle(Balle& balle, float deltaTemps)
{
    balle.position.x += balle.vitesse.x * deltaTemps;
    balle.position.y += balle.vitesse.y * deltaTemps;

    int largeur = GetScreenWidth();
    int hauteur = GetScreenHeight();

    if (balle.position.x - balle.rayon < 0 || balle.position.x + balle.rayon > largeur)
        balle.vitesse.x *= -1.0f;

    if (balle.position.y - balle.rayon < 0 || balle.position.y + balle.rayon > hauteur)
        balle.vitesse.y *= -1.0f;
}

void dessinerBalle(const Balle& balle)
{
    DrawCircleV(balle.position, balle.rayon, balle.couleur);
}

// ============================================================
//  Point d'entrée du programme
// ============================================================

int main()
{
    const int LARGEUR  = 800;
    const int HAUTEUR  = 600;
    const int FPS_CIBLE = 60;

    InitWindow(LARGEUR, HAUTEUR, "Intro Raylib - Balle rebondissante");
    SetTargetFPS(FPS_CIBLE);

    Balle balle = {
        .position = { LARGEUR / 2.0f, HAUTEUR / 2.0f },
        .vitesse  = { 300.0f, 220.0f },
        .rayon    = 20.0f,
        .couleur  = RED
    };

    // --------------------------------------------------------
    //  Boucle principale : tourne jusqu'à ce qu'on ferme la fenêtre
    // --------------------------------------------------------
    while (!WindowShouldClose())
    {
        // --- Mise à jour ---
        float dt = GetFrameTime();   // secondes depuis la dernière frame

        mettreAJourBalle(balle, dt);

        // Clic gauche : téléporte la balle sous le curseur
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            balle.position = GetMousePosition();

        // Espace : change la couleur aléatoirement
        if (IsKeyPressed(KEY_SPACE))
            balle.couleur = { (unsigned char)GetRandomValue(50, 255),
                              (unsigned char)GetRandomValue(50, 255),
                              (unsigned char)GetRandomValue(50, 255),
                              255 };

        // --- Dessin ---
        BeginDrawing();
            ClearBackground(DARKGRAY);

            dessinerBalle(balle);

            DrawText("Clic gauche : déplacer la balle", 10, 10, 18, LIGHTGRAY);
            DrawText("Espace      : changer la couleur", 10, 32, 18, LIGHTGRAY);
            DrawText("Echap       : quitter",            10, 54, 18, LIGHTGRAY);

            DrawFPS(LARGEUR - 80, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
