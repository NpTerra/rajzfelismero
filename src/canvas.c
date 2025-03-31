#include "debugmalloc.h"
#include "canvas.h"

#include "snippets.h"


Canvas create_canvas(size_t width, size_t height)
{
    double d = 0;
    Vector x = create_vector(width, sizeof(Vector), false);
    for(size_t i = 0; i < width; i++)
    {
        Vector y = create_vector(height, sizeof(double), false);
        for(size_t j = 0; j < height; j++)
            push_vector(&y, &d);
        push_vector(&x, &y);
    }

    return (Canvas){width, height, x};
}


void free_canvas(Canvas *canvas)
{
    for(size_t i = 0; i < canvas->m.size; i++)
    {
        Vector *y = &get_vector_as_type(&canvas->m, i, Vector);
        free_vector(y);
    }

    free_vector(&canvas->m);
}


double get_canvas_xy(const Canvas *canvas, size_t x, size_t y)
{
    return get_vector_as_type(&get_vector_as_type(&canvas->m, x, Vector), y, double);
}


void set_canvas_xy(Canvas *canvas, size_t x, size_t y, double n)
{
    get_vector_as_type(&get_vector_as_type(&(canvas->m), x, Vector), y, double) = n;
}


void clear_canvas(Canvas *canvas)
{
    for(size_t x = 0; x < canvas->width; x++)
    {
        for(size_t y = 0; y < canvas->height; y++)
        {
            set_canvas_xy(canvas, x, y, 0);
        }
    }
}
