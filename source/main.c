#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "terminal.h"
#include "prompt.h"
#include "editor.h"
#include "buffer.h"
#include "row.h"
#include "io.h"

struct editorConfig Editor;

void editorSetStatusMessage(const char* fmt, ...);
void editorRefreshScreen();

char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
  size_t bufsize = 128;
  char *buf = malloc(bufsize);

  size_t buflen = 0;
  buf[0] = '\0';

  while (1) {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen();

    int c = editorReadKey();

    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
      if (buflen != 0) buf[--buflen] = '\0';
    } else if (c == '\x1b') {
      editorSetStatusMessage("");
      if (callback) callback(buf, c);
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (buflen != 0) {
        editorSetStatusMessage("");
        if (callback) callback(buf, c);
        return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }

      buf[buflen++] = c;
      buf[buflen] = '\0';
    }

    if (callback) callback(buf, c);
  }
}

void editorMoveCursor(int key) {
  erow* row = (Editor.cy >= Editor.numrows) ? NULL : &Editor.row[Editor.cy];

  switch (key) {
    case ARROW_LEFT:
      if (Editor.cx != 0) {
        Editor.cx--;
      } else if (Editor.cy > 0) {
        Editor.cy--;
        Editor.cx = Editor.row[Editor.cy].size;
      }
      break;
    case ARROW_RIGHT:
        if (row && Editor.cx < row->size) {
          Editor.cx++;
        } else if (row && Editor.cx == row->size) {
          Editor.cy++;
          Editor.cx = 0;
        }
      break;
    case ARROW_UP:
      if (Editor.cy != 0) {
        Editor.cy--;
      }
      break;
    case ARROW_DOWN:
      if (Editor.cy < Editor.numrows) {
        Editor.cy++;
      }
      break;
  }

  row = (Editor.cy >= Editor.numrows) ? NULL : &Editor.row[Editor.cy];
  int rowlen = row ? row->size : 0;
  if (Editor.cx > rowlen) {
    Editor.cx = rowlen;
  }
}

void editorProcessKeypress() {
  static int quit_times = EDITOR_QUIT_TIMES;

  int c = editorReadKey();

  switch (c) {
    case '\r':
      editorInsertNewLine();
      break;

    case CTRL_KEY('q'):
      if (Editor.dirty && quit_times > 0) {
        editorSetStatusMessage(
          "WARNING!!! File has unsaved changes. " 
          "Press Ctrl-Q %d more times to quit.", quit_times);
          quit_times--;
          return;
      }

      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;

    case CTRL_KEY('s'):
      editorSave();
      break;

    case HOME_KEY:
      Editor.cx = 0;
      break;

    case END_KEY:
      if (Editor.cy < Editor.numrows) {
        Editor.cx = Editor.row[Editor.cy].size;
      }
      break;

    case CTRL_KEY('f'):
      editorFind();
      break;

    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL_KEY:
      if (c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
      editorDelChar();
      break;

    case PAGE_UP:
    case PAGE_DOWN:
      {
        if (c == PAGE_UP) {
          Editor.cy = Editor.rowoff;
        } else if (c == PAGE_DOWN) {
          Editor.cy = Editor.rowoff + Editor.screenrows - 1;
          if (Editor.cy > Editor.numrows) {
            Editor.cy = Editor.numrows;
          }
        }

        int times = Editor.screenrows;
        while(times--) editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
      break; 

    case ARROW_LEFT:
    case ARROW_RIGHT:
    case ARROW_UP:
    case ARROW_DOWN:
      editorMoveCursor(c);
      break;

    case CTRL_KEY('l'):
    case '\x1b':
      break;

    default:
      editorInsertChar(c);
      break;
  }

  quit_times = EDITOR_QUIT_TIMES;
}

void editorScroll() {  
  Editor.rx = 0;
  if (Editor.cy < Editor.numrows) {
    Editor.rx = editorRowCxToRx(&Editor.row[Editor.cy], Editor.cx);
  }

  if (Editor.cy < Editor.rowoff) {
    Editor.rowoff = Editor.cy;
  } 

  if (Editor.cy >= Editor.rowoff + Editor.screenrows) {
    Editor.rowoff = Editor.cy - Editor.screenrows + 1;
  }

  if (Editor.rx < Editor.coloff) {
    Editor.coloff = Editor.rx;
  }

  if (Editor.rx >= Editor.coloff + Editor.screencols) {
    Editor.coloff = Editor.rx - Editor.screencols + 1;
  }
}

void editorDrawRows(struct abuf* ab) {
  for (int i = 0; i < Editor.screenrows; i++) {
    int filerow = i + Editor.rowoff;

    if (filerow >= Editor.numrows) {

      if (Editor.numrows == 0 && i == Editor.screenrows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome), "Text Editor -- version %s", EDITOR_VERSION);
        
        if (welcomelen > Editor.screencols) welcomelen = Editor.screencols;

        int padding = (Editor.screencols - Editor.screenrows) / 2;
        if (padding) 
        {
          abAppend(ab, "~", 1);
          padding--;
        }
        
        while (padding--) abAppend(ab, " ", 1);

        abAppend(ab, welcome, welcomelen);
      } else {
        abAppend(ab, "~", 1);
      }

    } else {
      int len = Editor.row[filerow].rsize - Editor.coloff;
      if (len < 0) len = 0;
      if (len > Editor.screencols) len = Editor.screencols;
      abAppend(ab, &Editor.row[filerow].render[Editor.coloff], len);
    }

    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }
}

void editorRefreshScreen() {
  editorScroll();

  struct abuf ab = ABUF_INIT; 

  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawMessageBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (Editor.cy - Editor.rowoff) + 1, (Editor.rx + Editor.coloff) + 1);
  
  abAppend(&ab, buf, strlen(buf));
  
  abAppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.length);
  abFree(&ab);
}

void initEditor() {
  Editor.cx = 0;
  Editor.cy = 0;
  Editor.rx = 0;
  Editor.rowoff = 0;
  Editor.coloff = 0;
  Editor.numrows = 0;
  Editor.row = NULL;
  Editor.dirty = 0;
  Editor.filename = NULL;
  Editor.statusmsg[0] = '\0';
  Editor.statusmsg_time = 0;

  if (getWindowSize(&Editor.screenrows, &Editor.screencols) == -1) die("getWindowSize");
  Editor.screenrows -= 2;
}

int main(int argc, char* argv[]) { 
  enableRawMode();
  initEditor();
  if (argc >= 2) {
    editorOpen(argv[1]);
  }

  editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");

  while (1)
  {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
