#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define MAX_PINS 250
#define CORE_RADIUS 50
#define PIN_RADIUS 7
#define PIN_DISTANCE 65
#define PIN_SPEED 6.0f
#define COLLISION_THRESHOLD 12.0f

typedef enum scene { menu, game, fail, level_end, levels_menu, settings } scene;

scene current_scene = menu;

typedef struct {
    float angle;
    float yOffset;
    bool attached;
    bool collided;
} Pin;

void ResetGame(Pin pins[MAX_PINS], int *pinCount, bool *gameOver) {
    *pinCount = 0;
    *gameOver = false;
    for (int i = 0; i < MAX_PINS; i++) {
        pins[i].attached = false;
        pins[i].collided = false;
    }
}

void setLevel(int level, int *level_pin) {
    if (level < 1) level = 1;
    if (level > 15) level = 15;
    *level_pin = 3 + (level - 1) * 2;
}

int main() {
    const int screenWidth = 400;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "aa Game in Raylib");

    

    Pin pins[MAX_PINS] = {0};
    
    int pinCount = 0;
    int highest_level_reached = 1;
    int current_level = 1;
    int level_pin = 0;
    float failTimer = 0.0f;

    bool level_initialized = false;
    bool gameOver = false;
    bool failTriggered = false;
    
    char pinCountText[10];
   
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (current_scene == game) {
            if (!level_initialized) {
                setLevel(current_level, &level_pin);
                ResetGame(pins, &pinCount, &gameOver);
                level_initialized = true;
            }

            if (!gameOver) {
                if (IsKeyPressed(KEY_SPACE) && pinCount < level_pin) {
                    pins[pinCount].angle = 90;
                    pins[pinCount].yOffset = screenHeight - 300;
                    pins[pinCount].attached = false;
                    pinCount++;
                }

                for (int i = 0; i < pinCount; i++) {
                    if (!pins[i].attached) {
                        pins[i].yOffset -= PIN_SPEED;
                        if (pins[i].yOffset <= 0) {
                            pins[i].yOffset = 0;
                            pins[i].attached = true;
                        }
                    } else {
                        pins[i].angle += 1.0f;
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

                if (pinCount == level_pin && allAttached) {
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
        ClearBackground(RAYWHITE);

        switch (current_scene) {
            case menu: {
                DrawText(".AA.", screenWidth/2 - 55, screenHeight/4, 60, BLACK);

                int sBtnWidth = 120;
                int sBtnHeight = 40;
                int sBtnX = (screenWidth - sBtnWidth) / 2;
                int sBtnY = 320;

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

                DrawRectangle(sBtnX, sBtnY + 70, sBtnWidth, sBtnHeight, DARKGRAY);
                DrawText("Levels", sBtnX + 35, sBtnY + 80, 20, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= sBtnX && mouse.x <= sBtnX + sBtnWidth &&
                        mouse.y >= sBtnY + 70 && mouse.y <= sBtnY + 70 + sBtnHeight) {
                        current_scene = levels_menu;
                    }
                }
            } break;

            case levels_menu: {
                DrawText("Select Level", screenWidth / 2 - 70, 40, 30, BLACK);
            
                int levelsPerRow = 3;
                int boxSize = 80;
                int spacing = 20;
                int startX = (screenWidth - (levelsPerRow * (boxSize + spacing))) / 2;
                int startY = 100;
            
                for (int i = 1; i <= 15; i++) {
                    int row = (i - 1) / levelsPerRow;
                    int col = (i - 1) % levelsPerRow;
                    int x = startX + col * (boxSize + spacing);
                    int y = startY + row * (boxSize + spacing);
            
                    bool unlocked = i <= highest_level_reached;
                    Color borderColor = unlocked ? GREEN : RED;
                    DrawRectangleLines(x, y, boxSize, boxSize, borderColor);
            
                    char levelLabel[10];
                    sprintf(levelLabel, "Lv %d", i);
                    DrawText(levelLabel, x + 20, y + 30, 20, BLACK);
            
                    if (unlocked && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        Vector2 mouse = GetMousePosition();
                        if (mouse.x >= x && mouse.x <= x + boxSize &&
                            mouse.y >= y && mouse.y <= y + boxSize) {
                            current_level = i;
                            level_initialized = false;
                            current_scene = game;
                        }
                    }
                }
            
                // Back to main menu
                DrawRectangle(10, 10, 100, 40, DARKGRAY);
                DrawText("Menu", 30, 20, 20, WHITE);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (mouse.x >= 10 && mouse.x <= 110 && mouse.y >= 10 && mouse.y <= 50) {
                        current_scene = menu;
                    }
                }
            } break;
            

            case game: {
                int coreX = screenWidth / 2;
                int coreY = screenHeight / 3;

                DrawCircle(coreX, coreY, CORE_RADIUS, BLACK);

                for (int i = 0; i < pinCount; i++) {
                    if (!pins[i].attached) {
                        DrawCircle(coreX, coreY + pins[i].yOffset, PIN_RADIUS, BLACK);
                    } else {
                        float rad = (pins[i].angle) * DEG2RAD;
                        float pinX = coreX + cos(rad) * PIN_DISTANCE;
                        float pinY = coreY + sin(rad) * PIN_DISTANCE;
                        DrawLine(coreX, coreY, pinX, pinY, BLACK);
                        
                        Color pinColor = pins[i].collided ? YELLOW : RED;
                        DrawCircle(pinX, pinY, PIN_RADIUS, pinColor);
                    }
                }

                if (pinCount < level_pin) {
                    DrawCircle(coreX, screenHeight - 100, PIN_RADIUS, BLACK);
                }

                sprintf(pinCountText, "Pins: %d", level_pin - pinCount);
                DrawText(pinCountText, 20, screenHeight - 50, 20, BLACK);

                sprintf(pinCountText, "Level %d", current_level);
                DrawText(pinCountText, screenWidth - 120, screenHeight - 50, 20, DARKGRAY);
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
                            //Is it last level?
                            if (current_level >= 15) {
                                current_scene = menu;
                            } else {
                                current_level++;
                                current_scene = game;
                                level_initialized = false;
                            }
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= retryBtnY && mouse.y <= retryBtnY + btnHeight) {
                        ResetGame(pins, &pinCount, &gameOver);
                        current_scene = menu;
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
                        current_scene = game;
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= exitBtnY && mouse.y <= exitBtnY + btnHeight) {
                        CloseWindow();
                    }

                    if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                        mouse.y >= menuyBtnY && mouse.y <= menuyBtnY + btnHeight) {
                        ResetGame(pins, &pinCount, &gameOver);
                        current_scene = menu;
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
