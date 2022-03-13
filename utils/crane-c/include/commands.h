#ifndef commands_h
#define commands_h

#include "crane_map.h"
#include "prompt.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _CraneContext CraneContext;

typedef int (*CraneCommandHandler)(CraneCommand *, CraneContext *);

typedef enum {
  CraneArgumentTypeBoolean,
  CraneArgumentTypeFile,
  CraneArgumentTypeString,
  CraneArgumentTypeNumber
} CraneCommandArgumentType;

typedef struct {
  char *name;
  bool isOptional;
  CraneCommandArgumentType type;
} CraneCommandArgument;

typedef struct _CraneCommandEntry {
  char *name;
  char *description;
  bool isVariadic;
  bool requiresOpenFile;
  bool shouldOverride;
  int argumentCount;
  CraneCommandHandler handler;
  struct _CraneCommandEntry *overridenEntry;
  CraneCommandArgument **arguments;
} CraneCommandEntry;

typedef struct _CraneCommandMap {
  int size;
  int count;
  CraneCommandEntry **commands;
} CraneCommandMap;

// Sets the argument count to 0 by default
CraneCommandEntry *createCommand(char *name, CraneCommandHandler handler, bool isVariadic);
void setCommandDescription(CraneCommandEntry *entry, char *description);
void setCommandRequiresOpenFile(CraneCommandEntry *entry, bool requiresOpenFile);

CraneCommandArgument *createCommandArgument(char *name, CraneCommandArgumentType type, bool isOptional);
void addCommandArgument(CraneCommandEntry *entry, CraneCommandArgument *argument);

CraneCommandMap *createCommandMap();
CraneCommandEntry *findCommand(CraneCommandMap *map, char *name);
bool insertCommand(CraneCommandMap *map, CraneCommandEntry *command);
CraneCommandEntry **aggregateCommands(CraneCommandMap *map);
void deleteCommandMap(CraneCommandMap *map);

#endif
