#ifndef PROMPT_H
#define PROMPT_H 

#include "buffer.h"

void editorDrawStatusBar(struct abuf* ab);

void editorDrawMessageBar(struct abuf* ab);

void editorSetStatusMessage(const char* fmt, ...);

#endif