#ifndef FILEIO_H
#define FILEIO_H

char *editorRowsToString(int *buflen);

void editorOpen(char* filename);

void editorSave();

#endif