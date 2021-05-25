#ifndef EDITOR_H_
#define EDITOR_H_

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>

#include "terminal.h"
#include "keyboard.h"
#include "buffer.h"

#define EDITOR_NAME "Hyze Editor"
#define EDITOR_VERSION "0.0.1"
#define EDITOR_TAB_STOP 2
#define EDITOR_QUIT_TIMES 3

typedef struct erow {
  int size;
  int rsize;
  char* chars;
  char* render;
} erow;

struct editorConfig {
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  int numrows;
  erow* row;
  int dirty;
  char* filename;
  char statusmsg[80];
  time_t statusmsg_time;
  struct termios orig_termious; 
};

void initEditor();

void editorOpen(char* filename);

void editorSave();

void editorInsertChar(int c);

void editorInsertNewLine();

void editorDelChar();

void editorFindCallback(char *query, int key);

void editorFind();

void editorDrawStatusBar(struct abuf* ab);

void editorDrawMessageBar(struct abuf* ab);

void editorSetStatusMessage(const char* fmt, ...);

char *editorPrompt(char *prompt, void (*callback)(char *, int));

void editorMoveCursor(int key);

void editorProcessKeypress();

void editorScroll();

void editorDrawRows(struct abuf* ab);

void editorRefreshScreen();

int editorRowCxToRx(erow* row, int cx);

int editorRowRxToCx(erow* row, int rx);

void editorUpdateRow(erow* row);

void editorInsertRow(int at, char* s, size_t len);

void editorFreeRow(erow* row);

void editorDelRow(int at);

void editorRowInsertChar(erow* row, int at, int c);

void editorRowAppendString(erow *row, char *s, size_t len);

void editorRowDelChar(erow* row, int at);

char *editorRowsToString(int *buflen);

#endif