#ifndef commands_hpp
#define commands_hpp

#include "context.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

struct CraneCommand;

typedef int (*CraneCommandHandler)(CraneCommand *, CraneContext *);

enum class CraneCommandArgumentType {
  Boolean,
  File,
  String,
  Number
};

struct CraneCommandArgument {
public:
  std::string name; 
  CraneCommandArgumentType type;
  bool isOptional;

  CraneCommandArgument(std::string name, bool isOptional, CraneCommandArgumentType type)
    : name(name),
      type(type),
      isOptional(isOptional) {}
};

struct CraneCommandEntry {
public:
  std::string name;
  std::string description;
  bool isVariadic;
  bool requiresOpenFile;
  bool shouldOverride;
  int argumentCount;
  CraneCommandHandler handler;
  CraneCommandEntry *overridenEntry;
  std::vector<CraneCommandArgument*> arguments;

  CraneCommandEntry(std::string name, CraneCommandHandler handler, bool isVariadic)
    : name(name),
      isVariadic(isVariadic),
      handler(handler) {}
  
  inline void setCommandDescription(std::string desc) {
    this->description = desc;
  }

  inline void setCommandRequiresOpenFile(bool requiresOpenFile) {
    this->requiresOpenFile = requiresOpenFile;
  }

  inline void addArgument(std::string name, bool isOptional, CraneCommandArgumentType type) {
    auto arg = new CraneCommandArgument(name, isOptional, type);
    arguments.push_back(arg);
  }
};

#endif

