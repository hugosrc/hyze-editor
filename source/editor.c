#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "editor.h"
#include "row.h"

extern struct editorConfig Editor;
extern char *editorPrompt(char *prompt, void (*callback)(char *, int));

void editorInsertChar(int c) {
  if (Editor.cy == Editor.numrows) {
    editorInsertRow(Editor.numrows, "", 0);
  }

  editorRowInsertChar(&Editor.row[Editor.cy], Editor.cx, c);
  Editor.cx++;
}

void editorInsertNewLine() {
  if (Editor.cx == 0) {
    editorInsertRow(Editor.cy, "", 0);
  } else {
    erow *row = &Editor.row[Editor.cy];
    editorInsertRow(Editor.cy + 1, &row->chars[Editor.cx], row->size - Editor.cx);
    row = &Editor.row[Editor.cy];
    row->size = Editor.cx;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
  }

  Editor.cy++;
  Editor.cx = 0;
}

void editorDelChar() {
  if (Editor.cy == Editor.numrows) return;
  if (Editor.cx == 0 && Editor.cy == 0) return;

  erow* row = &Editor.row[Editor.cy];
  if (Editor.cx > 0) {
    editorRowDelChar(row, Editor.cx - 1);
    Editor.cx--;
  } else {
    Editor.cx = Editor.row[Editor.cy - 1].size;
    editorRowAppendString(&Editor.row[Editor.cy - 1], row->chars, row->size);
    editorDelRow(Editor.cy);
    Editor.cy--;
  }
}

/* Find */

void editorFindCallback(char *query, int key) {
  static int last_match = -1;
  static int direction = 1;

  if (key == '\r' || key == '\x1b') {
    last_match = -1;
    direction = 1;
    return;
  } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
    direction = 1;
  } else if (key == ARROW_LEFT || key == ARROW_UP) {
    direction = -1;
  } else {
    last_match = -1;
    direction = 1;
  }

  if (last_match == -1) direction = 1;

  int current = last_match;

  for (int i = 0; i < Editor.numrows; i++) {
    current += direction;
    if (current == -1) {
      current = Editor.numrows - 1;
    } else if (current == Editor.numrows) {
      current = 0;
    }

    erow *row = &Editor.row[current];
    char *match = strstr(row->render, query);
    if (match) {
      last_match = current;
      Editor.cy = current;
      Editor.cx = editorRowRxToCx(row, match - row->render);
      Editor.rowoff = Editor.numrows;
      break;
    }
  }
}

void editorFind() {
  int saved_cx = Editor.cx;
  int saved_cy = Editor.cy;
  int saved_coloff = Editor.coloff;
  int saved_rowoff = Editor.rowoff;

  char *query = editorPrompt("Search: %s (Use ESC/Arrows/Enter)", editorFindCallback);

  if (query) {
    free(query);
  } else {
    Editor.cx = saved_cx;
    Editor.cy = saved_cy;
    Editor.coloff = saved_coloff;
    Editor.rowoff = saved_rowoff;
  }
}
