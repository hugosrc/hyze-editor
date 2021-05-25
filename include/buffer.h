#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdlib.h>
#include <string.h>

/**
 * Constant used to start an empty append buffer.
*/
#define ABUF_INIT { NULL, 0 }

/**
 * The structure of an append buffer is a memory 
 * pointer to a buffer and its length.
*/
struct abuf {
  /**
   * memory pointer to a buffer
  */
  char* b;
  /**
   * buffer length
  */
  int length;
};

/**
 * Append a string to an abuf, for which the buffer 
 * memory block is reallocated, freeing up enough 
 * memory to append the new string.
 * 
 * @param ab append buffer to which the new string will be attached
 * @param s string to be attached.
 * @param length length of the string to be attached.
*/
void abAppend(struct abuf* ab, const char* s, int length);

/**
 * It is a destructor that deallocates the dynamic memory 
 * used by an append buffer.
 * 
 * @param ab append buffer which will be deallocated the memory.
*/
void abFree(struct abuf* ab);

#endif
