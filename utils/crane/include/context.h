#ifndef context_h
#define context_h

#include "commands.h"
#include "crane_map.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _CraneContext {
  int lastCommandResult;
  char *openedFilePath;
  FILE *openedFile;
  CraneMap *dynHandleMap;
  CraneCommandMap *commandMap;
} CraneContext;

CraneContext *newContext();

#endif
