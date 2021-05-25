#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>

#include "editor.h"

/**
 * Used for error handling, die() exits the application 
 * and prints an error message. After printing the error message, 
 * the program is closed with status 1, which indicates failure.
 * 
 * @param s reason message
*/
void die(const char *s);

/**
 * Restores the original attributes of the terminal. Uses 
 * the attributes stored in the terminal in a global variable 
 * before changes are made.
*/
void disableRawMode();

/**
 * Stores original attributes of the terminal and inserts 
 * new attributes modifying the original structure.
*/
void enableRawMode();

/**
 * Listens for keyboard events and reads keys pressed 
 * only or in sequence, and returns value referring 
 * to the ASCII table
 * 
 * @return Returns decimal value for the key pressed 
 * according to the ASCII table
*/
int editorReadKey();

/**
 * Take the horizontal and vertical position of 
 * the cursor on the terminal.
 * 
 * @param rows variable to place the vertical cursor position.
 * @param cols variable to place the horizontal cursor position.
 * @return >= 0 on success
*/
int getCursorPosition(int *rows, int *cols);

/**
 * It takes the size of the window used by the terminal, 
 * returning the number of rows and columns of the window.
 * 
 * @param rows variable to place the number of rows of the window.
 * @param cols vvariable to place the number of columns in the window.
 * @return >= 0 on success
*/
int getWindowSize(int *rows, int *cols);

#endif