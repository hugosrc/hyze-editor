#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "prompt.h"
#include "editor.h"

extern struct editorConfig Editor;

void editorDrawStatusBar(struct abuf* ab) {
  abAppend(ab, "\x1b[7m", 4);

  char status[80], rstatus[80];

  int len = snprintf(status, sizeof(status), 
    "%.20s - %d lines %s", 
    Editor.filename ? Editor.filename : "[No Name]", 
    Editor.numrows, Editor.dirty ? "(modified)" : "");
  
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", Editor.cy + 1, Editor.numrows);

  if (len > Editor.screencols) len = Editor.screencols;
  abAppend(ab, status, len);

  while (len < Editor.screencols) {
    if (Editor.screenrows - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    } else {
      abAppend(ab, " ", 1);
      len++;
    }
  }
  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(struct abuf* ab) {
  abAppend(ab, "\x1b[K", 3);
  int msglen = strlen(Editor.statusmsg);
  if (msglen > Editor.screencols) msglen = Editor.screencols;
  if (msglen &&  time(NULL) - Editor.statusmsg_time < 5) abAppend(ab, Editor.statusmsg, msglen);
}

void editorSetStatusMessage(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(Editor.statusmsg, sizeof(Editor.statusmsg), fmt, ap);
  va_end(ap);
  Editor.statusmsg_time = time(NULL);
}