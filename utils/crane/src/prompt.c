#include "prompt.h"
#include "context.h"
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

void generatePrompt(CraneContext *context, char **buf) {
  /**
   * The expected prompt should be clean and a bit colorful:
   *
   * {cyan}crane {magenta}(openFile)? {white if didNotFail else red}>{reset}
   */

  int lengthOffset = 3; // '(' + ')' + ' '
  if (context->openedFile != NULL) {
    lengthOffset += strlen(context->openedFilePath);
  } else {
    lengthOffset = 0;
  }

  *buf = calloc(strlen(kColorCyan) * 3 + strlen("crane >") + lengthOffset, sizeof(char));
  
  // {cyan}crane
  strcat(*buf, kColorCyan);
  strcat(*buf, "crane ");

  // {magenta}(openFile)?
  if (context->openedFile != NULL) {
    strcat(*buf, kColorMagenta);
    strcat(*buf, "(");
    strcat(*buf, context->openedFilePath);
    strcat(*buf, ") ");
  }

  // {white if didNotFail else red}>{reset}
  strcat(*buf, context->failedLastCommand ? kColorRed : kColorWhite);
  strcat(*buf, "> ");
  strcat(*buf, kColorReset);
}

CraneCommand *inputCommand(CraneContext *context) {
  CraneCommand *command = calloc(1, sizeof(CraneCommand));

  int argumentLimit = 1;
  int argumentCount = 0;
  command->arguments = calloc(argumentLimit, sizeof(char *));

  char *promptBuf = NULL;
  generatePrompt(context, &promptBuf);

  char *comBuf = readline(promptBuf);

  if (!comBuf) {
    return NULL;
  }

  add_history(comBuf);

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
