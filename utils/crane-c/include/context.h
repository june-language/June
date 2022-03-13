#ifndef context_h
#define context_h

#include "commands.h"
#include "crane_map.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _CraneOpenFile {
  char *filePath;
  char *alias;
  FILE *handle;
} CraneOpenFile;

typedef struct _CraneContext {
  int lastCommandResult;
  CraneOpenFile *openedFile;
  CraneMap *fileMap;
  CraneMap *dynHandleMap;
  CraneCommandMap *commandMap;
} CraneContext;

CraneContext *newContext();

#endif
