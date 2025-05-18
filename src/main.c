#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PINS 250
#define CORE_RADIUS 70
#define ATTACH_RADIUS 160
#define PIN_RADIUS 12
#define PIN_DISTANCE 160
#define PIN_SPEED 6.0f
#define COLLISION_THRESHOLD 9.0f

typedef enum scene { main_menu, game, fail, level_end, menu, settings, level_menu } scene;

scene current_scene = main_menu;

typedef struct {
    float angle;
    float yOffset;
    bool attached;
    bool collided;
} Pin;

typedef struct {
    int x, y, width, height, letterSize;
} Button;


void ResetGame(Pin pins[MAX_PINS], int *pinCount, bool *gameOver) {
    *pinCount = 0;
    *gameOver = false;
    for (int i = 0; i < MAX_PINS; i++) {
        pins[i].attached = false;
        pins[i].collided = false;
    }
}

void setLevel(int level, int *level_pin, Pin pins[MAX_PINS], int *pinCount) {

    if (level < 1) level = 1;

    if(level >= 9){
        *level_pin = (level-2)*2;
        if(level >=15){*level_pin = (level-2)*2;}
    }
    else{
        *level_pin = 3 + (level - 1) * 2;
    }

    int obstacle_pin = 0;
    int reverse = 0;

    if (level >= 2 && level < 4) { obstacle_pin = 1; *level_pin = 3 + level + 1;}
    if (level >= 4 && level < 7) { obstacle_pin = 2; *level_pin = level + 5;}
    if (level >= 7 && level < 12) { obstacle_pin = 3; *level_pin = level + 5;}
    if (level >= 12 && level < 18) { obstacle_pin = 4; *level_pin = level;}
    if (level >= 18 && level < 25) { obstacle_pin = 5; *level_pin = level;}
    if (level >= 25 && level < 32) { obstacle_pin = 6; *level_pin = level - 2;}

    for(int i=0; i <= obstacle_pin; i++){
                
        pins[i].angle= i*(360.0f/obstacle_pin);
        pins[i].yOffset= ATTACH_RADIUS;
        pins[i].attached=true;
        pins[i].collided=false;
    }
    *pinCount = obstacle_pin;

}

void DrawCenteredText(const char *text, Rectangle bounds, int fontSize, Color color) {
    Vector2 size = MeasureTextEx(GetFontDefault(), text, fontSize, 0);
    float x = bounds.x + (bounds.width - size.x) / 2;
    float y = bounds.y + (bounds.height - size.y) / 2;
    DrawText(text, x, y, fontSize, color);
}

int main() {
    const int screenWidth = 400;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "aa Game in Raylib");
    
    InitAudioDevice();  
    Music game_music = LoadMusicStream("resources/Flying_me_softly.mp3");
    Music beep_sound = LoadMusicStream("resources/beep.mp3");
    Sound fail_sound = LoadSound("resources/fail.mp3");
    Sound pin_sound = LoadSound("resources/pin.wav");

    PlayMusicStream(game_music);
    PlayMusicStream(beep_sound);
   

    Color lightBeige = (Color){ 243, 243, 224, 255 };
    Color darkBackground = (Color){60, 61, 55, 255};
    Color darkMaroon = (Color){66, 1, 1, 255};
    Color transparent = (Color){0, 0, 0, 0};

    Pin pins[MAX_PINS] = {0};
    

    int pinCount = 0;
    int highest_level_reached = 1;
    int current_level = 1;
    int level_pin = 0;
    int pin_start_point = screenHeight - 200;
    int inputLength = 0;

    float failTimer = 0;
    float rotationSpeed = 0;
    float rotationTimer= 1;

    bool level_initialized = false;
    bool gameOver = false;
    bool failTriggered = false;
    bool music_on = true;
    bool sound_on = true;
    bool dark_mode = false;
    
    char pinCountText[10];
    char levelInput[3] = "";

   
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (current_scene == game) {
            if (!level_initialized) {
                ResetGame(pins, &pinCount, &gameOver);
                setLevel(current_level, &level_pin, pins, &pinCount);
                level_initialized = true;
            }

            if (!gameOver) {
                if (IsKeyPressed(KEY_SPACE) && pinCount < level_pin) {
                    pins[pinCount].angle = 90;
                    pins[pinCount].yOffset = pin_start_point - 200;
                    pins[pinCount].attached = false;
                    pinCount++;
                }


                rotationTimer += GetFrameTime();
                if (fmod(rotationTimer, 3.0f) < 1.0f) {
                    rotationSpeed = 2.0f;
                } else {
                    rotationSpeed = 1.0f;
                }

                for (int i = 0; i < pinCount; i++) {
                    if (!pins[i].attached) {
                        pins[i].yOffset -= PIN_SPEED;
                        if (pins[i].yOffset <= ATTACH_RADIUS){
                            pins[i].yOffset = ATTACH_RADIUS;
                            pins[i].attached = true;
                            if(sound_on==true) PlaySound(pin_sound);
                        }
                    } else {
                        pins[i].angle += rotationSpeed;
                        if (pins[i].angle >= 360.0f) pins[i].angle -= 360.0f;
                    }

                    for (int j = 0; j < i; j++) {
                        if (pins[i].attached && pins[j].attached) {
                            float angleDiff = fabs(fmod(pins[i].angle - pins[j].angle + 360.0f, 360.0f));
                            if (angleDiff < COLLISION_THRESHOLD || angleDiff > (360.0f - COLLISION_THRESHOLD)) {
                                if(sound_on==true) PlaySound(fail_sound);
                                pins[i].collided = true;
                                pins[j].collided = true;
                                gameOver = true;
                                failTriggered = true;
                                failTimer = 0.0f;
                            }
                        }
                    }
                }

                // Level passed check
                bool allAttached = true;
                for (int i = 0; i < pinCount; i++) {
                    if (!pins[i].attached) {
                        allAttached = false;
                        break;
                    }
                }

                if (pinCount == level_pin && allAttached && !gameOver) {
                    level_initialized = false;
                    current_scene = level_end;
                }
                
            }
            if (failTriggered) {
                failTimer += GetFrameTime();
                if (failTimer >= 1.0f) {
                    current_scene = fail;
                    failTriggered = false;
                }
            }
        }

        BeginDrawing();
        if (failTriggered) {
            Color fail_color = dark_mode? darkMaroon : MAROON;
            ClearBackground(fail_color);
        } else {
            Color background_color = dark_mode? darkBackground : lightBeige;
            ClearBackground(background_color);
        }

        switch (current_scene) {
            case main_menu: {
                DrawText(".AA.", screenWidth/2 - 55, screenHeight/4, 60, BLACK);

                //START Button
                Rectangle startBtn = {(screenWidth - 120)/2, 320, 120, 40};
                DrawRectangleRec(startBtn, DARKGRAY);
                DrawCenteredText("Start", startBtn, 20, WHITE);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= startBtn.x && mouse.x <= startBtn.x + startBtn.width &&
                        mouse.y >= startBtn.y && mouse.y <= startBtn.y + startBtn.height) {
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        current_scene = game;
                        current_level = highest_level_reached;
                        level_initialized = false; 
                    }
                }
               
                //MENU Button
                Rectangle menuBtn = {300, 30, 70, 30};
                DrawRectangleRec(menuBtn, DARKGRAY);
                DrawCenteredText("MENU", menuBtn, 10, WHITE);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= menuBtn.x && mouse.x <= menuBtn.x + menuBtn.width &&
                        mouse.y >= menuBtn.y  && mouse.y <= menuBtn.y + menuBtn.height) {
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        current_scene = menu;
                    }
                }

                //Dark Mode Button
                Rectangle darkModeBtn = {screenWidth/2 - 30, 30, 120, 30};
                DrawRectangleRec(darkModeBtn, DARKGRAY);
                DrawCenteredText("Dark Mode", darkModeBtn, 10, WHITE);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= darkModeBtn.x && mouse.x <= darkModeBtn.x + darkModeBtn.width &&
                        mouse.y >= darkModeBtn.y && mouse.y <= darkModeBtn.y + darkModeBtn.height) {
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        dark_mode = !(dark_mode);
                    }
                }

            } break;
            case menu: {
                //Buttons
                Button soundBtn = {80, 125, 120, 30, 30};
                Button musicBtn = {240, 125, 120, 30, 30};
                Button darkModeBtn = {240, 225, 120, 30, 30};
                Button replayBtn = {80, 225, 120, 30, 10};
                Button backBtn = {360, 10, 30, 30, 30};
                Button playRoundsBtn = {80, 325, 120, 30, 30};
                
                //Sound on/off Button
                const char* sound_on_off = sound_on ? "on" : "off";
            
                DrawRectangle(soundBtn.x, soundBtn.y, soundBtn.width, soundBtn.height, DARKGRAY);
                DrawCenteredText(sound_on ? "Sound on" : "Sound off", (Rectangle){soundBtn.x, soundBtn.y, soundBtn.width, soundBtn.height}, 10, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= soundBtn.x && mouse.x <= soundBtn.x + soundBtn.width &&
                        mouse.y >= soundBtn.y && mouse.y <= soundBtn.y + soundBtn.height) {
                        sound_on = !(sound_on);
                        if(sound_on == true) UpdateMusicStream(beep_sound);

                    }
                }
            
                //Music Button
                const char* music_on_off = music_on ? "on" : "off";
                
                DrawRectangle(musicBtn.x, musicBtn.y, musicBtn.width, musicBtn.height, DARKGRAY);
                DrawCenteredText(music_on ? "Music on" : "Music off", (Rectangle){musicBtn.x, musicBtn.y, musicBtn.width, musicBtn.height}, 10, WHITE);
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= musicBtn.x && mouse.x <= musicBtn.x + musicBtn.width &&
                        mouse.y >= musicBtn.y && mouse.y <= musicBtn.y + musicBtn.height) {
                        music_on = !(music_on);
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                    }
                }
            
                //Dark Mode Button
                DrawRectangle(darkModeBtn.x, darkModeBtn.y, darkModeBtn.width, darkModeBtn.height, DARKGRAY);
                DrawCenteredText("Dark Mode", (Rectangle){darkModeBtn.x, darkModeBtn.y, darkModeBtn.width, darkModeBtn.height}, 10, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= darkModeBtn.x && mouse.x <= darkModeBtn.x + darkModeBtn.width &&
                        mouse.y >= darkModeBtn.y && mouse.y <= darkModeBtn.y + darkModeBtn.height) {
                        dark_mode = !(dark_mode);
                    }
                }
            
                // Play Rounds Button
                /*
                DrawRectangle(playRoundsBtn.x, playRoundsBtn.y, playRoundsBtn.width, playRoundsBtn.height, DARKGRAY);
                DrawText("Play Rounds", playRoundsBtn.x + 5, playRoundsBtn.y + 5, 10, WHITE);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= playRoundsBtn.x && mouse.x <= playRoundsBtn.x + playRoundsBtn.width &&
                        mouse.y >= playRoundsBtn.y && mouse.y <= playRoundsBtn.y + playRoundsBtn.height) {

                        }
                }
                */


                // Replay Level Button
                DrawRectangle(replayBtn.x, replayBtn.y, replayBtn.width, replayBtn.height, DARKGRAY);
                DrawCenteredText("Replay Level", (Rectangle){replayBtn.x, replayBtn.y, replayBtn.width, replayBtn.height}, 10, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= replayBtn.x && mouse.x <= replayBtn.x + replayBtn.width &&
                        mouse.y >= replayBtn.y && mouse.y <= replayBtn.y + replayBtn.height) {
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        current_scene = level_menu;
                    }
                }
            
                //Back To Main Menu Button
                DrawRectangle(backBtn.x, backBtn.y, backBtn.width, backBtn.height, transparent);
                DrawCenteredText("x", (Rectangle){backBtn.x, backBtn.y, backBtn.width, backBtn.height}, 30, BLACK);

            
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= backBtn.x && mouse.x <= backBtn.x + backBtn.width &&
                        mouse.y >= backBtn.y && mouse.y <= backBtn.y + backBtn.height) {
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        current_scene = main_menu;                        
                    }
                }
            } break;
            case level_menu: {

                DrawText("Replay Level", 125, 68, 28, BLACK);
                DrawText("    Replay any level that \n you have already passed", 90, 110, 18, BLACK);
                DrawText(TextFormat("Levels are available \n     between 1 & %d", highest_level_reached), 90, 154, 24, BLACK);


                int backWidth = 30; int backHeight = 30; int backX = 360; int backY = 10; int backLetterSize = 30;

                int boxPerRow = 3;
                int boxSize = 40;
                int spacing = 20;
                int startX = 115;
                int startY = 220;

                for (int i = 1; i <= 12; i++) {
                    int displayNumber = (i == 12) ? 0 : i;

                    int row = (i - 1) / boxPerRow;
                    int col = (i - 1) % boxPerRow;
                    int x = startX + col * (boxSize + spacing);
                    int y = startY + row * (boxSize + spacing);

                    // CLEAR BUTTON
                    if (i == 10) {
                        int clearWidth = 2 * boxSize + spacing;
                        int clearHeight = boxSize;

                        DrawRectangleLines(x, y, clearWidth, clearHeight, BLACK);
                        DrawText("Clear", x + 15, y + 10, 20, BLACK);

                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            Vector2 mouse = GetMousePosition();
                            if (CheckCollisionPointRec(mouse, (Rectangle){x, y, clearWidth, clearHeight})) {
                                inputLength = 0;
                                levelInput[0] = '\0';
                            }
                        }
                        i++; 
                        continue;
                    }

                    // DIGIT BUTTONS
                    DrawRectangleLines(x, y, boxSize, boxSize, BLACK);
                    DrawText(TextFormat(" %d", displayNumber), x + 15, y + 10, 20, BLACK);

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        Vector2 mouse = GetMousePosition();
                        if (CheckCollisionPointRec(mouse, (Rectangle){x, y, boxSize, boxSize}) && inputLength < 2) {
                            levelInput[inputLength++] = '0' + displayNumber;
                            levelInput[inputLength] = '\0';
                        }
                    }
                }

                // Enter Button
                int enterX = 115;
                int enterY = 480;
                int enterWidth = 160;
                int enterHeight = 40;

                DrawRectangle(enterX, enterY, enterWidth, enterHeight, DARKGRAY);
                DrawText("Enter ", enterX + 50, enterY + 10, 20, WHITE);
                DrawText(levelInput, enterX + 110, enterY + 10, 20, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, (Rectangle){enterX, enterY, enterWidth, enterHeight})) {
                        int selectedLevel = atoi(levelInput);
                        if (selectedLevel >= 1 && selectedLevel <= highest_level_reached) {
                            current_level = selectedLevel;
                            level_initialized = false;
                            current_scene = game;
                        }
                    }
                }
                

                //Back to menu
                DrawRectangle(backX, backY, backWidth, backHeight, transparent);
                Vector2 backTextSize = MeasureTextEx(GetFontDefault(), "x", backLetterSize, 0);
                float backtextX = backX + (backWidth - backTextSize.x) / 2;
                float backtextY = backY + (backHeight - backTextSize.y) / 2;
                DrawText("x", backtextX, backtextY, backLetterSize, BLACK);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= backX && mouse.x <= backX + backWidth &&
                        mouse.y >= backY  && mouse.y <= backY + backHeight) {
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        current_scene = menu;
                        
                    }
                }
            } break;
            
            case game: {
                if(music_on == true){
                    UpdateMusicStream(game_music);
                }

                int coreX = screenWidth / 2;
                int coreY = screenHeight / 3;

                DrawCircle(coreX, coreY, CORE_RADIUS, BLACK);
                
                //DrawCircleLines(coreX, coreY, ATTACH_RADIUS, LIGHTGRAY); -> Pins Attach Radius 

                for (int i = 0; i < pinCount; i++) {
                    if (!pins[i].attached) {
                        DrawCircle(coreX, coreY + pins[i].yOffset, PIN_RADIUS, BLACK);
                    } else {
                        float rad = (pins[i].angle) * DEG2RAD;
                        float pinX = coreX + cos(rad) * ATTACH_RADIUS;
                        float pinY = coreY + sin(rad) * ATTACH_RADIUS;
                        DrawLine(coreX, coreY, pinX, pinY, BLACK);
                       
                        Color pinColor = RED;
                        if(pins[i].collided) {
                            pinColor = YELLOW;
                        }
                        DrawCircle(pinX, pinY, PIN_RADIUS, pinColor);
                    }
                }

                if (pinCount < level_pin) {
                    if (pinCount < level_pin) {
                        int remaining_pins = level_pin - pinCount;
                        int maxVisible;
                        if(remaining_pins < 6) maxVisible = remaining_pins;
                        else maxVisible = 6; 

                        int baseY = pin_start_point;
                        int spacing = 35;
                    
                        for (int i = 0; i < maxVisible; i++) {
                            DrawCircle(coreX, baseY + i * spacing, PIN_RADIUS, BLACK);
                    
                            char numText[3];
                            sprintf(numText, "%d", remaining_pins - i);
                            int textWidth = MeasureText(numText, 15);
                            DrawText(numText, coreX - textWidth / 2, baseY + i * spacing - 8, 15, WHITE);
                        }
                    }
                }

                sprintf(pinCountText, "Pins: %d", level_pin - pinCount);
                DrawText(pinCountText, 20, screenHeight - 50, 20, BLACK);

                sprintf(pinCountText, "%d", current_level);
                DrawText(pinCountText, coreX-15, coreY-40, 60, WHITE);
                sprintf(pinCountText, "Level");
                DrawText(pinCountText, coreX-25, coreY+10, 20, WHITE);
            } break;

            case level_end: {

                if(music_on == true){
                    UpdateMusicStream(game_music);
                }

                if (current_level >= highest_level_reached) {
                    highest_level_reached = current_level + 1;
                }

                DrawText("Level Passed!", 130, 250, 30, RED);

                int btnWidth = 120;
                int btnHeight = 40;
                int btnX = (screenWidth - btnWidth) / 2;
                int btnY = 320;

                //Next Level Button
                DrawRectangle(btnX, btnY, btnWidth, btnHeight, DARKGRAY);
                DrawText("Next Level", btnX + 20, btnY + 10, 20, WHITE);

                //Retry Button
                int retryBtnY = 370;
                DrawRectangle(btnX, retryBtnY, btnWidth, btnHeight, DARKGRAY);
                DrawText("Menu", btnX + 30, retryBtnY + 10, 20, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= btnY && mouse.y <= btnY + btnHeight) {
                            if(sound_on == true) UpdateMusicStream(beep_sound);
                            current_level++;
                            current_scene = game;
                            level_initialized = false;
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= retryBtnY && mouse.y <= retryBtnY + btnHeight) {
                        ResetGame(pins, &pinCount, &gameOver);
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        current_scene = main_menu;
                    }
                }
            } break;

            case fail: {
                DrawText("GAME OVER!", 130, 200, 30, RED);

                int btnWidth = 120;
                int btnHeight = 40;
                int btnX = (screenWidth - btnWidth) / 2;
                

                int retryBtnY = 270;
                DrawRectangle(btnX, retryBtnY, btnWidth, btnHeight, DARKGRAY);
                DrawText("Retry", btnX + 30, retryBtnY + 10, 20, WHITE);

                int exitBtnY = 320;
                DrawRectangle(btnX, exitBtnY, btnWidth, btnHeight, DARKGRAY);
                DrawText("EXIT", btnX + 35, exitBtnY + 10, 20, WHITE);

                int menuyBtnY = 370;
                DrawRectangle(btnX, menuyBtnY, btnWidth, btnHeight, DARKGRAY);
                DrawText("Menu", btnX + 30, menuyBtnY + 10, 20, WHITE);

                

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= retryBtnY && mouse.y <= retryBtnY + btnHeight) {
                        ResetGame(pins, &pinCount, &gameOver);
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        level_initialized = false;
                        current_scene = game;
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= exitBtnY && mouse.y <= exitBtnY + btnHeight) {
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        CloseWindow();
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= menuyBtnY && mouse.y <= menuyBtnY + btnHeight) {
                        if(sound_on == true) UpdateMusicStream(beep_sound);
                        ResetGame(pins, &pinCount, &gameOver);
                        current_scene = main_menu;
                    }
                }
                failTimer = 0.0f;
                failTriggered = false;
            } break;

            default:
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
