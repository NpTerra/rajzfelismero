#include "debugmalloc.h"
#include "gui.h"
#include <stdbool.h>
#include <math.h>

#include "vector.h"
#include "mlp.h"
#include "filehandler.h"
#include "snippets.h"

#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"


static bool dialog_ready = false;
static GuiWindowFileDialogState file_dialog;

static char *message = NULL;
static char *title = NULL;


/**
 * Creates a Rectangle with a given width and height that's centered on the screen.
 * 
 * \param width The width of the Rectangle.
 * \param height The height of the Rectangle.
 * 
 * \returns A Rectangle with the calculated dimensions.
 */
static Rectangle center_box(int width, int height)
{
    return (Rectangle){ GetScreenWidth()/2.0 - width/2.0, GetScreenHeight()/2.0 - height/2.0, width, height };
}


GUISTATE show_load_gui(Vector *paths, Vector *names, MLP *mlp)
{
    static int scrollindex;
    static int active = -1;
    static int focus;

    if(!dialog_ready)
    {
        file_dialog = InitGuiWindowFileDialog(GetWorkingDirectory(), 700, 470);
        dialog_ready = true;
    }

    if(message != NULL || file_dialog.windowActive)
        GuiDisable();
    
    GuiGroupBox((Rectangle) {GetScreenWidth()-320, (GetScreenHeight()-400)/2.0, 300, 400}, "Usage:");

    GuiLabel((Rectangle) {GetScreenWidth()-310, (GetScreenHeight()-400)/2.0 + 10, 290, 40}, "1.\nUse the \"ADD\" button and the file dialog to add\na model to the list.");
    GuiLabel((Rectangle) {GetScreenWidth()-310, (GetScreenHeight()-400)/2.0 + 70, 290, 40}, "2.\nUse the \"REMOVE\" button to remove a selected\nmodel from the list.");
    GuiLabel((Rectangle) {GetScreenWidth()-310, (GetScreenHeight()-400)/2.0 + 130, 290, 40}, "3.\nUse the \"LOAD\" button to load the selected\nmodel and switch to drawing mode.");

    GuiListViewEx((Rectangle) {20, (GetScreenHeight()-400)/2.0 , 300, 400}, (const char**) names->arr, names->size, &scrollindex, &active, &focus);

    if(GuiButton((Rectangle) {340, (GetScreenHeight()-400)/2.0, 90, 40}, "ADD"))
    {
        TraceLog(LOG_INFO, TextFormat("%d", active));
        file_dialog.windowActive = true;
    }


    if(active == -1)
        GuiDisable();
    if(GuiButton((Rectangle) {340, (GetScreenHeight()-400)/2.0 + 60, 90, 40}, "DELETE"))
    {
        char *p = get_vector_as_type(paths, active, char*);
        char *n = get_vector_as_type(names, active, char*);
        free(p);
        free(n);
        erase_vector(paths, active);
        erase_vector(names, active);
        active = -1;
    }
    if(GuiButton((Rectangle) {340, (GetScreenHeight()+400)/2.0 - 40, 90, 40}, "LOAD"))
    {
        ReadResult read = read_model(get_vector_as_type(paths, active, char*), get_vector_as_type(names, active, char*));
        title = "Error";
        switch(read.status)
        {
            case NOFILE:
                message = "File not found!";
                break;
            case NODATA:
                message = "Couldn't read enough data from the file!";
                break;
            case KERNELSIZE:
                message = "The MaxPool2D kernel size is\nincompatible with the given Canvas!";
                break;
            case NOLAYER:
                message = "There wasn't enough layer instructions\nto build the model!";
                break;
            case WRONGINSTRUCTION:
                message = "Invalid instruction found!";
                break;
            default:
                free_mlp(mlp);
                *mlp = read.model;
                return DRAWING;
        }
    }
    if(active == -1)
        GuiEnable();


    if(file_dialog.windowActive && message == NULL)
        GuiEnable();

    GuiWindowFileDialog(&file_dialog);

    if (file_dialog.SelectFilePressed)
    {
        if (IsFileExtension(file_dialog.fileNameText, ".mlpmodel"))
        {
            char *p = strclone(TextFormat("%s" PATH_SEPERATOR "%s", file_dialog.dirPathText, file_dialog.fileNameText));
            char *n = strclone(GetFileNameWithoutExt(file_dialog.fileNameText));

            bool contains = false;
            for(size_t i = 0; i < paths->size; i++)
            {
                if(strcmp(p, get_vector_as_type(paths, i, char*)) == 0)
                {
                    contains = true;
                    break;
                }
            }

            if(contains)
            {
                free(p);
                free(n);
            }
            else
            {
                push_vector(paths, &p);
                push_vector(names, &n);
            }

            TraceLog(LOG_INFO, p);
        }
        else
        {
            title = "Warning";
            message = "Only files with the .mlpmodel extension are supported!";
        }

        file_dialog.SelectFilePressed = false;
    }

    if(message != NULL)
    {
        GuiUnlock();
        GuiEnable();
        int a = GuiMessageBox(center_box(350, 150), title, message, "OK");
        
        if(a != -1)
        {
            message = NULL;
        }
    }

    return IsKeyPressed(KEY_ESCAPE) ? EXIT : LOADING;
}


typedef enum TOOL {
    BRUSH,
    PENCIL
} TOOL;

typedef enum DRAWMODE {
    NORMAL,
    PREVIEW,
    PREVIEW_UNDO
} DRAWMODE;


/**
 * Draws the brush at the cursor's current position with a given radius.
 * 
 * \param mlp Pointer to the MLP that contains the two Canvases used by this function.
 * \param pos The cursor's position on the Canvas.
 * \param tool BRUSH or PENCIL
 * \param eraser Is the eraser enabled?
 * \param radius The radius of the brush.
 * \param undo If true, it resets the current values on the Canvas to their previous value.
 */
static void draw_brush(MLP *mlp, Vector2 *pos, TOOL tool, bool eraser, int radius, bool undo)
{
    for(size_t i = pos->x-radius+1; i != pos->x+radius; i++)
    {
        if(i >= mlp->x) continue;

        for(size_t j = pos->y-radius+1; j != pos->y+radius; j++)
        {
            if(j >= mlp->y) continue;

            double dist = distance(pos->x, pos->y, i, j);
            if(dist > radius)
                continue;

            double prev = get_canvas_xy(&mlp->canvas, i, j);
            double curr = get_canvas_xy(&mlp->draw_canvas, i, j);
            double change = 255 * ((radius-dist)/radius);

            curr = undo ? prev : (tool == BRUSH) ? (eraser ? max(min(prev-change, curr), 0) : min(max(prev+change, curr), 255)) : (eraser ? 0 : 255);
            set_canvas_xy(&mlp->draw_canvas, i, j, curr);
        }
    }
}


/**
 * Draws the drawing board on the screen and handles its functionality.
 * 
 * \param mlp Pointer to the MLP that contains the Canvas.
 * \param mouse The cursor's current position on the Canvas.
 * \param prevmouse The cursor's previous position on the Canvas.
 * \param toolbox An anchor point for the GUI.
 * \param tool The current tool. BRUSH or PENCIL
 * \param eraser Is the eraser enabled?
 * \param radius The radius of the current brush.
 */
static void draw_board_grid(MLP *mlp, Vector2 *mouse, Vector2 *prevmouse, Vector2 toolbox, TOOL tool, bool eraser, int radius)
{
    int cellsize = 400/max(mlp->x, mlp->y);
    int offset_x = (400 - cellsize*mlp->x)/2.0;
    int offset_y = (400 - cellsize*mlp->y)/2.0;
    
    GuiGrid((Rectangle) {toolbox.x-410+offset_x, toolbox.y+offset_y, mlp->x*cellsize, mlp->y*cellsize},
            "", cellsize, 1, mouse, GuiGetStyle(DEFAULT, cellsize > 4 ? LINE_COLOR : BACKGROUND_COLOR));

    if(!Vector2Equals(*mouse, (Vector2) {-1, -1}))
        draw_brush(mlp, mouse, tool, eraser, radius, false);

    Canvas *canv = &(mlp->draw_canvas);
    for(size_t i = 0; i < mlp->x; i++)
    {
        for(size_t j = 0; j < mlp->y; j++)
        {
            double d = (int) get_canvas_xy(canv, i, j);
            DrawRectangle(toolbox.x-410+offset_x + i*cellsize, toolbox.y+offset_y + j*cellsize, cellsize, cellsize,
                (Color) {0, 0, 0, d});
        }
    }

    if(!Vector2Equals(*mouse, (Vector2) {-1, -1}))
    {
        if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            draw_brush(mlp, mouse, PENCIL, true, radius, true);
        
        DrawCircleLines(toolbox.x-410+offset_x + mouse->x*cellsize + cellsize/2.0, toolbox.y+offset_y + mouse->y*cellsize + cellsize/2.0, (radius-1)*cellsize, RED);
    }
}


GUISTATE show_draw_gui(MLP *mlp)
{
    static TOOL tool = BRUSH;
    static bool eraser = false;
    static Vector2 prevmouse = {-1, -1};
    static Vector2 mouse = {-1, -1};

    static int bsize_int = -1;
    static float bsize_float = -1;
    static bool bsize_edit = false;
    if(bsize_int == -1)
    {
        bsize_int = max(mlp->x, mlp->y)/14;
        bsize_float = bsize_int;
    }
    
    Vector2 toolbox = {GetScreenWidth()/2.0-47, GetScreenHeight()/2.0-200};


    // ---------------
    //  DRAWING BOARD
    // ---------------
    GuiGroupBox((Rectangle) {toolbox.x-410, toolbox.y, 400, 400}, "Drawing Board");
    draw_board_grid(mlp, &mouse, &prevmouse, toolbox, tool, eraser, bsize_int);

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        prevmouse = (Vector2) {-1, -1};
    }

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        Canvas *canv1 = &(mlp->canvas);
        Canvas *canv2 = &(mlp->draw_canvas);

        for(size_t i = 0; i < mlp->x; i++)
            for(size_t j = 0; j < mlp->y; j++)
                set_canvas_xy(canv1, i, j, get_canvas_xy(canv2, i, j));
    }
    
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !Vector2Equals(mouse, (Vector2) {-1, -1}) && !Vector2Equals(mouse, prevmouse))
    {
        prevmouse = mouse;
        load_mlp_input(mlp);
        run_mlp(mlp);

        write_model_result(mlp, CONSOLE);
    }


    // ------------
    //  MODEL INFO
    // ------------
    GuiGroupBox((Rectangle) {toolbox.x+215, toolbox.y, 200, 90}, "Model Info");
    GuiLabel((Rectangle) {toolbox.x+225, toolbox.y+10, 200, 10}, TextFormat("Name: \'%s\'", mlp->name));
    GuiLabel((Rectangle) {toolbox.x+225, toolbox.y+10+15, 200, 10}, TextFormat("Canvas size: %zux%zu", mlp->x, mlp->y));
    GuiLabel((Rectangle) {toolbox.x+225, toolbox.y+10+30, 200, 10}, TextFormat("MaxPool2D kernel size: %zux%zu", mlp->kx, mlp->ky));
    GuiLabel((Rectangle) {toolbox.x+225, toolbox.y+10+45, 200, 10}, TextFormat("Layer count: %zu", mlp->layers.size));
    double perc = get_vector_as_type(&get_vector_as_type(&mlp->layers, mlp->layers.size-1, Vector), mlp->result, Node).output;
    GuiLabel((Rectangle) {toolbox.x+225, toolbox.y+10+60, 200, 10}, TextFormat("Result: %zu (%.2lf%%)", mlp->result, perc*100));


    // --------------
    //  DRAWING TOOL
    // --------------
    GuiGroupBox((Rectangle) {toolbox.x, toolbox.y, 95, 160}, TextFormat("Tool:     %s", eraser ? "     " : ""));
    GuiLabel((Rectangle) {toolbox.x+41, toolbox.y-15, 30, 30}, tool == BRUSH ? "#24#" : "#23#");
    if(eraser)
    {
        GuiLabel((Rectangle) {toolbox.x+56, toolbox.y-15, 30, 30}, "+");
        GuiLabel((Rectangle) {toolbox.x+61, toolbox.y-15, 30, 30}, "#28#");
    }

    // Brush toggle
    if(tool == BRUSH) GuiDisable();
    if(GuiButton((Rectangle) {toolbox.x+10, toolbox.y+15, 75, 30}, "#24#Brush"))
        tool = BRUSH;
    if(tool == BRUSH) GuiEnable();

    // Pencil toggle
    if(tool == PENCIL) GuiDisable();
    if(GuiButton((Rectangle) {toolbox.x+10, toolbox.y+55, 75, 30}, "#23#Pencil"))
        tool = PENCIL;
    if(tool == PENCIL) GuiEnable();
    
    // Eraser toggle
    GuiCheckBox((Rectangle) {toolbox.x+10, toolbox.y+95, 15, 15}, "#28#Eraser", &eraser);

    // Clear button
    if(GuiButton((Rectangle) {toolbox.x+10, toolbox.y+170, 75, 30}, "Clear")
        || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        clear_canvas(&mlp->canvas);
        clear_canvas(&mlp->draw_canvas);
    }

    // Brush size control
    if(GuiSliderBar((Rectangle) {toolbox.x+10, toolbox.y+125, 75, 7}, "", "", &bsize_float, 1, max(mlp->x, mlp->y)/2.0))
    {
        bsize_float = roundf(bsize_float);
        bsize_int = bsize_float;
    }
    if(GuiValueBox((Rectangle) {toolbox.x+10+20+2, toolbox.y+135, 31, 15},"", &bsize_int, 1, max(mlp->x, mlp->y)/2.0, bsize_edit))
    {
        bsize_edit = !bsize_edit;
        bsize_float = bsize_int;
    }
    if(GuiButton((Rectangle) {toolbox.x+10, toolbox.y+135, 20, 15}, "-") && bsize_int > 1)
    {
        bsize_int--;
        bsize_float = bsize_int;
    }
    if(GuiButton((Rectangle) {toolbox.x+10+20+35, toolbox.y+135, 20, 15}, "+") && bsize_int < max(mlp->x, mlp->y)/2.0)
    {
        bsize_int++;
        bsize_float = bsize_int;
    }


    // --------------
    //  STATE SWITCH
    // --------------
    if(GuiButton((Rectangle) {toolbox.x+10, toolbox.y+370, 75, 30}, "Explore"))
        return SIMULATION;

    return IsKeyPressed(KEY_ESCAPE) ? LOADING : DRAWING;
}


/**
 * Checks if a position is visible by a given Camera2D.
 * 
 * \param pos The position.
 * \param camera The camera.
 * 
 * \returns False if the position is off-screen, true if it's visible.
 */
static bool visible(Vector2 pos, Camera2D camera)
{
    Vector2 scr = GetWorldToScreen2D(pos, camera);

    return scr.x >= 0 && scr.x <= GetScreenWidth() && scr.y >= 0 && scr.y <= GetScreenHeight();
}


/**
 * Draws a Node's information on screen at the cursor.
 * Should only be called when Mode2D is active in raylib.
 * 
 * \param n The Node whose information should be shown.
 * \param camera The Camera2D to use for re-enabling Mode2D.
 */
static void draw_node_info(Node n, Camera2D camera)
{
    Vector2 pos = GetMousePosition();
    Vector2 size = {120, 50};

    EndMode2D();

    DrawRectangle(pos.x, pos.y-size.y, size.x, size.y, WHITE);
    DrawRectangleLines(pos.x, pos.y-size.y, size.x, size.y, SKYBLUE);

    GuiLabel((Rectangle) {pos.x+10, pos.y-size.y+10, size.x, 10}, TextFormat("Value: %lf", n.value));
    GuiLabel((Rectangle) {pos.x+10, pos.y-size.y+20, size.x, 10}, TextFormat("Bias: %lf", n.bias));
    GuiLabel((Rectangle) {pos.x+10, pos.y-size.y+30, size.x, 10}, TextFormat("Output: %lf", n.output));

    BeginMode2D(camera);
}


/**
 * Draws a layer and its connections to a target on-screen.
 * 
 * \param layer Pointer to the layer to draw.
 * \param target Pointer to the target layer. Can be NULL if no target is selected.
 * \param target_index The target Node's index inside the target layer. Irrelevant if the target layer is NULL.
 * \param center A center point for calculating coordinates on the 2D plane.
 * \param offset The offset on the X axis to draw the current layer at.
 * \param camera Pointer to the current camera.
 * \param mouse The cursor's current position on the 2D plane. Not to be confused with the cursor's position on the screen.
 */
static void draw_layer(Vector *layer, Vector2 *target, size_t target_index, Vector2 center, int offset, Camera2D *camera, Vector2 mouse)
{
    for(long long j = 0; j < layer->size; j++)
    {
        Vector2 circle = {center.x + offset, center.y + (j-layer->size/2.0)*100};
        
        if(target != NULL)
            DrawLine(circle.x, circle.y, target->x, target->y, RED);
        
        if(visible(circle, *camera))
        {
            DrawCircle(circle.x, circle.y, 30, BLUE);
            if(target != NULL)
            {
                Node *node = &get_vector_as_type(layer, j, Node);
                double weight = get_vector_as_type(&node->con, target_index, double);

                const char *str = TextFormat("%.4lf", weight);
                Font f = GetFontDefault();
                Vector2 strsize = MeasureTextEx(f, str, 10, 2);

                Vector2 box = {circle.x - strsize.x/2.0, circle.y - strsize.y/2.0};
                DrawRectangle(box.x-4, box.y-2, strsize.x+8, strsize.y+3, DARKBLUE);
                DrawTextEx(GetFontDefault(), str, box, 10, 2, RAYWHITE);
            }
        }
        
        if(CheckCollisionPointCircle(mouse, circle, 30))
            draw_node_info(get_vector_as_type(layer, j, Node), *camera);
    }
}


/**
 * Draws a layer's output as percentages in a Rectangle.
 * The output values should be probabilities between 0 and 1.
 * 
 * \param layer Pointer to the target layer.
 * \param offset The offset on the X axis on which the layer was drawn.
 * \param center A center point for calculating coordinates on the 2D plane.
 */
static void draw_output(Vector *layer, double offset, Vector2 center)
{
    Vector2 top = {center.x + offset + 60, center.y - ((layer->size+2)/2.0) * 100};
    DrawRectangle(top.x, top.y, 200, center.y + (layer->size - layer->size/2.0) * 100 - top.y, RAYWHITE);
    DrawRectangleLinesEx((Rectangle) {top.x, top.y, 200, center.y + (layer->size - layer->size/2.0) * 100 - top.y}, 5, SKYBLUE);
    for(long long i = 0; i < layer->size; i++)
    {
        Node *node = &get_vector_as_type(layer, i, Node);
        const char *str = TextFormat("%llu: %.2lf%%", i, node->output*100);
        Vector2 strsize = MeasureTextEx(GetFontDefault(), str, 30, 3);

        Vector2 circle = {center.x + offset + 100, center.y + (i-layer->size/2.0)*100 - strsize.y/2.0};
        DrawTextEx(GetFontDefault(), str, circle, 30, 3, BLACK);
    }
}


GUISTATE show_simulation_gui(MLP *mlp, Camera2D *camera)
{
    static Vector2 screen_center = {-1, -1};
    static Vector2 center = {-1, -1};

    static Vector2 click = {-1, -1};

    static Color filler = (Color) {230, 230, 230, 255};
    
    if(Vector2Equals(center, (Vector2) {-1, -1}))
    {
        screen_center = (Vector2) {GetScreenWidth()/2.0, GetScreenHeight()/2.0};
        center = GetScreenToWorld2D(screen_center, *camera);
    }

    /**
     * Camera movement and zoom code is from:
     * https://github.com/raysan5/raylib/blob/master/examples/core/core_2d_camera_mouse_zoom.c
     */
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f / camera->zoom);
        camera->target = Vector2Add(camera->target, delta);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0)
    {
        // Get the world point that is under the mouse
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

        // Set the offset to where the mouse is
        camera->offset = GetMousePosition();

        // Set the target to match, so that the camera maps the world space point 
        // under the cursor to the screen space point under the cursor at any zoom
        camera->target = mouseWorldPos;

        // Zoom increment
        float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
        if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
        camera->zoom = Clamp(camera->zoom*scaleFactor, 0.025f, 2.0f);
    }
    /**
     * End of camera movement code
     */

    Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), *camera);

    BeginMode2D(*camera);

    // Center point
    DrawCircle(center.x, center.y, 5, RED);
    double prevoffset = 0;

    Vector *first = &get_vector_as_type(&mlp->layers, 0, Vector);
    Vector *prev = first;
    for(long long i = 1; i < mlp->layers.size; i++)
    {
        Vector *layer = &get_vector_as_type(&mlp->layers, i, Vector);
        double offset = prevoffset + sqrt(exp(log2(prev->size)))*100;
        
        Vector2 poly[] = {
            (Vector2) {center.x + prevoffset, center.y - (prev->size/2.0)*100},
            (Vector2) {center.x + prevoffset, center.y + (prev->size-1-prev->size/2.0)*100},
            (Vector2) {center.x + offset, center.y + (layer->size-1-layer->size/2.0)*100},
            (Vector2) {center.x + offset, center.y - (layer->size/2.0)*100}
        };

        DrawTriangleFan(poly, 4, filler);
        DrawLineEx(poly[0], poly[3], 5, BLUE);
        DrawLineEx(poly[1], poly[2], 5, BLUE);
        
        size_t target_node = 0;

        for(long long j = 0; j < layer->size; j++)
        {
            Vector2 circle = (Vector2) {center.x + offset, center.y + (j-layer->size/2.0)*100};

            if(CheckCollisionPointCircle(mouse, circle, 30))    
            {
                draw_node_info(get_vector_as_type(layer, j, Node), *camera);

                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    click = Vector2Equals(click, circle) ? (Vector2) {-1, -1} : circle;
            }

            if(Vector2Equals(click, circle))
                target_node = (size_t) j;
        }

        
        draw_layer(prev, ((int)click.x == (int)(center.x+offset)) ? &click : NULL, target_node, center, prevoffset, camera, mouse);

        prev = layer;
        prevoffset = offset;
    }

    draw_output(prev, prevoffset, center);
    draw_layer(prev, NULL, 0, center, prevoffset, camera, mouse);

    EndMode2D();

    if(GuiButton((Rectangle) {GetScreenWidth()-100, GetScreenHeight()-100, 90, 40}, "Save result"))
        write_model_result(mlp, DISK);
    
    if(GuiButton((Rectangle) {GetScreenWidth()-100, GetScreenHeight()-50, 90, 40}, "Reset camera"))
        *camera = (Camera2D) {{0, 0}, {0, 0}, 0, 1.0f};

    return IsKeyPressed(KEY_ESCAPE) ? DRAWING : SIMULATION;
}

void free_loaded_mlp_vector(Vector *paths, Vector *names)
{
    for(int i = 0; i < names->size; i++)
    {
        char *p = get_vector_as_type(paths, i, char*);
        char *n = get_vector_as_type(names, i, char*);
        free(p);
        free(n);
    }
    free_vector(paths);
    free_vector(names);
}


void free_file_dialog()
{
    if(dialog_ready)
        FreeDialog(&file_dialog);
}
