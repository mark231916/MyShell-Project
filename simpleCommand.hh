#ifndef simplecommand_hh
#define simplecommand_hh

#include <string>
#include <vector>

struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;

  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  void print();
  std::string* envExpansion(std::string * argument);
  std::string* tildeExpansion(std::string * argument);
};

#endif
