#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define MAX_PINS 20
#define CORE_RADIUS 40
#define PIN_RADIUS 7
#define PIN_DISTANCE 60
#define PIN_SPEED 6.0f
#define COLLISION_THRESHOLD 12.0f

typedef enum scene{menu, game, end, levels_menu, settings, levels}scene;
scene current_scene = menu;

typedef struct {
    float angle;
    float yOffset;
    bool attached;
} Pin;

void ResetGame(Pin pins[MAX_PINS], int *pinCount, bool *gameOver) {
    *pinCount = 0;
    *gameOver = false;
    for (int i = 0; i < MAX_PINS; i++) {
        pins[i].attached = false;
    }
}


int main() {
    const int screenWidth = 400;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "aa Game in Raylib");

    Pin pins[MAX_PINS] = {0};
    int pinCount = 0;
    bool gameOver = false;
    char pinCountText[10];
    

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Game logic
        if(current_scene == game){
            if (!gameOver) {
            // Shoot pins
            if (IsKeyPressed(KEY_SPACE) && pinCount < MAX_PINS) {
                pins[pinCount].angle = 90;
                pins[pinCount].yOffset = screenHeight - 300;
                pins[pinCount].attached = false;
                pinCount++;
            }

            // Update pins & collisions
            for (int i = 0; i < pinCount; i++) {
                    if (!pins[i].attached) {
                        // Move the pin upwards toward the core
                        pins[i].yOffset -= PIN_SPEED;

                        // Once the pin reaches the core, attach it
                        if (pins[i].yOffset <= 0) {
                            pins[i].yOffset = 0;
                            pins[i].attached = true;
                        }
                    } else {
                        // Rotate attached pins with the core
                        pins[i].angle += 1.0f;
                        if (pins[i].angle >= 360.0f) pins[i].angle -= 360.0f;
                    }

                    // Collision detection
                    for (int j = 0; j < i; j++) {
                        if (pins[i].attached && pins[j].attached) {
                            float angleDiff = fabs(fmod(pins[i].angle - pins[j].angle + 360.0f, 360.0f));
                            if (angleDiff < COLLISION_THRESHOLD || angleDiff > (360.0f - COLLISION_THRESHOLD)) {
                                gameOver = true;
                                current_scene = end;
                            }
                        }
                    }
                }
            }
        }
        

        // Draw everything
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (current_scene)
        {
        case menu:
            DrawText(".AA.", screenWidth/2 - 55 , screenHeight/4, 60, BLACK);
            
            int sBtnWidth = 120;
            int sBtnHeight = 40;
            int sBtnX = (screenWidth - sBtnWidth) / 2;
            int sBtnY = 320;

            //Transition to game 
            DrawRectangle(sBtnX, sBtnY, sBtnWidth, sBtnHeight, DARKGRAY);
            DrawText("Start", sBtnX + 35, sBtnY + 10, 20, WHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (mouse.x >= sBtnX && mouse.x <= sBtnX + sBtnWidth &&
                    mouse.y >= sBtnY && mouse.y <= sBtnY + sBtnHeight) {
                    current_scene = game;
                }
            }

            //Transition to levels_menu
            DrawRectangle(sBtnX, sBtnY + 70, sBtnWidth, sBtnHeight, DARKGRAY);
            DrawText("Levels", sBtnX + 35, sBtnY + 80, 20, WHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (mouse.x >= sBtnX && mouse.x <= sBtnX + sBtnWidth &&
                    mouse.y >= sBtnY + 70 && mouse.y <= sBtnY+70 + sBtnHeight) {
                    current_scene = levels_menu;
                }
            }

            break;
        case game:
            // Core position
            int coreX = screenWidth / 2;
            int coreY = screenHeight / 3;

            // Draw core
            DrawCircle(coreX, coreY, CORE_RADIUS, BLACK);

            // Draw pins (moving & attached)
            for (int i = 0; i < pinCount; i++) {
                if (!pins[i].attached) {
                    // Moving pin (coming from bottom)
                    DrawCircle(coreX, coreY + pins[i].yOffset, PIN_RADIUS, BLACK);
                } else {
                    // Attached pins rotate with the core
                    float rad = (pins[i].angle) * DEG2RAD;
                    float pinX = coreX + cos(rad) * PIN_DISTANCE;
                    float pinY = coreY + sin(rad) * PIN_DISTANCE;

                    // Draw line from core to attached pin
                    DrawLine(coreX, coreY, pinX, pinY, BLACK);

                    // Draw attached pin
                    DrawCircle(pinX, pinY, PIN_RADIUS, RED);
                }
            }

            // Draw one pin below the core (next to be shot)
            if (pinCount < MAX_PINS) {
                DrawCircle(coreX, screenHeight - 100, PIN_RADIUS, BLACK);
            }

            // Display remaining pin count in the bottom-left corner
            sprintf(pinCountText, "Pins: %d", MAX_PINS - pinCount);
            DrawText(pinCountText, 20, screenHeight - 50, 20, BLACK);
                break;

        /*case levels_menu:
            DrawRectangle(sBtnX, sBtnY, sBtnWidth, sBtnHeight, BLACK);
             
            // Menu button
             int retryBtnY = 370;
             DrawRectangle(sBtnX, retryBtnY, sBtnWidth, sBtnHeight, DARKGRAY);
             DrawText("Back", sBtnX + 30, retryBtnY + 10, 20, WHITE);

             if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();

                if (mouse.x >= sBtnX && mouse.x <= sBtnX + sBtnWidth &&
                    mouse.y >= retryBtnY && mouse.y <= retryBtnY + sBtnHeight) {
                    ResetGame(pins, &pinCount, &gameOver);
                    current_scene = menu;
                }
            }
                */
        break;
        case end:
            DrawText("GAME OVER!", 130, 250, 30, RED);

            // Exit button
            int btnWidth = 120;
            int btnHeight = 40;
            int btnX = (screenWidth - btnWidth) / 2;
            int btnY = 320;

            DrawRectangle(btnX, btnY, btnWidth, btnHeight, DARKGRAY);
            DrawText("EXIT", btnX + 35, btnY + 10, 20, WHITE);

            // Menu button
            int retryBtnY = 370;
            DrawRectangle(btnX, retryBtnY, btnWidth, btnHeight, DARKGRAY);
            DrawText("Menu", btnX + 30, retryBtnY + 10, 20, WHITE);

            // Detect Exit button click
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                    mouse.y >= btnY && mouse.y <= btnY + btnHeight) {
                    CloseWindow(); // Exit the game
                }

                // Detect Retry button click
                if (mouse.x >= btnX && mouse.x <= btnX + btnWidth &&
                    mouse.y >= retryBtnY && mouse.y <= retryBtnY + btnHeight) {
                    ResetGame(pins, &pinCount, &gameOver);
                    current_scene = menu;
                }
            }
            break;
        default:
            break;
        }
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}