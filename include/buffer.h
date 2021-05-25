#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdlib.h>
#include <string.h>

#define ABUF_INIT { NULL, 0 }

struct abuf {
  char* b;
  int length;
};

void abAppend(struct abuf* ab, const char* s, int length);

void abFree(struct abuf* ab);

#endif
