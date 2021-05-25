#include "buffer.h"

void abAppend(struct abuf* ab, const char* s, int length) {
  char *new = realloc(ab->b, ab->length + length);
  
  if (new == NULL) return;

  memcpy(&new[ab->length], s, length);
  ab->b = new;
  ab->length += length;
} 

void abFree(struct abuf* ab) {
  free(ab->b);
}