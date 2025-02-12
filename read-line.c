/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_LINE 2048

extern void tty_raw_mode(void);

// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];

// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char * history [100];
int history_length = 0;

void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
  int pos = line_length; // hold current position of cursor
  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32) {
      // It is a printable character. 

      // Do echo
      write(1,&ch,1);

      // If max number of character reached return.
      if (line_length==MAX_BUFFER_LINE-2) break; 

      // add char to buffer.
      line_buffer[line_length]=ch;
      if(pos == line_length)
	line_length++;
      pos++;
    }
    else if (ch==10) {
      // <Enter> was typed. Return line
      
      // Print newline
      write(1,&ch,1);

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0]=0;
      break;
    }
    else if (ch == 8) {
      // ctrl+H was typed. Remove previous character read.
      if(line_length > 0) {
	// Go back one character
	ch = 8;
	write(1,&ch,1);

	// Write a space to erase the last character read
	ch = ' ';
	write(1,&ch,1);

	// Go back one character
	ch = 8;
	write(1,&ch,1);

	// Remove one character from buffer
	line_length--;
	pos--;
      }
    }
    else if(ch == 1) {
      //Home
      while (pos > 0) {
	ch = 27;
	write(1, &ch, 1);
	ch = 91;
	write(1, &ch, 1);
	ch = 68;
	write(1, &ch, 1);
	pos--;
      }
    }
    else if(ch == 5) {
      //End
      while (pos < line_length) {
	ch = 27;
	write(1, &ch, 1);
	ch = 91;
	write(1, &ch, 1);
	ch = 67;
	write(1, &ch, 1);
	pos++;
      }
    }
    else if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if (ch1==91 && ch2==65) {
	// Up arrow. Print next line in history.

	// Erase old line
	// Print backspaces
	int i = 0;
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}

	// Print spaces on top
	for (i =0; i < line_length; i++) {
	  ch = ' ';
	  write(1,&ch,1);
	}

	// Print backspaces
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}	

	// Copy line from history
	if(history_length > 0 && history_index >= 0) {
	  strcpy(line_buffer, history[history_index]);
	  history_index--;
	  //history_index=(history_index)%history_length;
	  if(history_index < 0) {
	    history_index = 0;
	  }
	  line_length = strlen(line_buffer);
	}

	// echo line
	//printf("%d\n", history_index);
	write(1, line_buffer, line_length);
      }
      else if (ch1 == 91 && ch2 == 66) {
	// Down arrow.

	// Erase old line
	// Print backspaces
	int i = 0;
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}

	// Print spaces on top
	for (i =0; i < line_length; i++) {
	  ch = ' ';
	  write(1,&ch,1);
	}

	// Print backspaces
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}

	// Copy line from history
	if(history_length > 0 && history_index < history_length) {
	  strcpy(line_buffer, history[history_index]);
	  history_index++;
	}
	else if (history_index >= history_length) {
	  history_index = history_length - 1;
	  strcpy(line_buffer, "");
	}
	line_length = strlen(line_buffer);
	//printf("%d\n", history_index);
	write(1, line_buffer, line_length);
      }
      //left arrow
      else if(ch1 == 91 && ch2 == 68) {
	if(pos > 0) {
	  ch = 27;
	  write(1, &ch, 1);
	  ch = 91;
	  write(1, &ch, 1);
	  ch = 68;
	  write(1, &ch, 1);
	  pos--;
	}
      }
      //right arrow
      else if(ch1 == 91 && ch2 == 67) {
	if(pos < line_length) {
	  ch = 27;
	  write(1, &ch, 1);
	  ch = 91;
	  write(1, &ch, 1);
	  ch = 67;
	  write(1, &ch, 1);
	  pos++;
	}
      }
      
    }

  }

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;
  
  //write buffer to history
  history[history_length] = (char*)malloc(strlen(line_buffer)*sizeof(char) + 1);
  strcpy(history[history_length], line_buffer);
  history[history_length][strlen(line_buffer)-1] = '\0';
  history_index = history_length;
  history_length++; 
  

  tty_term_mode();
  return line_buffer;
}

