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
#define EDITOR_VERSION "1.0"
#define EDITOR_TAB_STOP 2
#define EDITOR_QUIT_TIMES 3

/**
 * Erow stands for “editor row”, and stores a line 
 * of text as a pointer to the dynamically-allocated 
 * character data and a length, and also the renderer 
 * and its length.
*/
typedef struct erow {
  /**
   * index of the line within the file.
  */
  int index;
  /**
   * row length
  */
  int size;
  /**
   * renderer length
  */
  int rsize;
  /**
   * row content
  */
  char* chars;
  /**
   * renderer content
  */
  char* render;
  /**
   * contains the highlight type corresponding to each character in the render
  */
  unsigned char* highlight;
  /**
   * boolean value that defines if the line is part of a comment
  */
  int hl_open_comment;
} erow;

/**
 * Stores the global state of the text editor, such 
 * as cursor coordinates, rows, file, status message, etc.
*/
struct editorConfig {
  /**
   * horizontal coordinate of the cursor
  */
  int cx;
  /**
   * vertical coordinate of the cursor
  */
  int cy;
  /**
   * vertical coordinate of the renderer
  */
  int rx;
  /**
   * row offset
  */
  int rowoff;
  /**
   * column offset
  */
  int coloff;
  /**
   * screen height
  */
  int screenrows;
  /**
   * screen width
  */
  int screencols;
  /**
   * number of rows of the text editor
  */
  int numrows;
  /**
   * stores rows of the text editor
  */
  erow* row;
  /**
   * text buffer that indicates whether the file has been modified
  */
  int dirty;
  /**
   * file opened in the text editor.
  */
  char* filename;
  /**
   * stores the current status bar message. 
  */
  char statusmsg[80];
  /**
   * stores the timestamp for the message
  */
  time_t statusmsg_time;
  /**
   * pointer to current editor syntax
  */
  struct editorSyntax *syntax;
  /**
   * stores original attributes of the terminal.
  */
  struct termios orig_termious; 
};

/**
 * Starts the global state of the editor, setting 
 * default values for global variables.
*/
void initEditor();

/**
 * Starts the text editor with an existing file. Responsible 
 * for reading the file and inserting all the content in 
 * the text editor rows.
 * 
 * @param filename file to be read
*/
void editorOpen(char* filename);

/**
 * Saves the content written in the text editor in an existing 
 * file, if not, it is also responsible for creating the file 
 * and inserting the content.
*/
void editorSave();

/**
 * Inserts a new character in the row belonging to 
 * the cursor coordinate.
 * 
 * @param c new character
*/
void editorInsertChar(int c);

/**
 * Inserts a new line in the text structure.
*/
void editorInsertNewLine();

/**
 * Delete a character referring to the cursor coordinate in a row.
*/
void editorDelChar();

/**
 * Navigates through similar terms found in the text.
 * 
 * @param query search term
 * @param key pressed key
*/
void editorFindCallback(char *query, int key);

/**
 * Search for terms in the text.
*/
void editorFind();

/**
 * Draws a status bar in the terminal, where the cursor 
 * coordinate, number of rows and whether the file has 
 * been modified is indicated.
 * 
 * @param ab pointer to append buffer
*/
void editorDrawStatusBar(struct abuf* ab);

/**
 * Draws a message bar on the status bar, using 
 * the defined status message.
 * 
 * @param ab pointer to append buffer
*/
void editorDrawMessageBar(struct abuf* ab);

/**
 * Defines status message that will be displayed on the status bar.
 * 
 * @param fmt formatted text
*/
void editorSetStatusMessage(const char* fmt, ...);

/**
 * Create a text input in the status bar to receive user 
 * input, such as file names, search terms, etc.
 * 
 * @param prompt message to be displayed as the input label
 * @param callback callback function
 * @return returns value received in the text input
*/
char *editorPrompt(char *prompt, void (*callback)(char *, int));

/**
 * Move the cursor through the text.
 * 
 * @param key arrow key pressed
*/
void editorMoveCursor(int key);

/**
 * Processes the action for the key pressed in the text editor.
*/
void editorProcessKeypress();

/**
 * Changes scrolling through the text editor rows, based 
 * on the cursor coordinate.
*/
void editorScroll();

/**
 * Draws the rows of text in the terminal.
 * 
 * @param ab pointer to append buffer
*/
void editorDrawRows(struct abuf* ab);

/**
 * Refresh the terminal window by frame allowing the visualization 
 * of changes in the global status of the text editor.
*/
void editorRefreshScreen();

/**
 * Convert the cursor's horizontal coordinate value to 
 * the renderer's horizontal coordinate value.
 * 
 * @param row row belonging to cursor coordinate
 * @param cx horizontal cursor coordinate
 * @return returns the converted value of the operation performed
*/
int editorRowCxToRx(erow* row, int cx);

/**
 * Convert the renderer's horizontal coordinate value to 
 * the cursor's horizontal coordinate value.
 * 
 * @param row row belonging to renderer coordinate
 * @param rx horizontal renderer coordinate
 * @return returns the converted value of the operation performed
*/
int editorRowRxToCx(erow* row, int rx);

/**
 * Update state of a row.
 * 
 * @param row row to be updated
*/
void editorUpdateRow(erow* row);

/**
 * Insert a new row at a specific position in the text.
 * 
 * @param at position to be added to row
 * @param s row content
 * @param len row content length
*/
void editorInsertRow(int at, char* s, size_t len);

/**
 * Deallocates dynamic memory from a row.
 * 
 * @param row target row
*/
void editorFreeRow(erow* row);

/**
 * Delete a row from the text editor.
 * 
 * @param at row position
*/
void editorDelRow(int at);

/**
 * Inserts a new character at a specific position in the row.
 * 
 * @param row row which will be updated 
 * @param at position to be added new character
 * @param c new char
*/
void editorRowInsertChar(erow* row, int at, int c);

/**
 * Appends a new string to a row.
 * 
 * @param row row which will be updated
 * @param s new string
 * @param len string length
*/
void editorRowAppendString(erow *row, char *s, size_t len);

/**
 * Delete a character in a specific position in a row.
 * 
 * @param row row which will be updated
 * @param at position the character to be deleted
*/
void editorRowDelChar(erow* row, int at);

/**
 * Turns all rows of the text editor into a string.
 * 
 * @param buflen buffer length
 * @return returns rows in string format
*/
char *editorRowsToString(int *buflen);

#endif