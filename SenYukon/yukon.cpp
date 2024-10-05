#include <raylib.h>

#include "state.h"

void play() {
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
