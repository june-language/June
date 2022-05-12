#ifndef context_hpp
#define context_hpp

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>

struct CraneCommandEntry;

struct CraneOpenFile {
public:
  std::string filePath;
  std::string alias;
  FILE *handle;

  CraneOpenFile(std::string filePath, std::string alias, FILE *handle)
    : filePath(filePath), alias(alias), handle(handle) {}
};

struct CraneContext {
public:
  int lastCommandResult;
  CraneOpenFile *openedFile;
  std::map<std::string, CraneOpenFile*> fileMap;
  std::map<std::string, void*> sharedHandleMap;
  std::map<std::string, CraneCommandEntry*> commandMap;

  CraneContext()
    : lastCommandResult(0),
      openedFile(nullptr),
      fileMap(),
      sharedHandleMap(),
      commandMap() {}
};

#endif

