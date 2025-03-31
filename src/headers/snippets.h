#pragma once


/**
 * Macro that compares two values and returns the greater one.
 * 
 * \param A The first value.
 * \param B The second value.
 * 
 * \returns The value that is greater than the other.
 */
#define max(A, B) ({ \
    typeof(A) _tempx = (A); \
    typeof(B) _tempy = (B); \
    _tempx > _tempy ? _tempx : _tempy; \
})


/**
 * Macro that compares two values and returns the smaller one.
 * 
 * \param A The first value.
 * \param B The second value.
 * 
 * \returns The value that is lesser than the other.
 */
#define min(A, B) ({ \
    typeof(A) _tempx = (A); \
    typeof(B) _tempy = (B); \
    _tempx < _tempy ? _tempx : _tempy; \
})


/**
 * Creates a new copy of a given string.
 * The new string should be freed by the caller as it is dynamically allocated.
 * 
 * \param str The string to make a copy of.
 * 
 * \returns The new string with the same contents as the input.
 */
char* strclone(const char *str);


/**
 * Calls memdump() with the current file's name
 * and the line number where the macro was used.
 */
#define meminfo() memdump(__FILE__, __LINE__)


/**
 * Prints the current amount of memory allocation
 * calls and the sum of the allocated bytes.
 * 
 * Can be given a filename and a line number to indicate where it was called.
 * 
 * \param filename Path to the file.
 * \param line The line number.
 */
void memdump(char *filename, int line);

/**
 * Calculates the distance between two points on a 2D plane.
 * 
 * \param x1 The X coordinate of the first point.
 * \param y1 The Y coordinate of the first point.
 * \param x2 The X coordinate of the second point.
 * \param y2 The Y coordinate of the second point.
 * 
 * \returns The distance between the two points.
 */
double distance(long long x1, long long y1, long long x2, long long y2);
