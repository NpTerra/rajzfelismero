#pragma once
#include "vector.h"


/**
 * A Canvas struct contains a 2D matrix of 'double' values.
 */
typedef struct Canvas {
    size_t width, height;
    /**
     * This Vector contains as many other Vectors as the width of the Canvas.
     * These other Vectors represent the rows of the 2D matrix.
     */
    Vector m;
} Canvas;


/**
 * Creates a new Canvas with the given width and height.
 * The returned Canvas should be freed by the caller,
 * as it contains dynamically allocated memory.
 * 
 * \param width The width of the Canvas.
 * \param height The height of the Canvas.
 * 
 * \returns The new Canvas struct.
 */
Canvas create_canvas(size_t width, size_t height);


/**
 * Frees all dynamically allocated memory used by a Canvas.
 * 
 * \param Canvas Pointer to the Canvas that should be freed.
 */
void free_canvas(Canvas *canvas);


/**
 * Queries the Canvas at given coordinates.
 * The (0, 0) coordinate is at the top left corner.
 * 
 * \param Canvas Pointer to the target Canvas.
 * \param x The X coordinate on the Canvas.
 * \param y The Y coordinate on the Canvas.
 * 
 * \returns The greyscale value on the Canvas at the given coordinates.
 */
double get_canvas_xy(const Canvas *canvas, size_t x, size_t y);


/**
 * Overrides the current value at given coordinates on a Canvas.
 * 
 * \param Canvas Pointer to the target Canvas.
 * \param x The X coordinate on the Canvas.
 * \param y The Y coordinate on the Canvas.
 * \param n The new value.
 */
void set_canvas_xy(Canvas *canvas, size_t x, size_t y, double n);


/**
 * Sets all values on a Canvas to zero.
 * 
 * \param Canvas Pointer to the target Canvas.
 */
void clear_canvas(Canvas *canvas);
