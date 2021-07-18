#ifndef SYNTAX_H_
#define SYNTAX_H_

#include "editor.h"

/**
 * Bit field containing flags to highlight numbers.
*/
#define HL_HIGHLIGHT_NUMBERS (1<<0)

/**
 * Bit field containing flags to highlight strings.
*/
#define HL_HIGHLIGHT_STRINGS (1<<1)

/**
 * Constant to store the length of the HLDB array.
*/
#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

/**
 * Defines the highlight types
*/
enum editorHighlight {
  HL_NORMAL = 0,
  HL_STRING,
  HL_NUMBER,
  HL_COMMENT,
  HL_MLCOMMENT,
  HL_KEYWORD1,
  HL_KEYWORD2,
  HL_MATCH
};

/**
 * Struct that contains all syntax highlighting information 
 * for a particular filetype
*/
struct editorSyntax {
  /**
   * name of the filetype that will be displayed to the user in the status bar
  */
  char *filetype;
  /**
   * array of strings, where each string contains a pattern to match a filename
  */
  char **filematch;
  /**
   * array containing each keywords for syntax highlighting
  */
  char **keywords;
  /**
   * single line comment pattern
  */
  char *singlelineCommentStart;
  /**
   * multiline comment opening pattern
  */
  char *multilineCommentStart;
  /**
   * multiline comment closing pattern
  */
  char *multilineCommentEnd;
  /**
   * bit field which will contain flags to highlight eg numbers and 
   * highlight strings for that filetype
  */
  int flags;
};

/**
 * Traverse the entire line and define all the highlight types present 
 * in the line according to the selected syntax.
 * 
 * @param row row to be updated
*/
void editorUpdateSyntax(erow *row);

/**
 * Map highlight array values to ASCI color codes
 * 
 * @param highlight enum value editorHighlight
 * 
 * @return returns color code in ASCI format
*/
int editorSyntaxToColor(int highlight);

/**
 * Function that takes a character and returns true if 
 * considered a separator character
 * 
 * @param c character to be checked
 * 
 * @return return a boolean value, 0 for false and 1 for true
*/
int isSeparator(int c);

/**
 * Attempts to match the current filename to one of the file match
 * fields in HLDB. If one matches, it sets E.syntax for that file type.
*/
void editorSelectSyntaxHighlight();

#endif
