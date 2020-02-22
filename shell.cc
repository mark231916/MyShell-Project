#include <cstdio>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.hh"
#include <unistd.h>
#include <limits.h>
int yyparse(void);
int err = 1;

void Shell::prompt() {
  char* Prompt = getenv("PROMPT");
  char* onError = getenv("ON_ERROR");

  //use isatty to find if input is come from a file
  if(isatty(0) == 1 && !Prompt) {
    printf("myshell>");
    fflush(stdout);
  } 
  if(onError == NULL)
    err = 0;
  if(isatty(0) == 1 && err == 0 && Prompt) {
    printf("%s", Prompt);
  }
  if(isatty(0) == 1 && err == 1 && Prompt) {
    printf("%s\n", onError);
    printf("%s", Prompt);
  }

  fflush(stdout);
  err = 1;
}

extern "C" void disp (int sig) {
  printf("ctrl C\n");
  Shell::prompt();
}

extern "C" void zombie (int sig) {
 int pid = wait3(0, 0, NULL);
 while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
  //ctrlC
  struct sigaction sa;
  sa.sa_handler = disp;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &sa, NULL)) {
    perror("sigaction");
    exit(2);
  }

  //zombie elimination
  struct sigaction sa2;
  sa2.sa_handler = zombie;
  sigemptyset(&sa2.sa_mask);
  sa2.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD, &sa2, NULL)) {
    perror("sigaction");
    exit(2);
  }

  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
