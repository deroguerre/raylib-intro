#include "raylib.h"
#include <chrono>
#include <cmath>

int level = 0;
int temps_decompte = 60;
int duree_allumage_led = 45;
int chrono_led = duree_allumage_led;
int number = 3;
int score = 0;
int rayon = 60;
int target_number = 0;
int nb_leds = 3;
int total_time = 0;
int manche = 1;

struct Led {
    Vector2 position;   // {x, y}
    int rayon;    // pixels par seconde
    Color   color;
    bool on;
};
Led table_leds[11];
// ============================================================
//  Structures
//  En C++, on peut regrouper des données liées dans une struct.
// ============================================================


// ============================================================
//  Fonctions
//  On sépare la logique en petites fonctions réutilisables.
// ============================================================

// Allume la LED
void allumerLed(Led& led) {
    led.on = true;
    led.color = RED;
}

void dessinerLed(const Led& led)
{

    if (led.on) {
        if (level != 1) {
            DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % (level - 1) * 5 - (level - 1) * 5 / 2}, rayon, RED);
        } else {
            DrawCircleV(led.position, rayon, RED);
        }
        
    } else {
        if (level != 1) {
            DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, rayon, DARKGRAY);
        } else {
            DrawCircleV(led.position, rayon, DARKGRAY);
        }
    }
}

void decompte() {
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
        DrawText(TextFormat("Level %d", manche), 350, 500, 20, BLACK);
    EndDrawing();
}

void game() {
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        total_time++;
        for (int i = 0; i < nb_leds; i++) {
            dessinerLed(table_leds[i]);
        }
        chrono_led --;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            for (int i = 0; i < nb_leds; i++) {
                if (table_leds[i].on && CheckCollisionPointCircle(mousePos, table_leds[i].position, rayon - level * 5)) {
                    score++;
                    table_leds[i].on = false;
                    chrono_led = 0;
                } else if (!table_leds[i].on && CheckCollisionPointCircle(mousePos, table_leds[i].position, rayon - level * 5)) {
                    score--;
                }
            }
        }
        if (chrono_led == 0) {
            for (int i = 0; i < nb_leds; i++) {
                table_leds[i].on = false;
            }
            int x = target_number;
            while (x == target_number) {
                target_number = rand() % nb_leds;
            }
            table_leds[target_number].on = true;
            chrono_led = duree_allumage_led;

        }

        DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
        if (total_time >= 600) {
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
    srand(time(nullptr));
    const int LARGEUR  = 800;
    const int HAUTEUR  = 600;
    const int FPS_CIBLE = 60;

    InitWindow(LARGEUR, HAUTEUR, "Catch the light");
    SetTargetFPS(FPS_CIBLE);

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
        if (level < 6) {
            if (level == 0) {
                decompte();
            } else {
                game();
            }
        } else {
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
