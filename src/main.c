#include "raylib.h"
#include <math.h>
#include <stdio.h>

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

int main() {
    const int screenWidth = 400;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "aa Game in Raylib");

    Color lightBeige = (Color){ 243, 243, 224, 255 };
    
    Pin pins[MAX_PINS] = {0};
    
    int pinCount = 0;
    int highest_level_reached = 1;
    int current_level = 1;
    int level_pin = 0;
    int pin_start_point = screenHeight - 200;

    float failTimer = 0;
    float rotationSpeed = 0;
    float rotationTimer= 1;

    bool level_initialized = false;
    bool gameOver = false;
    bool failTriggered = false;
    
    char pinCountText[10];
   
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
                        }
                    } else {
                        pins[i].angle += rotationSpeed;
                        if (pins[i].angle >= 360.0f) pins[i].angle -= 360.0f;
                    }

                    for (int j = 0; j < i; j++) {
                        if (pins[i].attached && pins[j].attached) {
                            float angleDiff = fabs(fmod(pins[i].angle - pins[j].angle + 360.0f, 360.0f));
                            if (angleDiff < COLLISION_THRESHOLD || angleDiff > (360.0f - COLLISION_THRESHOLD)) {
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
            ClearBackground(MAROON);
        } else {
            ClearBackground(lightBeige);
        }

        switch (current_scene) {
            
            case main_menu: {
                DrawText(".AA.", screenWidth/2 - 55, screenHeight/4, 60, BLACK);

                int sBtnWidth = 120; int sBtnHeight = 40; int sBtnX = (screenWidth - sBtnWidth) / 2; int sBtnY = 320;
                int menuBtnWidth = 70; int menuBtnHeight = 30; int menuBtnX= 300; int menuBtnY = 30; int menuLetterSize = 10;

                //START Button
                DrawRectangle(sBtnX, sBtnY, sBtnWidth, sBtnHeight, DARKGRAY);
                DrawText("Start", sBtnX + 35, sBtnY + 10, 20, WHITE);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= sBtnX && mouse.x <= sBtnX + sBtnWidth &&
                        mouse.y >= sBtnY && mouse.y <= sBtnY + sBtnHeight) {
                        current_scene = game;
                        current_level = highest_level_reached;
                        level_initialized = false;
                    }
                }
               
                //MENU Button
                DrawRectangle(menuBtnX, menuBtnY, menuBtnWidth, menuBtnHeight, DARKGRAY);
                Vector2 menuTextSize = MeasureTextEx(GetFontDefault(), "MENU", menuLetterSize, 0);
                float MenutextX = menuBtnX + (menuBtnWidth - menuTextSize.x) / 2;
                float MenutextY = menuBtnY + (menuBtnHeight - menuTextSize.y) / 2;
                DrawText("MENU", MenutextX, MenutextY, 10, WHITE);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= menuBtnX && mouse.x <= menuBtnX + menuBtnWidth &&
                        mouse.y >= menuBtnY  && mouse.y <= menuBtnY + menuBtnHeight) {
                        current_scene = menu;
                    }
                }


            } break;
            case menu: {
                //Buttons
                Button soundBtn = {80, 125, 120, 30, 30};
                Button darkModeBtn = {240, 125, 120, 30, 30};
                Button playRoundsBtn = {240, 225, 120, 30, 30};
                Button replayBtn = {80, 225, 120, 30, 10};
                Button backBtn = {360, 10, 30, 30, 30};
            
                //Sound on/off Button
                bool sound = true;
                const char* on_off = sound ? "on" : "off";
            
                DrawRectangle(soundBtn.x, soundBtn.y, soundBtn.width, soundBtn.height, DARKGRAY);
                Vector2 soundTextSize = MeasureTextEx(GetFontDefault(), TextFormat("Sound %s", on_off), soundBtn.letterSize, 0);
                float soundTextX = soundBtn.x + (soundBtn.width - soundTextSize.x) / 2;
                float soundTextY = soundBtn.y + (soundBtn.height - soundTextSize.y) / 2;
                DrawText(TextFormat("Sound %s", on_off), soundTextX, soundTextY, 10, WHITE);
            
                // Dark Mode Button
                DrawRectangle(darkModeBtn.x, darkModeBtn.y, darkModeBtn.width, darkModeBtn.height, DARKGRAY);
                DrawText("Dark Mode", darkModeBtn.x + 5, darkModeBtn.y + 5, 10, WHITE);
            
                // Play Rounds Button
                DrawRectangle(playRoundsBtn.x, playRoundsBtn.y, playRoundsBtn.width, playRoundsBtn.height, DARKGRAY);
                DrawText("Play Rounds", playRoundsBtn.x + 5, playRoundsBtn.y + 5, 10, WHITE);
            
                // Replay Level Button
                DrawRectangle(replayBtn.x, replayBtn.y, replayBtn.width, replayBtn.height, DARKGRAY);
                Vector2 menuTextSize = MeasureTextEx(GetFontDefault(), "Replay Level", replayBtn.letterSize, 0);
                float menuTextX = replayBtn.x + (replayBtn.width - menuTextSize.x) / 2;
                float menuTextY = replayBtn.y + (replayBtn.height - menuTextSize.y) / 2;
                DrawText("Replay Level", menuTextX, menuTextY, 10, WHITE);
            
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= replayBtn.x && mouse.x <= replayBtn.x + replayBtn.width &&
                        mouse.y >= replayBtn.y && mouse.y <= replayBtn.y + replayBtn.height) {
                        current_scene = level_menu;
                    }
                }
            
                //Back To Main Menu Button
                DrawRectangle(backBtn.x, backBtn.y, backBtn.width, backBtn.height, lightBeige);
                Vector2 backTextSize = MeasureTextEx(GetFontDefault(), "x", backBtn.letterSize, 0);
                float backTextX = backBtn.x + (backBtn.width - backTextSize.x) / 2;
                float backTextY = backBtn.y + (backBtn.height - backTextSize.y) / 2;
                DrawText("x", backTextX, backTextY, backBtn.letterSize, BLACK);
            
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= backBtn.x && mouse.x <= backBtn.x + backBtn.width &&
                        mouse.y >= backBtn.y && mouse.y <= backBtn.y + backBtn.height) {
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
                    int row = (i - 1) / boxPerRow;
                    int col = (i - 1) % boxPerRow;
                    int x = startX + col * (boxSize + spacing);
                    int y = startY + row * (boxSize + spacing);

                    if (i == 10) {
                        DrawRectangleLines(x, y, 2 * boxSize + spacing, boxSize, BLACK);
                        DrawText("Clear", x + 15, y + 10, 20, BLACK);
                        i++;
                        continue;
                    }
                    int displayNumber=0;
                    if(i == 12) displayNumber = 0;
                    else displayNumber = i;

                    DrawRectangleLines(x, y, boxSize, boxSize, BLACK);
                    DrawText(TextFormat("%d", displayNumber), x + 15, y + 10, 20, BLACK);
                }
                    DrawRectangleLines(115, 480, 160, 40, BLACK);


            
                DrawRectangle(backX, backY, backWidth, backHeight, lightBeige);
                Vector2 backTextSize = MeasureTextEx(GetFontDefault(), "x", backLetterSize, 0);
                float backtextX = backX + (backWidth - backTextSize.x) / 2;
                float backtextY = backY + (backHeight - backTextSize.y) / 2;
                DrawText("x", backtextX, backtextY, backLetterSize, BLACK);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= backX && mouse.x <= backX + backWidth &&
                        mouse.y >= backY  && mouse.y <= backY + backHeight) {
                        current_scene = menu;
                    }
                }
            } break;
            

            case game: {
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

                if (current_level >= highest_level_reached) {
                    highest_level_reached = current_level + 1;
                }

                DrawText("Level Passed!", 130, 250, 30, RED);

                int btnWidth = 120;
                int btnHeight = 40;
                int btnX = (screenWidth - btnWidth) / 2;
                int btnY = 320;

                DrawRectangle(btnX, btnY, btnWidth, btnHeight, DARKGRAY);
                DrawText("Next Level", btnX + 20, btnY + 10, 20, WHITE);

                int retryBtnY = 370;
                DrawRectangle(btnX, retryBtnY, btnWidth, btnHeight, DARKGRAY);
                DrawText("Menu", btnX + 30, retryBtnY + 10, 20, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= btnY && mouse.y <= btnY + btnHeight) {
                                current_level++;
                                current_scene = game;
                                level_initialized = false;
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= retryBtnY && mouse.y <= retryBtnY + btnHeight) {
                        ResetGame(pins, &pinCount, &gameOver);
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
                        level_initialized = false;
                        current_scene = game;
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= exitBtnY && mouse.y <= exitBtnY + btnHeight) {
                        CloseWindow();
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= menuyBtnY && mouse.y <= menuyBtnY + btnHeight) {
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
