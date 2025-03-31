#pragma once

#include <stdbool.h>

#define SCALING 2
#define SHRINK 0.2


/**
 * A Vector is a dynamic array of generic values stored by their bytes.
 */
typedef struct Vector {
    void *arr; /*!< Pointer to the currently allocated array. */
    size_t elem_size; /*!< The amount of bytes a single element needs to be allocated. */
    size_t size; /*!< The number of elements currently stored inside the Vector */
    size_t cap; /*!< The current maximum capacity of the Vector in the number of elements. */
} Vector;


/**
 *  Queries a Vector at a given index and returns the stored value.
 * 
 * \param V Pointer to the target Vector.
 * \param N The index.
 * \param T The type of the value to be returned from the Vector.
 * 
 * \returns The stored value.
 */
#define get_vector_as_type(V, N, T) (*(T *) get_vector_address((V), (N)))


/**
 * Queries a Vector at a given index.
 * 
 * \param v Pointer to the target Vector.
 * \param index The index.
 * 
 * \returns The pointer to where the value's bytes start inside the Vector at the given index.
 */
void* get_vector_address(const Vector *v, size_t index);


/**
 * Creates a new Vector with a given capacity.
 * The returned Vector should be freed by the caller,
 * as it contains dynamically allocated memory.
 * 
 * \param size Starting capacity of the Vector.
 * \param elem_size The number of bytes to allocate for a single element.
 * \param reset Should the allocation reset all memory-garbage to 0?
 * 
 * \returns The new Vector struct.
 */
Vector create_vector(size_t size, size_t elem_size, bool reset);


/**
 * Places a new element at the end of the Vector.
 * 
 * \param v Pointer to the target Vector.
 * \param n Pointer to the new element's starting byte.
 */
void push_vector(Vector *v, const void *n);


/**
 * Removes the last element in a Vector.
 * 
 * \param v Pointer to the target Vector.
 */
void pop_vector(Vector *v);


/**
 * Places a new element into a Vector at a given index.
 * 
 * \param v Pointer to the target Vector.
 * \param 
 */
void insert_vector(Vector *v, const void *n, size_t index);


/**
 * Removes an element from a Vector at a given index.
 * 
 * \param v Pointer to the target Vector.
 * \param index The index of the element to be removed.
 */
void erase_vector(Vector *v, size_t index);


/**
 * Frees all dynamically allocated memory used by a Vector.
 * 
 * \param v Pointer to the Vector that should be be freed.
*/
void free_vector(Vector *v);
