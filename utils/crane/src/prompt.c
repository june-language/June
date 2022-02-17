#include "prompt.h"
#include "context.h"

char *_inputCommand(CraneContext *context) {
  char *buf = calloc(1, kCommandBufSize);
  int i = 0;
  int c;

  /**
   * The expected prompt should be clean and a bit colorful:
   *
   * {cyan}crane {magenta}(openFile)?{white if didNotFail else red}>{reset}
   */
  printf("\033[36mcrane ");
  // TODO: add file context support
  // if (context->openedFile != NULL) {
  //   // printf("(%s) ", context->openedFile->)
  // }
  if (context->failedLastCommand) {
    printf("\033[31m> \033[0m");
  } else {
    printf("\033[37m> \033[0m");
  }

  while (true) {
    if ((c = getchar()) == 0)
      break;

    if (c == '\r' || c == '\n') {
      return buf;
    }

    buf[i++] = c;
  }

  return NULL;
}

CraneCommand *inputCommand(CraneContext *context) {
  CraneCommand *command = calloc(1, sizeof(CraneCommand));

  int argumentLimit = 1;
  int argumentCount = 0;
  command->arguments = calloc(argumentLimit, sizeof(char *));

  char *comBuf = _inputCommand(context);

  if (!comBuf) {
    return NULL;
  }

  command->name = strtok(comBuf, " \t");
  char *argBuf = NULL;
  while ((argBuf = strtok(NULL, " \t")) != NULL) {
    if (argumentCount == argumentLimit) {
      command->arguments = realloc(command->arguments, argumentLimit * 2);

      if (command->arguments == NULL) {
        return NULL;
      }
    }

    command->arguments[argumentCount++] = argBuf;
  }

  command->argumentCount = argumentCount;

  return command;
}
