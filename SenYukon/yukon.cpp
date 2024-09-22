#include <raylib.h>

#include "state.h"

void play() {
    SetTargetFPS(60);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Sen Yukon");
    SetExitKey(KEY_NULL);

    InitAudioDevice();

    {
        State state;

        while (!WindowShouldClose()) {
            state.handle_input();
            state.update();
            BeginDrawing();
            state.render();
            EndDrawing();
        }
    } // calls destruction

    CloseAudioDevice();
    CloseWindow();
}
