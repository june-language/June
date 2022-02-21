#ifndef prompt_h
#define prompt_h

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define kCommandBufSize 1024
#define StringEquals(str1, str2) (strcmp(str1, str2) == 0)

#define kColorRed "\033[31m"
#define kColorGreen "\033[32m"
#define kColorYellow "\033[33m"
#define kColorBlue "\033[34m"
#define kColorMagenta "\033[35m"
#define kColorCyan "\033[36m"
#define kColorWhite "\033[37m"
#define kColorReset "\033[0m"

typedef struct _CraneContext CraneContext;

typedef struct _CraneCommand {
  char *name;
  char **arguments;
  int argumentCount;
} CraneCommand;

CraneCommand *inputCommand(CraneContext *context);

#endif
