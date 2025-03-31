#pragma once

#include "vector.h"
#include "mlp.h"
#include "raylib.h"


/** Part of a state machine. Indicates the current state of the program's GUI. */
typedef enum GUISTATE {
    LOADING,        /*!< State where a model is selected to be loaded from a file. */
    DRAWING,        /*!< State where the loaded model's input is set with a drawing board. */
    SIMULATION,     /*!< State where the loaded model's weights, biases and node outputs can be browsed. */
    EXIT            /*!< State where the program should free all of its allocated memory and exit. */
} GUISTATE;


/**
 * Draws the graphical interface for loading MLP's from files.
 * 
 * \param paths List of added loaded models' paths of the disk.
 * \param names List of the added models' file names.
 * \param mlp The MLP which should be overridden upon loading a new model from file.
 * 
 * \returns The state of the GUI to draw on the next frame.
 */
GUISTATE show_load_gui(Vector *paths, Vector *names, MLP *mlp);


/**
 * Draws the graphical interface for drawing.
 * 
 * \param mlp The MLP whose input should be overridden based on the current drawing Canvas.
 * 
 * \returns The state of the GUI to draw on the next frame.
 */
GUISTATE show_draw_gui(MLP *mlp);


/**
 * Draws the graphical interface for exploring and MLP's graph.
 * 
 * \param mlp The MLP that should be shown.
 * \param camera The Camera2D to base calculations on.
 * 
 * \returns The state of the GUI to draw on the next frame.
 */
GUISTATE show_simulation_gui(MLP *mlp, Camera2D *camera);


/**
 * Frees the two lists used by the "load GUI".
 * 
 * \param paths Pointer to the Vector that contains the file paths.
 * \param names Pointer to the Vector that contains the model names.
 */
void free_loaded_mlp_vector(Vector *paths, Vector *names);


/**
 * Frees the memory allocated by the file dialog.
 */
void free_file_dialog();
