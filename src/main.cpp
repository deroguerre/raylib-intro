// Pour la / les fois pro :
// - essayer de changer la couleur de fond lorsqu'il y a clic, bonus, malus...
// (- trouver un moyen d'arrêter la musique quand on entend verre brisé ou malus)
// - réfléchir à des nouveautés/complications plus diverses à partir du niveau 3 (niveau 1 : basique, niveau 2 : bonus + tremblements, niveau 3 : malus, reste ?)

#include "raylib.h"
#include <iostream>
#include "../sqlite/sqlite3.h"
#include <chrono>
#include <cmath>
#include <fstream>
#include <algorithm>

// ==================================================
// Déclaration des variables gérant la logique du jeu
// ==================================================

sqlite3* db;



int classic_best_score = 0;
int arcade_best_score = 0;
bool new_best_score = false;

int global_classic_best_score = 0;
int global_arcade_best_score = 0;
bool new_global_best_score = false;
int level = -2;  // -2 : entrée pseudo, -1 = menu principal, 0 = décompte, 1-5 = jeu, 6 = jeu fin, 7 = arcade, 8 = arcade fin
int precedent_level = 0;    // variable pour stocker le niveau précédent avant la pause
int countdown_time = 60;    // 60 frames à 60 FPS = 1 seconde pour le décompte
int on_led_lasting = 45;    // durée d'allumage de la LED en frames (45 frames à 60 FPS = 0.75 seconde initialement)
int led_chrono = on_led_lasting;    // chronameètre pour gérer l'allumage des LEDs
int number = 3;    // numéro à afficher pour le décompte
int score = 0;   // score du joueur
int radius = 60;    // rayon initial des LEDs
float animated_radius = radius;
int target_number = 0;  // numéro de la LED qui doit s'allumer
int nb_leds = 3;    // namebre initial de LEDs (niveau 1)
int total_time = 0;    // temps total écoulé en frames pour gérer les changements de niveau
int round_tournament = 1;    // numéro de la manche actuelle
float life = 200; // vie (mode arcade)
int timer_bonus = 0;    // chronameètre pour gérer la durée du bonus (LED dorée)
int timer_malus = 0;    // chronameètre pour gérer la durée du malus (LED noire)
int mode = 0;    // variable pour stocker le mode de jeu choisi par le joueur (classique ou arcade)
int chrono_end_arcade = 0;

bool bonus = false;    // variable pour gérer l'apparition du bonus (LED dorée) dans le jeu
bool malus = false;    // variable pour gérer l'apparition du malus (LED noire) dans le jeu

bool can_play_sound = true;
bool can_play_music = true;

bool playing = false;

float x = 0;
float y = 0;
int circle_counter = 0;

std::string player_pseudo = "";
int player_exists = 1;
int player_id = 0;

bool score_saved = false;
int new_player = 0;

// Déclaration des effets sonores et musiques

Sound arcade_lost_sound;
Sound bonus_touched_sound;
Sound game_won_sound;
Sound game_lost_sound;
Sound led_touched_sound;
Sound led_off_touched_sound;
Sound malus_sound;
Sound new_best_score_sound;
Sound pause_sound;
Sound whistle_sound;
Sound end_arcade_sound;

Music ticking_clock;
Music intro_music;
Music bg_music;
Music arcade_bg_music;

// Modélisation d'une LED

struct Led {
    Vector2 position;   // coordonnées de la LED
    int radius;    //radius de la LED
    Color   color;    // couleur de la LED
    bool on;    // état de la LED (allumée ou éteinte)
};

Led leds_table[11];    // déclaration tableau de LEDs


int callbackBestScore(void* data, int argc, char** argv, char**) {
    int* score = (int*)data;
    if (argv[0]) {
        *score = std::stoi(argv[0]);
    }
    return 0;
}



void remadePlaySound(Sound& sound) {    // refonte de la fonction PlaySound() pour empêcher la réitération de son exécution (60 FPS)
    if (can_play_sound) {
        PlaySound(sound);
        can_play_sound = false;
    }
}

void remadePlayMusic(Music& music) {    // refonte de la fonction PlayMusicStream() pour empêcher la réitération de son exécution (60 FPS)
    if (can_play_music) {
        PlayMusicStream(music);
        can_play_music = false;
    }
}

void turnOnLed(Led& led) {    // fonction pour allumer une LED
    led.on = true;
    led.color = RED;
}

void drawIntroBG() {    // dessine l'arrière-plan de l'écran d'accueil
    total_time++;
    if ((total_time+12) % 31 == 0) {
        x = rand() % 800;
        y = rand() % 600;
        circle_counter++;
    }    
        
    DrawCircleV({x, y}, radius, circle_counter % 4 == 0? (circle_counter % 3 == 0? Color {80,80,80,220}:Color {255,203,0,220}):(circle_counter % 3== 0? Color {10,10,10,220}: Color {230,41,55,220}));
    
}


void drawLed(const Led& led)    // fonction pour dessiner une LED
{
    if (playing) {
        if (mode == 0) {    // mode classique
            if (led.on) {   // LED allumée / bonus
                
                if (level != 1) {    // à partir du niveau 2 en mode classique, ajouter une position légèrement aléatoire à la LED pour rendre le jeu plus difficile
                    DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, animated_radius, bonus? GOLD:RED); // dessiner la LED avec une position légèrement aléatoire pour rendre le jeu plus difficile à partir du niveau 2
                } else {
                    DrawCircleV(led.position, animated_radius, RED);
                }
                
            } else {    // LED éteinte / malus

                if (level >= 3) {    // à partir du niveau 3 en mode classique, ajouter une position légèrement aléatoire à la LED pour rendre le jeu plus difficile
                    DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, animated_radius, malus? BLACK:DARKGRAY);
                } else if (level == 2) {
                    DrawCircleV({led.position.x + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2, led.position.y + rand() % ((level - 1) * 5) - (level - 1) * 5 / 2}, radius, DARKGRAY); 
                } else {
                    DrawCircleV(led.position, radius, DARKGRAY);
                }
            }
        } else {    // mode arcade
            if (led.on) {   // LED allumée
                DrawCircleV({led.position.x + rand() % 16 - 8, led.position.y + rand() % 16 - 8}, animated_radius, bonus? GOLD:RED);
            } else {    // LED éteinte
                DrawCircleV({led.position.x + rand() % 16 - 8, led.position.y + rand() % 16 - 8}, animated_radius, malus? BLACK:DARKGRAY);    // si le malus est actif et que la LED à dessiner est celle qui doit s'allumer, la dessiner en noir
            }
        }
    }
}

void reinitialize() {   // fonction pour réinitialiser les paramètres
    score_saved = false;
    playing = false;
    chrono_end_arcade = 0;
    can_play_music = true;
    new_best_score = false;
    level = -1;
    countdown_time = 60;
    on_led_lasting = 45;
    led_chrono = on_led_lasting;
    number = 3;
    score = 0;
    radius = 60;
    target_number = 0;
    nb_leds = 3;
    total_time = 0;
    round_tournament = 1;
    bonus = false;
    malus = false;
    life = 200;
    for (int i = 0; i < 11; i++) {
        leds_table[i].on = false;
    }
    new_global_best_score = false;
    StopMusicStream(bg_music);  // arrêtent les musiques en cours de lecture
    StopMusicStream(arcade_bg_music);
    StopMusicStream(intro_music);
}

void relaunch() {
    player_pseudo = "";
    level = -2;
}

int callbackGetID(void* data, int argc, char** argv, char**) {
    int* id = (int*)data;
    if (argc > 0 && argv[0]) {
        *id = std::stoi(argv[0]);
    }
    return 0;   
}

int callbackNewPlayer(void* data, int argc, char** argv, char**) {
    int* exists = (int*)data;
    if (argv[0]) {
        *exists = std::stoi(argv[0]);
    }

    return 0;

}

void pseudo() {
    
    remadePlayMusic(ticking_clock);
    UpdateMusicStream(ticking_clock);

    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS players (id INTEGER PRIMARY KEY, name TEXT UNIQUE, classic_best_score INTEGER, arcade_best_score INTEGER);", nullptr,nullptr, nullptr);

    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 125) {
                player_pseudo.push_back((char)key);
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !player_pseudo.empty()) {
            player_pseudo.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            sqlite3_exec(db, ("SELECT COUNT(*) FROM players WHERE name = '" + player_pseudo + "' LIMIT 1;").c_str(), callbackNewPlayer, &player_exists, nullptr);
            if (player_exists == 0) {
                sqlite3_exec(db, ("INSERT INTO players(name, classic_best_score, arcade_best_score) VALUES ('" + player_pseudo + "', 0, 0);").c_str(), nullptr, nullptr, nullptr);
            }
            sqlite3_exec(db, ("SELECT id FROM players WHERE name = '" + player_pseudo + "';").c_str(), callbackGetID, &player_id, nullptr);
            level = -1;
            StopMusicStream(ticking_clock);
            can_play_music = true;
        }

        DrawText("Enter your pseudo :", 50, 250, 20, DARKGRAY);
        DrawText((player_pseudo).c_str(), 270, 250, 20, BLACK);
    EndDrawing();
}

void menu() {   // fonction pour gérer le menu principal
    remadePlayMusic(intro_music);
    UpdateMusicStream(intro_music);
    BeginDrawing();
    
        ClearBackground(LIGHTGRAY);
        drawIntroBG();
        DrawText("Catch the light", 200, 100, 50, BLACK);
        DrawText(("Hello, " + player_pseudo + " !").c_str(), 10, 550, 17, BLACK);
        DrawText("Press SPACE to start", 260, 300, 20, BLACK);
        DrawText("Press A for Arcade Mode", 250, 350, 20, BLACK);
        DrawText("Click on the red LED to score points!", 200, 400, 20, BLACK);
        DrawText("Click on the dark LED to lose points!", 205, 450, 20, BLACK);
        DrawText("Wrong player ? Press R !", 290, 500, 15, BLACK);
        DrawText("MK", 750, 550, 20, BLACK);
        if (IsKeyPressed(KEY_R)) {
            reinitialize();
            relaunch();
        }
        if (IsKeyPressed(KEY_SPACE)) {    // si le joueur appuie sur la touche espace, passer au décompte du mode classique
            total_time = 0;
            playing = true;
            number = 3;
            countdown_time = 60;
            level = 0;
            can_play_music = true;
        } else if (IsKeyPressed(KEY_Q)) {    // si le joueur appuie sur la touche A (clavier AZERTY), passer au décompte du mode arcade
            total_time = 0;
            playing = true;
            number = 3;
            countdown_time = 60;
            level = 7;
            can_play_music = true;
        }
    EndDrawing();
}

void countdown() {   // fonction pour gérer le décompte avant le début du jeu classique
    StopMusicStream(intro_music);
    UpdateMusicStream(bg_music);
    if (countdown_time == 60) {
        PlaySound(whistle_sound);   // coup de sifflet toutes les secondes
    }
    countdown_time --;
    if (countdown_time == 0) {
        number --;
        countdown_time = 60;
    }
    if (number == 0) {  // passage au niveau suivant
        level = round_tournament;
        number = 3;
        round_tournament++;
        return;
    }
    BeginDrawing(); // affichage du décompte
        ClearBackground(LIGHTGRAY);
        DrawText(TextFormat("%d", number), 350, 250, 200, BLACK);
        DrawText(TextFormat("Level %d", round_tournament), 370, 500, 20, BLACK);  // affichage du niveau
    EndDrawing();
}

void arcadeCountown() {   // fonction pour gérer le décompte avant le début du jeu (format quasi-identique à countdown())
    StopMusicStream(intro_music);
    UpdateMusicStream(arcade_bg_music);
    if (countdown_time == 60) {
        PlaySound(whistle_sound);
    }
    countdown_time --;
    if (countdown_time == 0) {
        number --;
        countdown_time = 60;
    }
    if (number == 0) {
        level = 8;
        number = 3;
        return;
    }
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        DrawText(TextFormat("%d", number), 350, 250, 200, BLACK);
        DrawText(TextFormat("Ready ?"), 370, 500, 20, BLACK); 
    EndDrawing();
}

void game() {   // fonction pour gérer le jeu classique en lui-même
    remadePlayMusic(bg_music);
    UpdateMusicStream(bg_music);
    mode = 0;    // mode classique
    if (IsKeyPressed(KEY_P)) {    // si le joueur appuie sur la touche P, mettre le jeu en pause
        can_play_sound = true;
        can_play_music = true;
        level = 10;
        round_tournament--;
    }
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        total_time++;
        float progression = total_time / 600.0f;    // calcul de la progression du temps pour faire évoluer le jeu en fonction du temps écoulé
        animated_radius = radius + 3 * sinf(GetTime() * 8.0f);   

        if (total_time == 300 && level >= 2) {    // après 5 secondes de jeu, faire apparaître le bonus (LED dorée) pour 3 secondes
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
            drawLed(leds_table[i]);    // dessiner les LEDs en fonction de leur état (allumée ou éteinte)
        }
        led_chrono --;  // décrémenter le chronameètre pour gérer l'allumage des LEDs
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // vérifier si le joueur a cliqué
            Vector2 mouse_pos = GetMousePosition();
            for (int i = 0; i < nb_leds; i++) {
                if (leds_table[i].on && CheckCollisionPointCircle(mouse_pos, leds_table[i].position, animated_radius)) {   // vérifier si le clic est sur une LED allumée 
                    if (bonus && i == target_number) {    // si le bonus est actif et que le joueur a cliqué sur la LED dorée, lui donner un bonus de points
                        PlaySound(bonus_touched_sound);
                        score += 5;
                        bonus = false;    // désactiver le bonus après que le joueur l'ait attrapé
                    } else {
                        PlaySound(led_touched_sound);
                        score++;    // sinon, incrémenter le score normalement
                    }
                    leds_table[i].on = false;
                    led_chrono = 0;
                } else if (!leds_table[i].on && CheckCollisionPointCircle(mouse_pos, leds_table[i].position, radius - level * 5)) {
                    if (malus) {    // si le malus est actif et que le joueur a cliqué sur la LED noire, lui donner un malus de points
                        PlaySound(malus_sound);
                        score -= 5;
                        malus = false;    // désactiver le malus après que le joueur l'ait attrapé
                    } else {
                        PlaySound(led_off_touched_sound);
                        score--;    // pénalité si le joueur clique sur une LED éteinte
                    }
                    break;  // éviter de pénaliser plusieurs fois pour un même clic
                }
            }
        }
        if (led_chrono == 0) {  // si le chronameètre pour l'allumage des LEDs est à 0, allumer une nouvelle LED
            for (int i = 0; i < nb_leds; i++) {
                leds_table[i].on = false;   // éteindre toutes les LEDs avant d'en allumer une nouvelle
            }
            int x = target_number;  // stock du numéro de la LED qui doit s'allumer pour éviter de tomber sur la même LED que précédemment
            while (x == target_number) {
                target_number = rand() % nb_leds;   // choisir une LED aléatoire parmi celles disponibles
            }
            leds_table[target_number].on = true;
            led_chrono = on_led_lasting;    // réinitialiser le chronameètre pour l'allumage des LEDs

        }

        DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);    // afficher le score en haut à gauche
        DrawRectangle(10, 40, 200, 20, GRAY);    // afficher une barre de progression du temps écoulé pour le changement de niveau
        DrawRectangle(10, 40, 200 * progression, 20, bonus? YELLOW : malus? BLACK : DARKGRAY);    // la barre de progression est jaune si le bonus est actif, grise sinon
        if (total_time >= 600) {    // après 10 secondes de jeu, augmenter le niveau pour rendre le jeu plus difficile et réinitialiser les paramètres
            if (level != 5) {
                level = 0;
            } else {
                can_play_sound = true;
                level = 6;
                playing = false;
            }
            total_time = 0;
            number = 3;
            on_led_lasting -= 3;
            nb_leds += 2;
            for (int i = 0; i < nb_leds; i++) {
                leds_table[i].on = false;
            }
            
        }


    EndDrawing();
}


void gameEnd() {    // fonction pour gérer l'écran de fin du jeu classique
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        if (!score_saved) {
            sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS classic (id_game INTEGER PRIMARY KEY, player_id INTEGER, name TEXT, score INTEGER, FOREIGN KEY (player_id) REFERENCES players(id), FOREIGN KEY (name) REFERENCES players(name));", nullptr, nullptr, nullptr);
            sqlite3_exec(db, ("SELECT classic_best_score FROM players WHERE id = " + std::to_string(player_id) + ";").c_str(), callbackBestScore, &classic_best_score, nullptr);
            std::string add_classic_score_command = "INSERT INTO classic (player_id, name, score) VALUES ("
                + std::to_string(player_id) + ", '"
                + player_pseudo + "', "
                + std::to_string(score) + ");";
            sqlite3_exec(db, add_classic_score_command.c_str(), callbackGetID, &player_id, nullptr);
            sqlite3_exec(db, "SELECT MAX(classic_best_score) FROM players;", callbackBestScore, &global_classic_best_score, nullptr);
            sqlite3_exec(db, ("UPDATE players SET classic_best_score = (SELECT MAX(score) FROM classic WHERE player_id = " + std::to_string(player_id) + ") WHERE id = " + std::to_string(player_id) + ";").c_str(), callbackBestScore, &classic_best_score, nullptr);
            score_saved = true;
        }

        if (score > global_classic_best_score) {
            global_classic_best_score = score;
            new_global_best_score = true;
        } else if (score > classic_best_score) {   // sauvegarde du meilleur score si le précédent vient d'être battu
            classic_best_score = score;
            new_best_score = true;
        }

        // Affichage d'un message personnalisé en fonction du score obtenu
        if (new_global_best_score) {
            remadePlaySound(new_best_score_sound);
            DrawText("THE BEST SCORE EVER !!", 135, 250, 40, GOLD);                
        } else if (new_best_score) {
            DrawText("Incredible ! New best score !", 100, 250, 40, GOLD);
            remadePlaySound(new_best_score_sound);
        } else {
            if (score < 0) {
                remadePlaySound(game_lost_sound);            
                DrawText("You are probably the worst player ever...", 100, 250, 30, BLACK);
            } else {
                switch (score / 20) {   // afficher un message différent en fonction du score final du joueur
                    case 0:
                        remadePlaySound(game_lost_sound);
                        DrawText("What happened...", 200, 250, 50, BLACK);
                        break;
                    case 1:
                        remadePlaySound(game_lost_sound);
                        DrawText("So so...", 310, 250, 50, BLACK);
                        break;
                    case 2:
                        remadePlaySound(game_lost_sound);
                        DrawText("That's fine, you can do better though...", 100, 250, 30, BLACK);
                        break;
                    case 3:
                        remadePlaySound(game_won_sound);
                        DrawText("Not bad!", 290, 250, 50, BLACK);
                        break;
                    case 4:
                        remadePlaySound(game_won_sound);
                        DrawText("Great job!", 270, 250, 50, BLACK);
                        break;
                    case 5:
                        remadePlaySound(game_won_sound);
                        DrawText("Amazing! You are a pro!", 120, 250, 50, BLACK);
                        break;
                    default:
                        remadePlaySound(game_won_sound);
                        DrawText("Unbelievable... are you sure you are not cheating?", 17, 250, 30, BLACK);
                        break;
                }
            }
        }

        // Affichage des commandes possibles et des scores

        DrawText(TextFormat("Score: %d", score), 340, 350, 20, BLACK);
        DrawText(TextFormat("Personal best score : %d", classic_best_score), 270, 500, 20, DARKGRAY);
        DrawText(TextFormat("Best score ever : %d", global_classic_best_score), 290, 550, 20, DARKGRAY);
        DrawText("Press ESC to exit", 300, 400, 20, BLACK);
        DrawText("Press R to restart", 296, 450, 20, BLACK);
        if (IsKeyPressed(KEY_R)) {    // si le joueur appuie sur la touche R, réinitialiser le jeu pour recommencer une nouvelle partie
            reinitialize();
        }
    EndDrawing();
}

void arcade() {   // fonction pour gérer le mode arcade
    mode = 1;    // mode arcade
    nb_leds = 11;    // dans le mode arcade, on utilise toutes les LEDs disponibles

    remadePlayMusic(arcade_bg_music);
    UpdateMusicStream(arcade_bg_music);

    if (IsKeyPressed(KEY_P)) {    // si le joueur appuie sur la touche P, mettre le jeu en pause
        can_play_sound = true;
        can_play_music = true;
        level = 10;
        round_tournament--;
    }
    BeginDrawing();
        ClearBackground(LIGHTGRAY);

        if (playing) {
        
            total_time++;
            life -= 0.02;
            animated_radius = radius + 3 * sinf(GetTime() * 8.0f);  // dé/grossissement des LED
            
            if (on_led_lasting > 25) {    // faire évoluer la durée d'allumage des LEDs en fonction du temps écoulé pour rendre le jeu plus difficile au fur et à mesure du temps (de 30 frames à 50 frames au début, puis jusqu'à 30 frames à la fin)
                on_led_lasting = 30 + (1 - total_time / 1800.0f) * 20;
            }
            
            if ((total_time + 300) % 600 == 0) {    // après 5 secondes de jeu, faire apparaître le bonus (LED dorée) toutes les 10 secondes de jeu pour 1 seconde (t0 = 3)
                bonus = true;
                timer_bonus = 0;    // réinitialiser le chronameètre pour gérer la durée du bonus
            } else if (bonus) {
                timer_bonus++;    // incrémenter le chronameètre pour gérer la durée du bonus
                if (timer_bonus >= 60) {    // si le bonus est actif depuis 1 seconde, le désactiver
                    bonus = false;
                }
            }

            if ((total_time + 180) % 600 == 0) {   // après 7 secondes de jeu, faire apparaître le malus (LED noire) toutes les 10 secondes de jeu pour 1 seconde (t0 = 7)
                malus = true;
                timer_malus = 0;    // réinitialiser le chronameètre pour gérer la durée du malus
            } else if (malus) {
                timer_malus++;    // incrémenter le chronameètre pour gérer la durée du malus
                if (timer_malus >= 60) {    // si le malus est actif depuis 1 seconde, le désactiver
                    malus = false;
                }
            }

            for (int i = 0; i < nb_leds; i++) {
                drawLed(leds_table[i]);    // dessiner les LEDs en fonction de leur état (allumée ou éteinte)
            }
            led_chrono --;  // décrémenter le chronameètre pour gérer l'allumage des LEDs
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {  // vérifier si le joueur a cliqué
                Vector2 mouse_pos = GetMousePosition();
                for (int i = 0; i < nb_leds; i++) {
                    if (leds_table[i].on && CheckCollisionPointCircle(mouse_pos, leds_table[i].position, animated_radius)) {   // vérifier si le clic est sur une LED allumée 
                        if (bonus && i == target_number) {    // si le bonus est actif et que le joueur a cliqué sur la LED dorée, lui donner un bonus de points
                            PlaySound(bonus_touched_sound);
                            score += 3;
                            bonus = false;    // désactiver le bonus après que le joueur l'ait attrapé
                        } else {
                            PlaySound(led_touched_sound);
                            score++;    // sinon, incrémenter le score normalement
                        }
                        leds_table[i].on = false;
                        led_chrono = 0;
                    } else if (!leds_table[i].on && CheckCollisionPointCircle(mouse_pos, leds_table[i].position, animated_radius)) {
                        if (malus) {    // si le malus est actif et que le joueur a cliqué sur la LED noire, lui donner un malus de points
                            PlaySound(malus_sound);
                            score -= 3;
                            life -= 40;    // pénalité de vie si le joueur clique sur une LED noire
                            malus = false;    // désactiver le malus après que le joueur l'ait attrapé
                        } else {
                            PlaySound(led_off_touched_sound);
                            life -= 10;    // pénalité de vie si le joueur clique sur une LED éteinte
                        }
                        break;  // éviter de pénaliser plusieurs fois pour un même clic
                    }
                }
            }

            if (led_chrono == 0) {  // si le chronameètre pour l'allumage des LEDs est à 0, allumer une nouvelle LED
                for (int i = 0; i < nb_leds; i++) {
                    leds_table[i].on = false;   // éteindre toutes les LEDs avant d'en allumer une nouvelle
                }
                int x = target_number;  // stock du numéro de la LED qui doit s'allumer pour éviter de tomber sur la même LED que précédemment
                while (x == target_number) {
                    target_number = rand() % nb_leds;   // choisir une LED aléatoire parmi celles disponibles
                }
                leds_table[target_number].on = true;
                led_chrono = on_led_lasting;    // réinitialiser le chronameètre pour l'allumage des LEDs

            }

            DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);    // afficher le score en haut à gauche
            DrawRectangle(10, 40, 200, 20, GRAY);
            DrawRectangle(10, 40, life, 20, bonus? YELLOW : malus? BLACK : RED);    // afficher une barre de progression de la vie           

        }

        if (life <= 0) {    // si la vie du joueur est à 0, passer à l'écran de fin du mode arcade et réinitialiser les params
            playing = false;
            chrono_end_arcade++;
            StopMusicStream(arcade_bg_music);            
            total_time = 0;
            if (chrono_end_arcade == 1) {
                can_play_sound = true;
            }
            remadePlaySound(end_arcade_sound);
            chrono_end_arcade++;
            if (chrono_end_arcade == 180) {
                level = 9;
                can_play_sound = true;
            }
        }

    EndDrawing();
}

void arcadeEnd() {   // fonction pour gérer la fin du mode arcade
    StopMusicStream(arcade_bg_music);
    
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        if (!score_saved) {
            sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS arcade (id_game INTEGER PRIMARY KEY, player_id INTEGER, name TEXT, score INTEGER, FOREIGN KEY (player_id) REFERENCES players(id), FOREIGN KEY (name) REFERENCES players(name));", nullptr, nullptr, nullptr);
            sqlite3_exec(db, ("SELECT arcade_best_score FROM players WHERE id = " + std::to_string(player_id) + ";").c_str(), callbackBestScore, &arcade_best_score, nullptr);
            std::string add_arcade_score_command = "INSERT INTO arcade (player_id, name, score) VALUES ("
                + std::to_string(player_id) + ", '"
                + player_pseudo + "', "
                + std::to_string(score) + ");";
            sqlite3_exec(db, add_arcade_score_command.c_str(), nullptr, nullptr, nullptr);
            sqlite3_exec(db, "SELECT MAX(arcade_best_score) FROM players;",callbackBestScore, &global_arcade_best_score, nullptr);
            sqlite3_exec(db, ("UPDATE players SET arcade_best_score = (SELECT MAX(score) FROM arcade WHERE player_id = " + std::to_string(player_id) + ") WHERE id = " + std::to_string(player_id) + ";").c_str(), nullptr, nullptr, nullptr);
            score_saved = true;
        }
        if (score > global_arcade_best_score) {
            global_arcade_best_score = score;
            new_global_best_score = true;
        } else if (score > arcade_best_score) {    // sauvegarde meilleur score si précédent battu
            arcade_best_score = score;
            new_best_score = true;
        }

        if (new_global_best_score) {
            remadePlaySound(new_best_score_sound);
            DrawText("THE BEST SCORE EVER !!", 135, 250, 40, GOLD);       
        } else if (new_best_score) {   // affichage msg perso
            remadePlaySound(new_best_score_sound);
            DrawText("Incredible ! New best score !", 100, 250, 40, GOLD);
        } else {
            remadePlaySound(arcade_lost_sound);
            DrawText("You lost!", 280, 250, 50, BLACK);
        }
        // Affichage commandes possibles et scores
        DrawText(TextFormat("Score: %d", score), 340, 350, 20, BLACK);
        DrawText(TextFormat("Personal best score : %d", arcade_best_score), 270, 500, 20, DARKGRAY);
        DrawText(TextFormat("Best score ever : %d", global_arcade_best_score), 290, 550, 20, DARKGRAY);
        DrawText("Press ESC to exit", 300, 400, 20, BLACK);
        DrawText("Press R to restart", 296, 450, 20, BLACK);
        if (IsKeyPressed(KEY_R)) {    // si le joueur appuie sur la touche R, réinitialiser le jeu pour recommencer une nouvelle partie
            reinitialize();
        }
    EndDrawing();
}

void pause() {    // fonction pour gérer la pause du jeu
    remadePlaySound(pause_sound);
    PauseMusicStream(arcade_bg_music);
    PauseMusicStream(bg_music);
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        DrawText("Game Paused", 240, 250, 50, BLACK);
        DrawText("Press P to resume", 300, 350, 20, BLACK);
        DrawText("Press M for menu", 305, 370, 20, BLACK);
        if (IsKeyPressed(KEY_SEMICOLON)) {      // retour au menu (semicolon = point-virgule, à la place du M en clavier QWERTY)
            reinitialize();
        }
        if (IsKeyPressed(KEY_P)) {    // si le joueur appuie sur la touche P, reprendre le jeu
            level = precedent_level;    // revenir au niveau précédent avant la pause
            ResumeMusicStream(bg_music);
            ResumeMusicStream(arcade_bg_music);
        }
    EndDrawing();
}

// ============================================================
//  Point d'entrée du programme
// ============================================================


int main()
{
    if (sqlite3_open("catch_the_light_database.db", &db) != SQLITE_OK) {
        std::cout << sqlite3_errmsg(db) << '\n';
    }

    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    
    
    srand(time(nullptr));   // initialiser la graine pour les namebres aléatoires en fonction du temps actuel pour que les positions des LEDs soient différentes à chaque exécution du jeu
    const int LARGEUR  = 800;
    const int HAUTEUR  = 600;
    const int FPS_CIBLE = 60;

    InitWindow(LARGEUR, HAUTEUR, "Catch the light");
    SetTargetFPS(FPS_CIBLE);    

    InitAudioDevice();  

    // Chargement des sons

    intro_music = LoadMusicStream("assets/intro.mp3");
    arcade_lost_sound = LoadSound("assets/arcade_and_game_lost.mp3");
    new_best_score_sound = LoadSound("assets/new_best_score.mp3");
    game_won_sound = LoadSound("assets/game_end.mp3");
    bonus_touched_sound = LoadSound("assets/bonus_touched.mp3");
    led_touched_sound = LoadSound("assets/led_touched.mp3");
    malus_sound = LoadSound("assets/malus.mp3");
    whistle_sound = LoadSound("assets/whistle.mp3");
    pause_sound = LoadSound("assets/pause.mp3");
    led_off_touched_sound = LoadSound("assets/led_off_touched.mp3");
    bg_music = LoadMusicStream("assets/bg_music.mp3");
    arcade_bg_music = LoadMusicStream("assets/arcade_bg_music.mp3");
    end_arcade_sound = LoadSound("assets/end_arcade.mp3");
    game_lost_sound = LoadSound("assets/game_lost.mp3");
    ticking_clock = LoadMusicStream("assets/ticking_clock.mp3");

    SetMusicVolume(bg_music, 0.4);

    // initialisation des LEDs avec leurs positions, leur radius, leur couleur et leur état (allumée ou éteinte)

    leds_table[0] = {{400, 300}, radius, DARKGRAY, false};
    leds_table[1] = {{620, 300}, radius, DARKGRAY, false};
    leds_table[2] = {{180, 300}, radius, DARKGRAY, false};
    leds_table[3] = {{332, 91}, radius, DARKGRAY, false};
    leds_table[4] = {{468, 509}, radius, DARKGRAY, false};
    leds_table[5] = {{222, 171}, radius, DARKGRAY, false};
    leds_table[6] = {{578, 429}, radius, DARKGRAY, false};
    leds_table[7] = {{222, 429}, radius, DARKGRAY, false};
    leds_table[8] = {{578, 171}, radius, DARKGRAY, false};
    leds_table[9] = {{332, 509}, radius, DARKGRAY, false};
    leds_table[10] = {{468, 91}, radius, DARKGRAY, false};

    while (!WindowShouldClose())
    {
        switch (level) {    // en fonction du niveau actuel, appeler la fonction correspondante pour gérer l'affichage et la logique du jeu
            case -2:
                pseudo();
                break;
            case -1:
                menu();
                break;
            case 0:
                countdown();
                break;
            case 6:
                gameEnd();
                break;
            case 7:
                arcadeCountown();
                break;
            case 8:
                precedent_level = 7;    
                arcade();
                break;
            case 9:
                arcadeEnd();
                break;
            case 10:
                pause();
                break;
            default:
                precedent_level = 0;    
                game();
                break;
        }
    }
    
    // Déchargement des sons si le jeu est fermé

    UnloadSound(arcade_lost_sound);
    UnloadSound(bonus_touched_sound);
    UnloadSound(game_won_sound);
    UnloadSound(game_lost_sound);
    UnloadSound(led_touched_sound);
    UnloadSound(led_off_touched_sound);
    UnloadSound(malus_sound);
    UnloadSound(new_best_score_sound);
    UnloadSound(pause_sound);
    UnloadSound(whistle_sound);
    UnloadSound(end_arcade_sound);
    UnloadMusicStream(intro_music);
    UnloadMusicStream(bg_music);
    UnloadMusicStream(arcade_bg_music);
    UnloadMusicStream(ticking_clock);

    CloseAudioDevice();
    CloseWindow();
    sqlite3_close(db);
    return 0;
}
