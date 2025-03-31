#include "debugmalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"

#include "vector.h"
#include "mlp.h"
#include "filehandler.h"
#include "snippets.h"
#include "gui.h"

#define WIDTH 1000
#define HEIGHT 600
#define APP_NAME "Rajzfelismerő"


int main(){
    InitWindow(WIDTH, HEIGHT, APP_NAME);
    
    SetTargetFPS( GetMonitorRefreshRate( GetCurrentMonitor() ) );

    GUISTATE state = LOADING;

    MLP mlp;
    mlp.name = NULL;
    Vector paths = create_vector(1, sizeof(char*), false);
    Vector names = create_vector(1, sizeof(char*), false);

    Camera2D camera = {{0, 0}, {0, 0}, 0, 1.0f};


    bool running = true;

    while(running && (!WindowShouldClose() || IsKeyPressed(KEY_ESCAPE)))
    {
        BeginDrawing();
        ClearBackground(WHITE);

        switch(state)
        {
            case LOADING:
                state = show_load_gui(&paths, &names, &mlp);
                break;
            case DRAWING:
                state = show_draw_gui(&mlp);
                if(state == SIMULATION)
                    camera = (Camera2D) {{0, 0}, {0, 0}, 0, 1.0f};
                break;
            case SIMULATION:
                state = show_simulation_gui(&mlp, &camera);
                break;
            default:
                running = false;
        }
        
        DrawFPS(10, 5);

        EndDrawing();
    }

    
    free_mlp(&mlp);
    free_loaded_mlp_vector(&paths, &names);
    free_file_dialog();

    CloseWindow();
}
