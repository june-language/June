#ifndef prompt_h
#define prompt_h

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define kCommandBufSize 1024
#define StringEquals(str1, str2) (strcmp(str1, str2) == 0)

typedef struct _CraneContext CraneContext;

typedef struct _CraneCommand {
  char *name;
  char **arguments;
  int argumentCount;
} CraneCommand;

CraneCommand *inputCommand(CraneContext *context);

#endif
