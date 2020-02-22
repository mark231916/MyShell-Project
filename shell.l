
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include <string.h>
#include "y.tab.hh"
#include "command.hh"
#include <unistd.h>
#include <sys/wait.h>

////////////  Start added code ///////////

extern "C" char * read_line();


int mygetc(FILE * f) {
  static char *p;
  char ch;

  if (!isatty(0)) {
    // stdin is not a tty. Call real getc
    return getc(f);
  }

  // stdin is a tty. Call our read_line.
  if (p==NULL || *p == 0) {
    char * s = read_line();
    p = s;
  }

  ch = *p;
  p++;
  return ch;
}

#undef getc
#define getc(f) mygetc(f)

/////////// End added code ///////////

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

">>" {
  return GREATGREAT;
}

"|" {
  return PIPE;
}

"<" {
   return LESS;
}

"&" {
  return AMPERSAND;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;  
}

"2>" {
  return TWOGREAT;
}

$\([^\n]*\) {
  //subshell
  char *temp = strdup(yytext+2);
  if(temp[strlen(temp) - 1] == ')') {
    temp[strlen(temp) - 1] = '\0';
  }

  //printf("%s\n", temp);
  //printf("%d\n", strlen(temp));
  //save default io
  int tempIn = dup(0);
  int tempOut = dup(1);
  int pin[2];
  int pout[2];
    
  pipe(pin);
  pipe(pout);

  //wirte commands to pipe
  int k = write(pin[1], temp, strlen(temp));
  k = write(pin[1], "\n", 1);
 // write(pin[1], "exit", 4);
  //write(pin[1], "\n", 1);
  close(pin[1]);

  dup2(pin[0], 0);
  close(pin[0]);
  dup2(pout[1], 1);
  close(pout[1]);
    
  //creat child process
  int ret = fork();
  if(ret == 0) {
    execvp("/proc/self/exe", NULL);
    _exit(1);
  } else if(ret < 0) {
    perror("fork");
    exit(1);
  }
  
  waitpid(ret, NULL, 0);
    
  dup2(tempIn, 0);
  dup2(tempOut, 1);
  close(tempIn);
  close(tempOut);
    
  //read output from the child process(subshell)
  char c;
  char *buffer = (char*)malloc(5000);
  int i = 0;
    
  while(read(pout[0], &c, 1)) {
    if(c == '\n') {
      buffer[i] = ' ';
      i++;
    } else {
	buffer[i] = c;
	i++;
      }
  }

  //buffer[i] = '\0';
  //printf("%s\n", buffer);
  //put characters of the output back to the scanner's buffer
  for(int j = strlen(buffer); j >= 0; j--) {
    myunputc(buffer[j]);
  }
}

\"[^\n\"]*\" {
  //quotes
  char *temp = strdup(yytext+1);
  if(temp[strlen(temp)-1] == '"');
    temp[strlen(temp)-1] = '\0';
  yylval.cpp_string = new std::string(temp);
  //printf("%s\n", temp);
  return WORD;
}

[^ \t\n]*\\[^ \t\n]* {
  //escaping
  char *temp = yytext;
  char *word = (char*)malloc(100);
  int i = 0;
  //printf("%s\n", temp);
  while (*temp) {
    if(*temp == '\\') {
      if(*(temp+1) == '\\') {
	temp = temp + 2;
	word[i] = '\\';
	i++;
      }
      else {
	word[i] = *(++temp);
	i++;
      }
    }
    else {
      word[i] = *temp;
      i++;
    }

    temp++;
  }
  //printf("%s\n", word);
  word[i] = '\0';
  yylval.cpp_string = new std::string(word);
  return WORD;
}

[^ \t\n|><][^ \t\n|><]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}
