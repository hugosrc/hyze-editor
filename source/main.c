/* Includes */

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>

#include "terminal.h"
#include "editor.h"

/* Defines */

#define EDITOR_VERSION "0.0.1"
#define EDITOR_TAB_STOP 8
#define EDITOR_QUIT_TIMES 3

/* Data */

struct editorConfig Editor;

/* Prototypes */

void editorSetStatusMessage(const char* fmt, ...);

void editorRefreshScreen();

char *editorPrompt(char *prompt, void (*callback)(char *, int));

/* Row Operations */

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

/* Editor Operations */

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

/* File I/O */

char *editorRowsToString(int *buflen) {
  int totlen = 0;

  for (int i = 0; i < Editor.numrows; i++) {
    totlen += Editor.row[i].size + 1;
  }

  *buflen = totlen;

  char* buf = malloc(totlen);
  char *p = buf;

  for (int i = 0; i < Editor.numrows; i++) {
    memcpy(p, Editor.row[i].chars, Editor.row[i].size);
    p += Editor.row[i].size;
    *p = '\n';
    p++;
  }

  return buf;
}

void editorOpen(char* filename) {
  free(Editor.filename);
  Editor.filename = strdup(filename);

  FILE* fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char* line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  
  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) linelen--;
    editorInsertRow(Editor.numrows, line, linelen);
  }

  free(line);
  fclose(fp);
  Editor.dirty = 0;
}

void editorSave() {
  if (Editor.filename == NULL) {
    Editor.filename = editorPrompt("Save as: %s (ESC to cancel)", NULL);
    if (Editor.filename == NULL) {
      editorSetStatusMessage("Save aborted");
      return;
    }
  }

  int len;
  char *buf = editorRowsToString(&len);
  
  int fd = open(Editor.filename, O_RDWR | O_CREAT, 0644);

  if (fd != -1) {
    if (ftruncate(fd, len) != -1) {
      if (write(fd, buf, len) == len) {
        close(fd);
        free(buf);
        Editor.dirty = 0;
        editorSetStatusMessage("%d bytes written to disk", len);
        return;
      }
    }
    close(fd);
  }
  
  free(buf);
  editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
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

/* Append Buffer */

struct abuf {
  char* b;
  int length;
};

#define ABUF_INIT {NULL, 0}

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

/* Input */

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

/* Output */

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

void editorSetStatusMessage(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(Editor.statusmsg, sizeof(Editor.statusmsg), fmt, ap);
  va_end(ap);
  Editor.statusmsg_time = time(NULL);
}

/* Init */

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
