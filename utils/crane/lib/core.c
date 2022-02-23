#include "contributions.h"
#include <ctype.h>

contributableCommand(open) {
  if (context->openedFile != NULL) {
    printf("There is already a file open! This will close the current file\n"
           "Are you sure? [y/n] ");

    int c;
    while ((c = getchar()) != '\n') {
      if (c == 'y' || c == 'Y') {
        if (fclose(context->openedFile) != 0) {
          perror("Failed to close file");
          return 1;
        }
        context->openedFile = NULL;
        context->openedFilePath = "";
        break;
      } else {
        printf("'%s' was not opened.", command->arguments[0]);
        return 0;
      }
    }
    // getchar() reads newline as well so we need to consume that
    // before it gets consumed in prompt.c
    getchar();
  }

  FILE *presaveOpen = fopen(command->arguments[0], "rb+");
  if (presaveOpen == NULL) {
    perror("Failed to open the file");
    return 1;
  }

  context->openedFile = presaveOpen;
  context->openedFilePath = command->arguments[0];

  return 0;
}

contributableCommand(close) {
  if (context->openedFile == NULL) {
    printf("There is no file currently opened.\n");
    return 1;
  }

  int res = fclose(context->openedFile);
  if (res != 0) {
    perror("Failed to close file");
    return 1;
  }
  context->openedFile = NULL;
  context->openedFilePath = "";

  return 0;
}

#define kReadBufferSize 1024
#define kHexDumpWidth 6

contributableCommand(dump) {
  if (context->openedFile == NULL) {
    printf("There is no file currently opened.\n");
    return 1;
  }

  char *format = command->arguments[0];

  if (strcmp(format, "hex") == 0) {
    char buf[kReadBufferSize];
    size_t bytesRead;

    while ((bytesRead = fread(buf, 1, kReadBufferSize, context->openedFile)) != 0) {
      int i = 0;
      
      char tempBuf[kReadBufferSize];
      for (int j = 0; j < kReadBufferSize; j++) {
        tempBuf[j] = '\0';
      }

      while (bytesRead != 0) {
        if ((i % kHexDumpWidth) == 0 && i != 0) {
          printf("      ");

          for (int j = 0; j < kHexDumpWidth; j++) {
            if (iscntrl(tempBuf[j])) {
              printf(".");
            } else {
              printf("%c", tempBuf[j]);
            }
            tempBuf[j] = '\0';
          }

          printf("\n");
        }

        printf("%02x ", buf[i] & 0xff);

        tempBuf[i % kHexDumpWidth] = buf[i];
        bytesRead--;
        i++;
      }

      if (i != 0) {
        if (i % kHexDumpWidth == 0) {
          printf("      ");
        } else {
          // What are these constants?
          // 3 - "%02x " this outputs a total of 3 characters
          // 6 - Constant number of spaces in "      "
          int k = ((kHexDumpWidth - (i % kHexDumpWidth)) * 3) + 6;
          for (int ip = 0; ip < k; ip++) {
            printf(" ");
          }
        }
        for (int j = 0; j < kHexDumpWidth; j++) {
          if (tempBuf[j] == '\0' && (j + 1) >= kHexDumpWidth) break;

          if (iscntrl(tempBuf[j])) {
            printf(".");
          } else {
            printf("%c", tempBuf[j]);
            fflush(stdout);
          }
          tempBuf[j] = '\0';
        }
      }
    }

    printf("\n");

    if (fseek(context->openedFile, 0, SEEK_SET) != 0) {
      perror("Failed to reset file pointer");
      printf("Use `rewind` to attempt a reset of the file pointer.\n");
      return 1;
    }
    return 0;
  } else {
    printf("Unknown format\n");
    return 1;
  }
}

contributableCommand(rewind) {
  if (context->openedFile == NULL) {
    printf("There is no file currently opened.\n");
    return 1;
  }

  if (fseek(context->openedFile, 0, SEEK_SET) != 0) {
    perror("Failed to reset file pointer");
    return 1;
  }

  return 0;
}

contributableCommand(write) {
  if (context->openedFile == NULL) {
    printf("There is no file currently opened.\n");
    return 1;
  }

  return 0;
}

CraneContributedCommands *Crane_contributedCommands() {
  CraneContributedCommands *contributions = initContributedCommands();

  contributeCommand(contributions, open, false);
  addContributedArgument(contributions, "file", CraneArgumentTypeFile);

  contributeCommand(contributions, close, false);
  contributeCommand(contributions, rewind, false);

  contributeCommand(contributions, dump, false);
  addContributedArgument(contributions, "format", CraneArgumentTypeString);

  contributeCommand(contributions, write, true);
  addContributedArgument(contributions, "format", CraneArgumentTypeString);


  return contributions;
}
