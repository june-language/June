#ifndef prompt_hpp
#define prompt_hpp

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#define kColorRed "\033[31m"
#define kColorGreen "\033[32m"
#define kColorYellow "\033[33m"
#define kColorBlue "\033[34m"
#define kColorMagenta "\033[35m"
#define kColorCyan "\033[36m"
#define kColorWhite "\033[37m"
#define kColorReset "\033[0m"

struct CraneContext;

struct CraneCommand {
public:
  std::string name;
  std::vector<std::string> arguments;
  CraneCommand(std::string name, std::vector<std::string> arguments)
    : name(name), arguments(arguments) {}

  static CraneCommand *fromUser(CraneContext *context);
  static CraneCommand *parseCommand(std::string buffer);
};

#endif

