#include "raylib.h"
#include <chrono>
#include <cmath>

int level = -1;  // -1 = menu principal, 0 = décompte, 1-5 = jeu, 6 = fin
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

bool bonus = false;    // variable pour gérer l'apparition du bonus (LED dorée) dans le jeu
bool malus = false;    // variable pour gérer l'apparition du malus (LED noire) dans le jeu

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
            if (bonus) {    // si le bonus est actif et que la LED à dessiner est celle qui doit s'allumer, la dessiner en doré
                DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, rayon, GOLD);
            } else {
                DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, rayon, RED); // dessiner la LED avec une position légèrement aléatoire pour rendre le jeu plus difficile à partir du niveau 2
            }
        } else {
            DrawCircleV(led.position, rayon, RED);
        }
        
    } else {

        if (level >= 3) {
            if (malus) {    // si le malus est actif et que la LED à dessiner est celle qui doit s'allumer, la dessiner en noir
                DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, rayon, BLACK);
            } else {
                DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, rayon, DARKGRAY); 
            }
        } else if (level == 2) {
            DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, rayon, DARKGRAY); 
        } else {
            DrawCircleV(led.position, rayon, DARKGRAY);
        }
    }
}

void menu() {   // fonction pour gérer le menu principal
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        DrawText("Catch the light", 250, 100, 50, BLACK);
        DrawText("Press SPACE to start", 250, 300, 20, BLACK);
        DrawText("Click on the red LED to score points!", 200, 400, 20, BLACK);
        DrawText("Click on the dark LED to lose points!", 200, 450, 20, BLACK);
        if (IsKeyPressed(KEY_SPACE)) {    // si le joueur appuie sur la touche espace, passer au décompte
            level = 0;
        }
    EndDrawing();
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

        if (total_time == 300 && level > 1) {    // après 5 secondes de jeu, faire apparaître le bonus (LED dorée) pour 3 secondes
            bonus = true;
        } else if (total_time == 360) {    // après 6 secondes de jeu, faire disparaître le bonus
            bonus = false;
        }

        if (total_time == 420 && level >= 3) {    // après 7 secondes de jeu, faire apparaître le malus (LED noire) pour 3 secondes
            malus = true;
        } else if (total_time == 480) {    // après 8 secondes de jeu, faire disparaître le malus
            malus = false;
        }

        for (int i = 0; i < nb_leds; i++) {
            dessinerLed(table_leds[i]);    // dessiner les LEDs en fonction de leur état (allumée ou éteinte)
        }
        chrono_led --;  // décrémenter le chronomètre pour gérer l'allumage des LEDs
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // vérifier si le joueur a cliqué
            Vector2 mousePos = GetMousePosition();
            for (int i = 0; i < nb_leds; i++) {
                if (table_leds[i].on && CheckCollisionPointCircle(mousePos, table_leds[i].position, rayon - level * 5)) {   // vérifier si le clic est sur une LED allumée 
                    if (bonus && i == target_number) {    // si le bonus est actif et que le joueur a cliqué sur la LED dorée, lui donner un bonus de points
                        score += 5;
                        bonus = false;    // désactiver le bonus après que le joueur l'ait attrapé
                    } else {
                        score++;    // sinon, incrémenter le score normalement
                    }
                    table_leds[i].on = false;
                    chrono_led = 0;
                } else if (!table_leds[i].on && CheckCollisionPointCircle(mousePos, table_leds[i].position, rayon - level * 5)) {
                    if (malus) {    // si le malus est actif et que le joueur a cliqué sur la LED noire, lui donner un malus de points
                        score -= 5;
                        malus = false;    // désactiver le malus après que le joueur l'ait attrapé
                    } else {
                        score--;    // pénalité si le joueur clique sur une LED éteinte
                    }
                    break;  // éviter de pénaliser plusieurs fois pour un même clic
                }
            }
        }
        if (chrono_led == 0) {  // si le chronomètre pour l'allumage des LEDs est à 0, allumer une nouvelle LED
            for (int i = 0; i < nb_leds; i++) {
                table_leds[i].on = false;   // éteindre toutes les LEDs avant d'en allumer une nouvelle
            }
            int x = target_number;  // stock du numéro de la LED qui doit s'allumer pour éviter de tomber sur la même LED que précédemment
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

void end() {
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        if (score < 0) {
            DrawText("You are probably the worst player ever...", 150, 250, 30, BLACK);
        } else {
            switch (score / 15) {   // afficher un message différent en fonction du score final du joueur
                case 0:
                    DrawText("You can do better!", 150, 250, 50, BLACK);
                    break;
                case 1:
                    DrawText("So so...", 150, 250, 50, BLACK);
                    break;
                case 2:
                    DrawText("That's fine, you can do better though!", 150, 250, 30, BLACK);
                    break;
                case 3:
                    DrawText("Not bad!", 150, 250, 50, BLACK);
                    break;
                case 4:
                    DrawText("Great job!", 150, 250, 50, BLACK);
                    break;
                case 5:
                    DrawText("Amazing! You are a pro!", 150, 250, 50, BLACK);
                    break;
                default:
                    DrawText("Unbelievable... are you sure you are not cheating?", 150, 250, 30, BLACK);
                    break;
            }
        }
        DrawText(TextFormat("Score: %d", score), 320, 350, 20, BLACK);
        DrawText("Press ESC to exit", 300, 400, 20, BLACK);
        DrawText("Press R to restart", 300, 450, 20, BLACK);
        if (IsKeyPressed(KEY_R)) {    // si le joueur appuie sur la touche R, réinitialiser le jeu pour recommencer une nouvelle partie
            level = -1;
            temps_decompte = 60;
            duree_allumage_led = 45;
            chrono_led = duree_allumage_led;
            number = 3;
            score = 0;
            rayon = 60;
            target_number = 0;
            nb_leds = 3;
            total_time = 0;
            manche = 1;
            bonus = false;
            malus = false;
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
        switch (level) {    // en fonction du niveau actuel, appeler la fonction correspondante pour gérer l'affichage et la logique du jeu
            case -1:
                menu();
                break;
            case 0:
                decompte();
                break;
            case 6:
                end();
                break;
            default:
                game();
                break;
        }
    }

    CloseWindow();
    return 0;
}
