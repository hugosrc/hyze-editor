#include "syntax.h"

#include "syntax/c.h"

extern struct editorConfig Editor;

struct editorSyntax HLDB[] = {
  C_SYNTAX
};

void editorUpdateSyntax(erow *row) {
  row->highlight = realloc(row->highlight, row->rsize);
  memset(row->highlight, HL_NORMAL, row->rsize);

  if (Editor.syntax == NULL) return;

  char **keywords = Editor.syntax->keywords;

  char *scs = Editor.syntax->singlelineCommentStart;
  char *mcs = Editor.syntax->multilineCommentStart;
  char *mce = Editor.syntax->multilineCommentEnd;

  int scs_len = scs ? strlen(scs) : 0;
  int mcs_len = mcs ? strlen(mcs) : 0;
  int mce_len = mce ? strlen(mce) : 0;

  int prev_sep = 1;
  int in_string = 0;
  int in_comment = (row->index > 0 && Editor.row[row->index - 1].hl_open_comment);

  int i = 0;
  while (i < row->rsize) {
    int c = row->render[i];
    unsigned char prev_hl = (i > 0) ? row->highlight[i - 1] : HL_NORMAL;

    if (scs_len && !in_string && !in_comment) {
      if (!strncmp(&row->render[i], scs, scs_len)) {
        memset(&row->highlight[i], HL_COMMENT, row->size - i);
        break;
      }
    }

    if (mcs_len && mce_len && !in_string) {
      if (in_comment) {
        row->highlight[i] = HL_MLCOMMENT;
        if (!strncmp(&row->render[i], mce, mce_len)) {
          memset(&row->highlight[i], HL_MLCOMMENT, mce_len);
          i += mce_len;
          in_comment = 0;
          prev_sep = 1;
          continue;
        } else {
          i++;
          continue;
        }
      } else if (!strncmp(&row->render[i], mcs, mcs_len)) {
        memset(&row->highlight[i], HL_MLCOMMENT, mcs_len);
        i += mcs_len;
        in_comment = 1;
        continue;
      }
    }

    if (Editor.syntax->flags & HL_HIGHLIGHT_STRINGS) {
      if (in_string) {
        row->highlight[i] = HL_STRING;
        if (c == '\\' && i + 1 < row->rsize) {
          row->highlight[i + 1] = HL_STRING;
          i+= 2;
          continue;
        }
        if (c == in_string) in_string = 0;
        i++;
        prev_sep = 1;
        continue;
      } else {
        if (c == '"' || c == '\'') {
          in_string = c;
          row->highlight[i] = HL_STRING;
          i++;
          continue;
        }
      }
    }

    if (Editor.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
      if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) || (c == '.' && prev_hl == HL_NUMBER)) {
        row->highlight[i] = HL_NUMBER;
        i++;
        prev_sep = 0;
        continue;
      }
    }

    if (prev_sep) {
      int j;
      for (j = 0; keywords[j]; j++) {
        int klen = strlen(keywords[j]);
        int kw2 = keywords[j][klen - 1] == '|';
        if (kw2) klen--;

        if (!strncmp(&row->render[i], keywords[j], klen) && isSeparator(row->render[i + klen])) {
          memset(&row->highlight[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
          i += klen;
          break;
        }
      }

      if (keywords[j] != NULL) {
        prev_sep = 0;
        continue;
      }
    }

    prev_sep = isSeparator(c);
    i++;
  }

  int changed = (row->hl_open_comment != in_comment);
  row->hl_open_comment = in_comment;
  if (changed && row->index + 1 < Editor.numrows) {
    editorUpdateSyntax(&Editor.row[row->index + 1]);
  }
}

int editorSyntaxToColor(int highlight) {
  switch (highlight)
  {
  case HL_STRING:
    return 93; // Bright Yellow
  case HL_NUMBER:
    return 33; // Yellow
  case HL_COMMENT:
  case HL_MLCOMMENT:
    return 90; // Bright Black (Gray)
  case HL_KEYWORD1:
    return 36; // Cyan
  case HL_KEYWORD2:
    return 35; // Magenta
  case HL_MATCH: 
    return 34; // Blue
  default:
    return 37; // White
  }
}

int isSeparator(int c) {
  return isspace(c) || c == '\0' || strchr(",.()+=/*=~%<>[];", c) != NULL;
}

void editorSelectSyntaxHighlight() {
  Editor.syntax = NULL;
  if (Editor.filename == NULL) return;

  char *extention = strrchr(Editor.filename, '.');

  for (unsigned int i = 0; i < HLDB_ENTRIES; i++) {
    struct editorSyntax *syntax = &HLDB[i];
    
    unsigned int j = 0;
    while (syntax->filematch[j]) {
      int is_extention = (syntax->filematch[j][0] == '.');

      if ((is_extention && extention && !strcmp(extention, syntax->filematch[j])) 
        || (!is_extention && strstr(Editor.filename, syntax->filematch[j]))) {
          Editor.syntax = syntax;
          return;
      }

      int filerow;
      for (filerow = 0; filerow < Editor.numrows; filerow++) {
        editorUpdateSyntax(&Editor.row[filerow]);
      }

      j++;
    }
    
  }
}