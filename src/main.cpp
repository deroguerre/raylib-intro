#include "raylib.h"
#include <chrono>
#include <cmath>

int level = 0;  // 0 = décompte, 1-5 = jeu, 6 = fin
int temps_decompte = 60;    // 60 frames à 60 FPS = 1 seconde pour le décompte
int duree_allumage_led = 45;    // durée d'allumage de la LED en frames (45 frames à 60 FPS = 0.75 seconde initialement)
int chrono_led = duree_allumage_led;    // chronomètre pour gérer l'allumage des LEDs
int number = 3;    // numéro à afficher pour le décompte
int score = 0;   // score du joueur
int rayon = 60;    // rayon initial des LEDs
int target_number = 0;  // numéro de la LED qui doit s'allumer
int nb_leds = 3;    // nombre initial de LEDs (niveau 1)
int total_time = 0;    // temps total écoulé en frames pour gérer les changements de niveau
int manche = 1;    // numéro de la manche actuelle

struct Led {
    Vector2 position;   // coordonnées de la LED
    int rayon;    //rayon de la LED
    Color   color;    // couleur de la LED
    bool on;    // état de la LED (allumée ou éteinte)
};

Led table_leds[11];    // tableau de LEDs, on en utilisera que les 3 premières au début, puis on en ajoutera progressivement jusqu'à 11 au niveau 5

void allumerLed(Led& led) {    // fonction pour allumer une LED
    led.on = true;
    led.color = RED;
}

void dessinerLed(const Led& led)    // fonction pour dessiner une LED
{

    if (led.on) {
        if (level != 1) {
            DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % (level - 1) * 5 - (level - 1) * 5 / 2}, rayon, RED); // dessiner la LED avec une position légèrement aléatoire pour rendre le jeu plus difficile à partir du niveau 2
        } else {
            DrawCircleV(led.position, rayon, RED);
        }
        
    } else {
        if (level != 1) {
            DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, rayon, DARKGRAY); // même chose pour les LEDs éteintes
        } else {
            DrawCircleV(led.position, rayon, DARKGRAY);
        }
    }
}

void decompte() {   // fonction pour gérer le décompte avant le début du jeu
    temps_decompte --;
    if (temps_decompte == 0) {
        number --;
        temps_decompte = 60;
    }
    if (number == 0) {
        for(int i = 0; i < manche; i++) {
            level++;
        }
        manche++;
        return;
    }
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        DrawText(TextFormat("%d", number), 350, 250, 200, BLACK);
        DrawText(TextFormat("Level %d", manche), 350, 500, 20, BLACK);  // affichage du niveau
    EndDrawing();
}

void game() {   // fonction pour gérer le jeu en lui-même
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        total_time++;
        for (int i = 0; i < nb_leds; i++) {
            dessinerLed(table_leds[i]);    // dessiner les LEDs en fonction de leur état (allumée ou éteinte)
        }
        chrono_led --;  // décrémenter le chronomètre pour gérer l'allumage des LEDs
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // vérifier si le joueur a cliqué
            Vector2 mousePos = GetMousePosition();
            for (int i = 0; i < nb_leds; i++) {
                if (table_leds[i].on && CheckCollisionPointCircle(mousePos, table_leds[i].position, rayon - level * 5)) {   // vérifier si le clic est sur une LED allumée 
                    score++;
                    table_leds[i].on = false;
                    chrono_led = 0;
                } else if (!table_leds[i].on && CheckCollisionPointCircle(mousePos, table_leds[i].position, rayon - level * 5)) {
                    score--;    // pénalité si le joueur clique sur une LED éteinte
                    break;  // éviter de pénaliser plusieurs fois pour un même clic
                }
            }
        }
        if (chrono_led == 0) {  // si le chronomètre pour l'allumage des LEDs est à 0, allumer une nouvelle LED
            for (int i = 0; i < nb_leds; i++) {
                table_leds[i].on = false;   // éteindre toutes les LEDs avant d'en allumer une nouvelle
            }
            int x = target_number;  // stocker le numéro de la LED qui doit s'allumer pour éviter de tomber sur la même LED que précédemment
            while (x == target_number) {
                target_number = rand() % nb_leds;   // choisir une LED aléatoire parmi celles disponibles
            }
            table_leds[target_number].on = true;
            chrono_led = duree_allumage_led;    // réinitialiser le chronomètre pour l'allumage des LEDs

        }

        DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);    // afficher le score en haut à gauche
        if (total_time >= 600) {    // après 10 secondes de jeu, augmenter le niveau et réinitialiser les paramètres pour rendre le jeu plus difficile
            if (level != 5) {
                level = 0;
            } else {
                level = 6;
            }
            total_time = 0;
            number = 3;
            duree_allumage_led -= 3;
            nb_leds += 2;
            for (int i = 0; i < nb_leds; i++) {
                table_leds[i].on = false;
            }
        }
    EndDrawing();
}

// ============================================================
//  Point d'entrée du programme
// ============================================================


int main()
{
    srand(time(nullptr));   // initialiser la graine pour les nombres aléatoires en fonction du temps actuel pour que les positions des LEDs soient différentes à chaque exécution du jeu
    const int LARGEUR  = 800;
    const int HAUTEUR  = 600;
    const int FPS_CIBLE = 60;

    InitWindow(LARGEUR, HAUTEUR, "Catch the light");
    SetTargetFPS(FPS_CIBLE);    

    // initialisation des LEDs avec leurs positions, leur rayon, leur couleur et leur état (allumée ou éteinte)

    table_leds[0] = {{400, 300}, rayon, DARKGRAY, false};
    table_leds[1] = {{620, 300}, rayon, DARKGRAY, false};
    table_leds[2] = {{180, 300}, rayon, DARKGRAY, false};
    table_leds[3] = {{332, 91}, rayon, DARKGRAY, false};
    table_leds[4] = {{468, 509}, rayon, DARKGRAY, false};
    table_leds[5] = {{222, 171}, rayon, DARKGRAY, false};
    table_leds[6] = {{578, 429}, rayon, DARKGRAY, false};
    table_leds[7] = {{222, 429}, rayon, DARKGRAY, false};
    table_leds[8] = {{578, 171}, rayon, DARKGRAY, false};
    table_leds[9] = {{332, 509}, rayon, DARKGRAY, false};
    table_leds[10] = {{468, 91}, rayon, DARKGRAY, false};

    while (!WindowShouldClose())
    {
        if (level < 6) {    // tant que le niveau n'est pas à 6 (fin du jeu), continuer à afficher le décompte ou le jeu en fonction du niveau actuel
            if (level == 0) {
                decompte();
            } else {
                game();
            }
        } else {    // niveau 6 = fin du jeu, afficher un message de fin et le score final
            BeginDrawing();
                ClearBackground(LIGHTGRAY);
                DrawText("Nice try!", 280, 250, 50, BLACK);
                DrawText(TextFormat("Score: %d", score), 320, 350, 20, BLACK);
            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
