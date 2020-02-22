/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>


#include "command.hh"
#include "shell.hh"


Command::Command() {
  // Initialize a new vector of Simple Commands
  _simpleCommands = std::vector<SimpleCommand *>();

  _outFile = NULL;
  _inFile = NULL;
  _errFile = NULL;
  _background = false;
  _append = false;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
  // add the simple command to the vector
  _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
  // deallocate all the simple commands in the command vector
  for (auto simpleCommand : _simpleCommands) {
     delete simpleCommand;
    }

  // remove all references to the simple commands we've deallocated
  // (basically just sets the size to 0)
  _simpleCommands.clear();

  if ( _outFile ) {
      delete _outFile;
  }
  _outFile = NULL;

  if ( _inFile ) {
      delete _inFile;
  }
  _inFile = NULL;

  if ( _errFile ) {
      delete _errFile;
  }
  _errFile = NULL;

  _background = false;
}

void Command::print() {
  printf("\n\n");
  printf("              COMMAND TABLE                \n");
  printf("\n");
  printf("  #   Simple Commands\n");
  printf("  --- ----------------------------------------------------------\n");

  int i = 0;
  // iterate over the simple commands and print them nicely
  for ( auto & simpleCommand : _simpleCommands ) {
      printf("  %-3d ", i++ );
      simpleCommand->print();
  }

  printf( "\n\n" );
  printf( "  Output       Input        Error        Background\n" );
  printf( "  ------------ ------------ ------------ ------------\n" );
  printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
  printf( "\n\n" );
}

void Command::execute() {
  // Don't do anything if there are no simple commands
  if ( _simpleCommands.size() == 0 ) {
      Shell::prompt();
      return;
  }
  if(_simpleCommands[0]->_arguments[0]->compare("exit") == 0) {
    printf("Good bye!!\n");
    exit(1);
  }


  // Print contents of Command data structure
  //print();

  // Add execution here
  // For every simple command fork a new process
  // Setup i/o redirection
  // and call exec
    
  //save original io
  int tempIn = dup(0);
  int tempOut = dup(1);
  int tempErr = dup(2);

  int fdin;
  //open input file if exist
  if(_inFile) 
    fdin = open(_inFile->c_str(), O_RDONLY, 0600);
//  if(fdin < 0) {
  //  perror("inFile");
    //exit(1);
  //}
  else //otherwise use the default one
    fdin = dup(tempIn);

  int fderr;
  //open error file if exist
  if(_errFile) {
    if(_append) //open error file in append mode
      fderr = open(_errFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
    else //otherwise open in write mode
      fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
  }
  else //otherwise use the default error file
    fderr = dup(tempErr);
  
  dup2(fderr,2);
  close(fderr);

  int fdout;
  int ret;
  for (size_t i = 0; i < _simpleCommands.size(); i++) {
    if(fdin == -1) {
      perror("inFile");
      break;
    }
    //set env
    if(_simpleCommands[i]->_arguments[0]->compare("setenv") == 0) {
      setenv(_simpleCommands[i]->_arguments[1]->c_str(),
      _simpleCommands[i]->_arguments[2]->c_str(), 1);
      clear();
      Shell::prompt();
      return;
    }

    //unset env
    if(_simpleCommands[i]->_arguments[0]->compare("unsetenv") == 0) {
      unsetenv(_simpleCommands[i]->_arguments[1]->c_str());
      clear();
      Shell::prompt();
      return;
    }

    //cd
    if(_simpleCommands[i]->_arguments[0]->compare("cd") == 0) {
      int err;
      if(_simpleCommands[i]->_arguments.size() == 1)
	err = chdir(getenv("HOME"));
      else
	err = chdir(_simpleCommands[i]->_arguments[1]->c_str());
      if(err < 0) {
	char *msg = (char*)malloc(50);
	char *temp = "cd: can't cd to ";
	strcat(msg, temp);
	strcat(msg, _simpleCommands[i]->_arguments[1]->c_str());
	//printf("%s\n", msg);
	perror(msg);
      }
      clear();
      Shell::prompt();
      return;
    }
   //redirect input and error
    dup2(fdin, 0);
    //dup2(fderr, 2);
    close(fdin);
    //close(fderr);
    
    //if it's the last simple command
    if (i == _simpleCommands.size()-1) {
      if(_outFile) { //open output file
	if(_append)
	  fdout = open(_outFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
	else
	  fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
      }
      else //otherwise use the default one
	fdout = dup(tempOut);
    }
    else {
      //if it's not the last, creat pipe
      int fdpipe[2];
      pipe(fdpipe);
      fdout = fdpipe[1];
      fdin = fdpipe[0];
    }

    //redirect output
    dup2(fdout, 1);
    close(fdout);

    //creat child process
    ret = fork();
    if(ret == 0) {
      if(_simpleCommands.at(i)->_arguments.size() == 0) {
	execvp(_simpleCommands.at(i)->_arguments.at(0)->c_str(), NULL);
	_exit(1);
      }
      
      //printenv
      if(_simpleCommands.at(i)->_arguments[0]->compare("printenv") == 0) {
	extern char ** environ;
	while (*environ) {
	  printf("%s\n", *environ);
	  environ++;
	}
	_exit(1);
      }

      //source
      if(_simpleCommands.at(i)->_arguments[0]->compare("source") == 0) {
	fdin = open(_simpleCommands.at(i)->_arguments[1]->c_str(), O_RDONLY, 0600);
	dup2(fdin, 0);
	close(fdin);
	execvp("/proc/self/exe", NULL);
      }

      char** temp = (char**)malloc((_simpleCommands.at(i)->_arguments.size()+1)*sizeof(char*));
      for(size_t j = 0; j < _simpleCommands.at(i)->_arguments.size(); j++) {
	//printf("%s\n", _simpleCommands.at(i)->_arguments.at(j)->c_str());
	if(_simpleCommands.at(i)->_arguments.at(j)->length() != 0)
	  temp[j] = (char*) _simpleCommands.at(i)->_arguments.at(j)->c_str();
      }
      temp[_simpleCommands.at(i)->_arguments.size()] = NULL;

      //printf("test");
      //printf("%s\n", _simpleCommands.at(i)->_arguments.at(1)->c_str());
      execvp(_simpleCommands.at(i)->_arguments.at(0)->c_str(), temp);
      //printf("%s", temp);
      perror("execvp");
      _exit(1);
    }

  }

  //restore io
  dup2(tempIn, 0);
  dup2(tempOut, 1);
  dup2(tempErr, 2);
  close(tempIn);
  close(tempOut);
  close(tempErr);

  //wait for last command
  if(!_background)
    waitpid(ret, NULL, 0);


  // Clear to prepare for next command
  clear();

  // Print new prompt
  Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
