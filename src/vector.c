#include "debugmalloc.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "snippets.h"


/**
 * Terminates the program if an index in a Vector is out of bounds.
 * 
 * \param v Pointer to the target Vector.
 * \param index The element's index.
 */
static void check_index(const Vector *v, size_t index)
{
    if(index >= v->size)
        exit(ERR_INDEXOUTOFBOUNDS);
}


void* get_vector_address(const Vector *v, size_t index)
{
    check_index(v, index);

    return v->arr + (index * v->elem_size);
}


void resize(Vector *v, size_t new_size)
{
    void *p = realloc(v->arr, new_size * v->elem_size);

    if(p == NULL)
        exit(ERR_NULLPOINTER);

    v->arr = p;
    v->cap = new_size;
}


bool scale_up(Vector *v)
{
    // if the vector is at its current capacity
    // resizing is needed
    if(v->size == v->cap)
    {
        size_t s = max((size_t)(v->cap*SCALING), v->cap+1);
        
        resize(v, s);

        return true;
    }

    return false;
}


bool scale_down(Vector *v)
{
    // vector is at its shrinking threshold
    // resizing is needed
    if(v->size == v->cap*SHRINK)
    {
        size_t s = max((size_t) (v->cap*SHRINK*SCALING), 1);
        
        if(s == v->cap)
            return false;
        
        resize(v, s);

        return true;
    }

    return false;
}


Vector create_vector(size_t size, size_t elem_size, bool reset)
{
    Vector v;
    v.arr = reset ? calloc(size, elem_size) : malloc(size * elem_size);
    v.elem_size = elem_size;
    v.cap = size;
    v.size = 0;

    return v;
}


void push_vector(Vector *v, const void *n)
{
    scale_up(v);

    memcpy(v->arr+(v->size*v->elem_size), n, v->elem_size);
    v->size++;
}


void pop_vector(Vector *v)
{
    // return if the vector is empty
    if(v->size == 0) return;

    v->size--;

    scale_down(v);
}


void insert_vector(Vector *v, const void *n, size_t index)
{
    if(index < 0)
        exit(ERR_INDEXOUTOFBOUNDS);

    if(index >= v->cap)
        resize(v, index*SCALING);
    
    scale_up(v);

    for(size_t i = v->size-1; index < v->size && i > index; i--)
    {
        memcpy(v->arr+((i+1)*v->elem_size), v->arr+(i*v->elem_size), v->elem_size);
    }

    memcpy(v->arr+(index*v->elem_size), n, v->elem_size);
}


void erase_vector(Vector *v, size_t index)
{
    check_index(v, index);
        
    v->size--;

    for(size_t i = index; i < v->size; i++)
    {
        memcpy(v->arr+(i*v->elem_size), v->arr+((i+1)*v->elem_size), v->elem_size);
    }
}


void free_vector(Vector *v)
{
    free(v->arr);
    v->arr = NULL;
}
