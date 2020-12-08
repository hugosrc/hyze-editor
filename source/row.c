#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "row.h"
#include "editor.h"

extern struct editorConfig Editor;

int editorRowCxToRx(erow* row, int cx) {
  int rx = 0;
  for (int i = 0; i < cx; i++) {
    if (row->chars[i] == '\t') {
      rx += (EDITOR_TAB_STOP - 1) - (rx % EDITOR_TAB_STOP);
    }

    rx++;
  }

  return rx;
}

int editorRowRxToCx(erow* row, int rx) {
  int cur_rx = 0;
  
  int cx;
  for (cx = 0; cx < row->size; cx++) {
    if (row->chars[cx] == '\t') {
      cur_rx += (EDITOR_TAB_STOP - 1) - (cur_rx % EDITOR_TAB_STOP);
    }

    cur_rx++;

    if (cur_rx > rx) return cx;
  }

  return cx;
}

void editorUpdateRow(erow* row) {
  int tabs = 0;

  for (int i = 0; i < row->size; i++) {
    if (row->chars[i] == '\t') tabs++;
  }

  free(row->render);
  row->render = malloc(row->size + tabs * (EDITOR_TAB_STOP - 1) + 1);

  int index = 0;
  for (int i = 0; i < row->size; i++) {
    if (row->chars[i] == '\t') {
      row->render[index++] = ' ';
      while (index % EDITOR_TAB_STOP != 0) row->render[index++] = ' ';
    } else {
      row->render[index++] = row->chars[i];
    }
  }

  row->render[index] = '\0';
  row->rsize = index;
}

void editorInsertRow(int at, char* s, size_t len) {
  if (at < 0 || at > Editor.numrows) return;

  Editor.row = realloc(Editor.row, sizeof(erow) * (Editor.numrows + 1));
  memmove(&Editor.row[at + 1], &Editor.row[at], sizeof(erow) * (Editor.numrows - at));

  Editor.row[at].size = len;
  Editor.row[at].chars = malloc(len + 1);
  memcpy(Editor.row[at].chars, s, len);
  Editor.row[at].chars[len] = '\0';

  Editor.row[at].rsize = 0;
  Editor.row[at].render = NULL;
  editorUpdateRow(&Editor.row[at]);

  Editor.numrows++;
  Editor.dirty++;
}

void editorFreeRow(erow* row) {
  free(row->render);
  free(row->chars);
}

void editorDelRow(int at) {
  if (at < 0 || at >= Editor.numrows) return;
  editorFreeRow(&Editor.row[at]);
  memmove(&Editor.row[at], &Editor.row[at + 1], sizeof(erow) * (Editor.numrows - at - 1));
  Editor.numrows--;
  Editor.dirty++;
}

void editorRowInsertChar(erow* row, int at, int c) {
  if (at < 0 || at > row->size) at = row->size;

  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  
  row->size++;
  row->chars[at] = c;

  editorUpdateRow(row);
  
  Editor.dirty++;
}

void editorRowAppendString(erow *row, char *s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);

  memcpy(&row->chars[row->size], s, len);

  row->size += len;
  row->chars[row->size] = '\0';

  editorUpdateRow(row);
  Editor.dirty++;
}

void editorRowDelChar(erow* row, int at) {
  if (at < 0 || at >= row->size) return;

  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);

  row->size--;
  editorUpdateRow(row);

  Editor.dirty++;
}