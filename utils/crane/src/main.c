/**
 * To quickly outline the Plugin API for Crane:
 *
 * You need to have a dynamic library compiled with the following function:
 *
 * CraneContributedCommands *Crane_contributedCommands();
 *
 * Use the `CraneContributedCommands` type to contribute commands to Crane.
 * Loading the library is provided by Crane (AKA Crane Core):
 *
 * load </path/to/contributedLibrary>
 */

#include "commands.h"
#include "context.h"
#include "contributions.h"
#include "crane_map.h"
#include "prompt.h"
#include <dlfcn.h>
#include <unistd.h>
#include <termios.h>
#include <readline/readline.h>

int Crane_load(CraneCommand *command, CraneContext *context) {
  char *fileToLoad = command->arguments[0];
  
  if (StringEquals(fileToLoad, "Core")) {
    fileToLoad = "extern/core.so";
  }

  if (access(fileToLoad, R_OK) != 0) {
    printf("The file '%s' doesn't exist.\n", fileToLoad);
    return 1;
  }

  if (findEntry(context->dynHandleMap, fileToLoad) != NULL) {
    printf("The module '%s' has already been loaded!\n", command->arguments[0]);

    // Despite this being an "error", the file was apparently loaded
    // successfully so there is no need to emit and error.
    return 0;
  }

  void *handle = dlopen(fileToLoad, RTLD_NOW | RTLD_GLOBAL);
  if (handle == NULL) {
    printf("Failed to open the module.\n%s\n", dlerror());
    dlclose(handle);
    return 1;
  }

  insertEntry(context->dynHandleMap, fileToLoad, handle);

  CraneContributionInitialiser init =
      dlsym(handle, "Crane_contributedCommands");

  CraneContributedCommands *contrib = init();
  for (int i = 0; i < contrib->contributedCount; i++) {
    if (contrib->contributedCommands[i] == NULL)
      continue;

    insertCommand(context->commandMap, contrib->contributedCommands[i]);
  }

  return 0;
}

int Crane_QMark(CraneCommand *command, CraneContext *context) {
  printf("%d\n", context->lastCommandResult);

  return 0;
}

int Crane_help(CraneCommand *command, CraneContext *context) {
  CraneCommandEntry *entry;
  if ((entry = findCommand(context->commandMap, command->arguments[0])) != NULL) {
    printf("%s - %d arguments\n\n", command->arguments[0], command->argumentCount);
    for (int i = 0; i < command->argumentCount; i++) {
      CraneCommandArgument *argument = entry->arguments[i];
      printf("%s - ", argument->name);
      
      switch (argument->type) {
      case CraneArgumentTypeBoolean:
        printf("Boolean [true/false]\n");
        break;
      case CraneArgumentTypeFile:
        printf("File\n");
        break;
      case CraneArgumentTypeNumber:
        printf("Number\n");
        break;
      case CraneArgumentTypeString:
        printf("String\n");
        break;
      }
    }
    printf("\n");
  } else {
    printf("Command '%s' does not exist.\n", command->arguments[0]);
    return 1;
  }

  return 0;
}

static CraneContext *context;

char **completionEngine(const char *text, int start, int end) {
  

  CraneCommand *parsedCommand = parseCommand(text);
  CraneCommandEntry *command = NULL;
  if ((command = findCommand(context->commandMap, parsedCommand->name)) != NULL) {
    if (command->argumentCount > parsedCommand->argumentCount) {
      // TODO: Implement Argument Completion
      return NULL;
    }
  } else {
    // TODO: Implement Levenshtein Distance algorithm
  }

  rl_attempted_completion_over = 1;
  return NULL;
}

int main(int argc, char **argv) {
  rl_attempted_completion_function = completionEngine;

  bool noCore = false;
  context = newContext();
  
  CraneCommandEntry *loadCommand = createCommand("load", Crane_load, false);
  addCommandArgument(loadCommand, createCommandArgument("module", CraneArgumentTypeFile));
  insertCommand(context->commandMap, loadCommand);
  
  CraneCommandEntry *helpCommand = createCommand("help", Crane_help, false);
  addCommandArgument(helpCommand, createCommandArgument("command", CraneArgumentTypeString));
  insertCommand(context->commandMap, helpCommand);
  
  insertCommand(context->commandMap, createCommand("?", Crane_QMark, false));

  
  for (int i = 1; i < argc; i++) {
    if (StringEquals(argv[i], "--no-core")) {
      noCore = true;
    }
  }

  if (!noCore) {
    CraneCommandEntry *loadCommand = findCommand(context->commandMap, "load");
    if (loadCommand == NULL) {
      printf("Failed to initialise Crane. Could not load Core library.\n");
      return 1;
    }

    CraneCommand *coreLoad = calloc(1, sizeof(CraneCommand));

    coreLoad->arguments = calloc(1, sizeof(char *));
    coreLoad->arguments[0] = "extern/core.so";

    int result = loadCommand->handler(coreLoad, context);
    if (result != 0) {
      printf("Failed to initialise Crane. Core library not found in 'extern/core.so'\n");
      return 1;
    }

    free(coreLoad);
  }

  while (true) {
    CraneCommand *command = inputCommand(context);

    if (command == NULL) {
      printf("\nGoodbye.\n");
      return 1;
    }

    if (StringEquals(command->name, "quit") ||
        StringEquals(command->name, "exit")) {
      free(command);
      break;
    }

    CraneCommandEntry *foundCommand =
        findCommand(context->commandMap, command->name);
    if (foundCommand != NULL) {
      if (foundCommand->argumentCount != command->argumentCount && !foundCommand->isVariadic) {
        printf("The '%s' command requires %d argument%s\n", foundCommand->name, foundCommand->argumentCount, foundCommand->argumentCount == 1 ? "" : "s");
      } else {
        context->lastCommandResult = foundCommand->handler(command, context);
      }
    } else {
      context->lastCommandResult = 1;
      printf("No such command exists.");

      if (noCore) {
        printf(" Did you forget to load Core?");
      }
      printf("\n");
    }
    printf("\n");

    free(command);
    command = NULL;
  }

  printf("Goodbye!\n");

  // Cleanup Steps
  if (context->openedFile != NULL)
    fclose(context->openedFile);
  for (int i = 0; i < context->dynHandleMap->count; i++) {
    if (context->dynHandleMap->entries[i] == NULL)
      continue;

    dlclose(context->dynHandleMap->entries[i]);
  }
  // NOTE: This caused a segfault (but not during program run but after which is
  // odd).
  // deleteMap(context->dynHandleMap);
  // free(context->commandMap);
  // free(context);

  return 0;
}
