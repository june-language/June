#include "contributions.h"
#include <ctype.h>

contributableCommand(open) {
  if (context->openedFile != NULL) {
    context->openedFile = NULL;
  }

  FILE *presaveOpen = fopen(command->arguments[0], "rb+");
  if (presaveOpen == NULL) {
    perror("Failed to open the file");
    return 1;
  }

  context->openedFile->handle = presaveOpen;
  context->openedFile->filePath = command->arguments[0];

  return 0;
}

#define HashPrime 87178291199UL

static int Crane_MapHash(const char *s, const int buckets) {
  long hash = 0;
  const int stringLength = strlen(s);
  for (int i = 0; i < stringLength; i++) {
    hash += (long)pow(HashPrime, stringLength - (i + 1)) * s[i];
    hash = hash % buckets;
  }

  return (int)hash;
}

CraneMapEntry *contribFindEntry(CraneMap *map, char *name) {
  int entryHash = Crane_MapHash(name, map->size);

  int i = 0;
  while (true) {
    if ((entryHash + i) >= map->size)
      break;

    CraneMapEntry *entry = map->entries[entryHash + i];

    if (entry != NULL) {
      if (strcmp(entry->name, name) == 0) {
        return entry;
      }
    }
    i++;
  }

  return NULL;
}

contributableCommand(close) {
  if (context->openedFile == NULL && command->argumentCount == 0) {
    printf("There is no file currently opened.\n");
    return 1;
  }

  if (command->argumentCount == 0) {
    int res = fclose(context->openedFile->handle);
    if (res != 0) {
      perror("Failed to close file");
      return 1;
    }
    free(context->openedFile);
    context->openedFile = NULL;
  } else {
    char *alias = command->arguments[0];
    
    CraneOpenFile *entry = (CraneOpenFile*)contribFindEntry(context->fileMap, alias);
    if (!entry) {
      printf("No such file with the alias '%s' exists.\n", alias);
      return 1;
    }

    int res = fclose(entry->handle);
    if (res != 0) {
      perror("Failed to close file");
      return 1;
    }

    printf("Closed %s", alias);
  }

  return 0;
}

#define kReadBufferSize 1024
#define kHexDumpWidth 6

contributableCommand(dump) {
  char *format = command->arguments[0];

  if (strcmp(format, "hex") == 0) {
    char buf[kReadBufferSize];
    size_t bytesRead;

    while ((bytesRead = fread(buf, 1, kReadBufferSize, context->openedFile->handle)) != 0) {
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

    if (fseek(context->openedFile->handle, 0, SEEK_SET) != 0) {
      perror("Failed to reset file pointer");
      return 1;
    }
    return 0;
  } else {
    printf("Unknown format\n");
    return 1;
  }
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
  setContributionDescription(contributions, "Opens a file and automatically selects (if noSelect is not set to true)");
  addContributedArgument(contributions, "fileToOpen", CraneArgumentTypeFile, false);
  addContributedArgument(contributions, "alias", CraneArgumentTypeString, false);
  addContributedArgument(contributions, "noSelect", CraneArgumentTypeBoolean, true);

  contributeCommand(contributions, close, false);
  setContributionDescription(contributions, "Closes a file given an alias. Otherwise, it closes the currently open file");
  addContributedArgument(contributions, "alias", CraneArgumentTypeString, true);

  contributeCommand(contributions, dump, false);
  setContributionDescription(contributions, "Dumps the contents of a file as a set format");
  addContributedArgument(contributions, "format", CraneArgumentTypeString, false);

  contributeCommand(contributions, write, true);
  setContributionDescription(contributions, "Writes some data to a file with a given format");
  addContributedArgument(contributions, "format", CraneArgumentTypeString, false);

  return contributions;
}
