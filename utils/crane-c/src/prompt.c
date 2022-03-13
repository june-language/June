#include "prompt.h"
#include "context.h"
#include <ctype.h>
#include <readline/history.h>
#include <readline/readline.h>

CraneString *newString() {
  CraneString *str = calloc(1, sizeof(CraneString));
  str->buffer = calloc(1, sizeof(char));
  str->length = 0;

  return str;
}

void appendChar(CraneString *string, char c) {
  string->buffer = realloc(string->buffer, (string->length + 1) * sizeof(char));
  string->buffer[string->length++] = c;
}

char *asCString(CraneString *string) {
  char *buf = calloc(string->length + 1, sizeof(char));
  strcpy(buf, string->buffer);
  buf[string->length] = '\0';

  return buf;
}

void clearString(CraneString *string) {
  free(string->buffer);
  string->buffer = calloc(1, sizeof(char));
  string->length = 0;
}

void generatePrompt(CraneContext *context, char **buf) {
  /**
   * The expected prompt should be clean and a bit colorful:
   *
   * {cyan}crane {magenta}(openFile)? {white if didNotFail else red}>{reset}
   */

  int lengthOffset = 3; // '(' + ')' + ' '
  if (context->openedFile != NULL) {
    lengthOffset += strlen(context->openedFile->alias);
  } else {
    lengthOffset = 0;
  }

  *buf = calloc(strlen("\33[2K\r") + strlen(kColorCyan) * 3 +
                    strlen("crane >") + lengthOffset,
                sizeof(char));

  // \33[2K\r
  strcat(*buf, "\33[2K\r");

  // {cyan}crane
  strcat(*buf, kColorCyan);
  strcat(*buf, "crane ");

  // {magenta}(openFile)?
  if (context->openedFile != NULL) {
    strcat(*buf, kColorMagenta);
    strcat(*buf, "(");
    strcat(*buf, context->openedFile->alias);
    strcat(*buf, ") ");
  }

  // {white if didNotFail else red}>{reset}
  strcat(*buf, context->lastCommandResult != 0 ? kColorRed : kColorWhite);
  strcat(*buf, "> ");
  strcat(*buf, kColorReset);
}

CraneCommand *parseCommand(char *buffer) {
  int bufferLength = strlen(buffer);
  CraneCommand *command = calloc(1, sizeof(CraneCommand));

  int argumentLimit = 1;
  int argumentCount = -1;
  command->arguments = calloc(argumentLimit, sizeof(char *));

  bool escapeNext = false;
  bool inString = false;
  int i = 0;
  int lastQuoteIndex = 0;
  CraneString *argumentBuffer = newString();
  char currentChar;

  while (bufferLength) {
    currentChar = buffer[i++];

    if (escapeNext) {
      appendChar(argumentBuffer, currentChar);
      continue;
    }

    if (currentChar == '\\' && !escapeNext) {
      escapeNext = true;
    }

    if (currentChar == '"' || currentChar == '\'') {
      if (!inString) {
        inString = true;
        lastQuoteIndex = i;
        continue;
      } else {
        inString = false;

        if (argumentCount == -1) {
          command->name = asCString(argumentBuffer);
          clearString(argumentBuffer);
          argumentCount++;
        } else {
          if (argumentCount == argumentLimit) {
            command->arguments =
                realloc(command->arguments, sizeof(char *) * (++argumentLimit));
          }
          command->arguments[argumentCount++] = asCString(argumentBuffer);
          clearString(argumentBuffer);
        }
      }
    }

    if (isspace(currentChar) && !inString) {
      if (argumentCount == -1) {
        command->name = asCString(argumentBuffer);
        clearString(argumentBuffer);
        argumentCount++;
      } else {
        if (argumentCount == argumentLimit) {
          command->arguments =
              realloc(command->arguments, sizeof(char *) * (++argumentLimit));
        }
        command->arguments[argumentCount++] = asCString(argumentBuffer);
        clearString(argumentBuffer);
      }
      continue;
    }

    appendChar(argumentBuffer, currentChar);
  }

  if (inString) {
    printf("Expected closing quote to string at %d\n", lastQuoteIndex);
    return NULL;
  }

  return command;
}

CraneCommand *inputCommand(CraneContext *context) {
  char *promptBuffer = NULL;
  generatePrompt(context, &promptBuffer);

  char *commandBuffer = readline(promptBuffer);

  if (!commandBuffer) {
    return NULL;
  }

  add_history(commandBuffer);

  return parseCommand((char *)commandBuffer);
}
