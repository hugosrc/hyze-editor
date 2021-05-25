#ifndef KEYBOARD_H_
#define KEYBOARD_H_

/**
 * Constant that returns a decimal value for a 
 * sequence formed by pressing ctrl + key.
 * 
 * @param k key (char)
*/
#define CTRL_KEY(k) ((k) & 0x1f) 

/**
 * Custom keys on the keyboard.
*/
enum editorKey {
  BACKSPACE = 127, 
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

#endif