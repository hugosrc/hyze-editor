/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>

/* Data */
struct termios orig_termious;

/* Terminal */
void die(const char* s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termious) == -1) 
  {
    die("tcsetattr");
  }
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termious);
  atexit(disableRawMode);

  struct termios raw = orig_termious;

  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
  {
    die("tcsetattr");
  }
}

/* Init */
int main() { 
  enableRawMode();

  while (1)
  {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) 
    {
      die("read");
    }

    if (iscntrl(c)) 
    {
      printf("%d\r\n", c);
    } else 
    {
      printf("%d ('%c')\r\n", c, c);
    }

    if (c == 'q') break;
  }

  return 0;
}