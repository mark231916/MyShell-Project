
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>

/* 
 * Sets terminal into raw mode. 
 * This causes having the characters available
 * immediately instead of waiting for a newline. 
 * Also there is no automatic echo.
 */

struct termios old_tty_attr; //use to store the default mode

void tty_raw_mode(void)
{
	struct termios new_tty_attr;
     
	tcgetattr(0,&old_tty_attr);
	tcgetattr(0,&new_tty_attr);

	/* Set raw mode. */
	new_tty_attr.c_lflag &= (~(ICANON|ECHO));
	new_tty_attr.c_cc[VTIME] = 0;
	new_tty_attr.c_cc[VMIN] = 1;
     
	tcsetattr(0,TCSANOW,&new_tty_attr);
}

//set terminal back to default mode
void tty_term_mode(void) {
	tcsetattr(0,TCSANOW,&old_tty_attr);
}

