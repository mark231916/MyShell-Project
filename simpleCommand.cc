#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <pwd.h>
#include "simpleCommand.hh"

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

void SimpleCommand::insertArgument( std::string* argument ) {
  // simply add the argument to the vector
  if(argument->length() != 0) {
    argument = envExpansion(argument);
    argument = tildeExpansion(argument);
  }
  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}

std::string* SimpleCommand::envExpansion(std::string* argument) {
  if(argument->find('$') == std::string::npos)
    return argument;

  //if contains $, replace all $ in it
  //std::string temp = argument->erase(std::remove(argument->begin(), argument->end(), '$'),
  //argument->end());
  std::string temp(argument->c_str());
  if(temp.at(1) == '(')
    return argument;

  
  //use a new string to hold the output
  std::string output;
  //std::size_t found1 = temp.find("{");
  //std::size_t found2 = temp.find("}");
  std::size_t found;
  size_t i = 0;
  while(i < temp.length()) {
    if(temp.at(i) != '{' && temp.at(i) != '}' && temp.at(i) != '$')
      output += temp.at(i);
    if(temp.at(i) == '{') {
      found = temp.find("}", i+1);
      if(found != i+1) {
	std::string str = temp.substr(i+1, found-i-1);
	if(str == "$") {
	  int pid = getpid();
	  output += std::to_string(pid);
	}
	else if(str == "SHELL") {
	  char buffer[PATH_MAX+1];
	  char *path = realpath("shell", buffer);
	  if(path)
	    output += path;
	  else {
	    path = realpath("../shell", buffer);
	    output += path;
	  }
	}
	else {
	  char * env = getenv(str.c_str());
	  //std::string* envString = new std::string(env);
	  if(env)
	    output += env;
	  else
	    output += "No such variable";
	}
	i = found + 1;
      }
    } else
	i++;
  }
  return new std::string(output.c_str());  
}

std::string* SimpleCommand::tildeExpansion(std::string* argument) {
  if(argument->at(0) != '~')
    return argument;
  std::string temp(argument->c_str());
  std::string output;

  if(temp.length() == 1 && temp == "~") {
    output += getenv("HOME");
    return new std::string(output.c_str());
  }

  if(temp.at(1) == '/') {
    output += getenv("HOME");
    output += temp.substr(1); 
    return new std::string(output.c_str());
  }

  size_t found = temp.find("/", 0);
  std::string name;
  if(found != std::string::npos)
    name = temp.substr(1, found-1);
  else
    name = temp.substr(1);
  output += getpwnam(name.c_str())->pw_dir;
  if(found != std::string::npos && found != temp.length()-1)
    output += temp.substr(found);

  return new std::string(output.c_str());
}
