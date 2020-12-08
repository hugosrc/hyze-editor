#ifndef BUFFER_H
#define BUFFER_H

struct abuf {
  char* b;
  int length;
};

void abAppend(struct abuf* ab, const char* s, int length);

void abFree(struct abuf* ab);

#endif