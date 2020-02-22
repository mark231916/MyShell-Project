
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <string.h>
#include <regex.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE GREATGREAT PIPE LESS AMPERSAND GREATGREATAMPERSAND GREATAMPERSAND TWOGREAT

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"
#include "y.tab.hh"

void yyerror(const char * s);
int yylex();
void expandWildCardsIfNecessary(char * arg);
int compare(const void * file1, const void * file2);
void expandWildCard(char * prefix, char * arg);
%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;

simple_command:	
  pipe_list iomodifier_list background_optional NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
    if($1->find("{") != std::string::npos && $1->find("}") != std::string:: npos)
      Command::_currentSimpleCommand->insertArgument( $1 );
    else if($1->find("*") == std::string::npos && $1->find("?") == std::string::npos)
      Command::_currentSimpleCommand->insertArgument( $1 );
    else
      expandWildCardsIfNecessary((char*)$1->c_str());
  }
  ;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;
pipe_list:
  pipe_list PIPE {
    //printf("Pipe");
  } command_and_args
  | command_and_args
  ;

iomodifier_opt:
  GREAT WORD {
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
  }
  | GREATGREAT WORD {
    //printf("GREATGREAT: append output to \"%s\"\n", $2->c_str());  
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._append = true;
  }
  | LESS WORD {
    //printf("LESS: input from \"%s\"\n", $2->c_str());
    Shell::_currentCommand._inFile = $2;
  }
  | GREATAMPERSAND WORD { 
    //printf("GREATAMPERSAND: redirect stdout and stderr to \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._errFile = $2;
  } 
  | GREATGREATAMPERSAND WORD {
    //printf("GREATGREATAMPERSAND: append stdout and stderr to \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._append = true;
  }
  | TWOGREAT WORD {
    //printf("TWOGREAT: redirect stderr to \"%s\"\n", $2->c_str());
    Shell::_currentCommand._errFile = $2;
  }
  ;
iomodifier_list:
  iomodifier_list iomodifier_opt
  | iomodifier_opt
  | /*empty*/
  ;

background_optional:
  AMPERSAND {
    //printf("AMPERSAND");
    Shell::_currentCommand._background = true;
  }
  | /*empty*/
  ;

%%
int maxEntries = 20;
int nEntries = 0;
char ** array = (char**)malloc(maxEntries*sizeof(char*));

int compare(const void * file1, const void * file2) {
  const char * name1 = *(const char**) file1;
  const char * name2 = *(const char**) file2;
  return strcmp(name1, name2);
}

void expandWildCardsIfNecessary(char * arg) {
  //maxEntries = 20;
  //nEntries = 0;
  //array = (char**)malloc(maxEntries*sizeof(char*));

  expandWildCard("", arg);
  qsort(array, nEntries, sizeof(char*), compare);
  for(int i = 0; i < nEntries; i++) {
    if(array[i][0] != '.')
      Command::_currentSimpleCommand->insertArgument(new std::string(array[i]));
  }

  free(array);
}

void expandWildCard(char * prefix, char * arg) {
  char *reg = (char*)malloc(2*strlen(arg)+10);
  char *a = arg;
  char *r = reg;
  *r = '^';
  r++;
  while(*a) {
    if(*a == '*') {
      *r = '.';
      r++;
      *r = '*';
      r++;
    }
    else if(*a == '?') {
      *r = '.';
      r++;
    }
    else if(*a == '.') {
      *r = '\\';
      r++;
      *r = '.';
      r++;
    }
    else {
      *r = *a;
      r++;
    }
    a++;
  }
  *r = '$';
  r++;
  *r = 0;

  regex_t re;
  int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
  DIR * dir = opendir(".");

  if(dir == NULL) {
    perror("oopendir");
    return;
  }
  struct dirent *ent;

  while((ent = readdir(dir)) != NULL) {
    if(regexec(&re, ent->d_name, (size_t) 0, NULL, 0) == 0) {
      if(nEntries == maxEntries) {
	maxEntries *= 2;
	array = (char**)realloc(array, maxEntries * sizeof(char*));
	assert(array!=NULL);
      }
      array[nEntries] = strdup(ent->d_name);
      nEntries++;
    }
  }
  closedir(dir);
}

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
