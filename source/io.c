#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "io.h"
#include "editor.h"
#include "terminal.h"

extern struct editorConfig Editor;
extern void editorSetStatusMessage(const char* fmt, ...);
extern char *editorPrompt(char *prompt, void (*callback)(char *, int));

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