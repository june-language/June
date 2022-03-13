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
#include <readline/readline.h>
#include <termios.h>
#include <unistd.h>

static char *CraneCoreLocation = "extern/core.so";
static char *CraneStagingLocation = "extern/staging.so";

int Crane_load(CraneCommand *command, CraneContext *context) {
  char *fileToLoad = command->arguments[0];

  if (StringEquals(fileToLoad, "Core")) {
    fileToLoad = "extern/core.so";
  } else if (StringEquals(fileToLoad, "Staging")) {
    fileToLoad = "extern/staging.so";
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
  printf("%d - %s\n", context->lastCommandResult, context->lastCommandResult == 0 ? "Successful" : "Failed");

  return 0;
}

int Crane_help(CraneCommand *command, CraneContext *context) {
  CraneCommandEntry *entry;
  if ((entry = findCommand(context->commandMap, command->arguments[0])) !=
      NULL) {
    printf("%s - Accepts %d%s argument%s\n", command->arguments[0],
           entry->argumentCount, entry->isVariadic ? "+" : "",
           entry->argumentCount == 1 ? "" : "s");

    if (entry->description) {
      printf("\n%s\n", entry->description);
    } else {
      printf("\nThis command doesn't have a description\n");
    }

    if (entry->argumentCount)
      printf("\n");

    for (int i = 0; i < entry->argumentCount; i++) {
      CraneCommandArgument *argument = entry->arguments[i];
      printf("%s - ", argument->name);

      switch (argument->type) {
      case CraneArgumentTypeBoolean:
        printf("Boolean\n");
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

  } else {
    printf("Command '%s' does not exist.\n", command->arguments[0]);
    return 1;
  }

  return 0;
}

static CraneContext *context;

// TODO: tweak this setting a bit to find the "sweet spot"
#define kMaximumLevenshteinDistance 3

char *commandCompletionEngine(const char *text, int state) {
  static int index;
  static CraneCommandEntry **commands;
  CraneCommandEntry *command;

  if (state == 0) {
    index = 0;
    commands = aggregateCommands(context->commandMap);
  }

  while ((command = commands[index++])) {
    if (strncmp(command->name, text, strlen(text)) == 0) {
      return strdup(command->name);
    }
  }

  return NULL;
}

typedef struct _ArgumentCompletionData {
  char *text;
  CraneCommandArgumentType type;
} ArgumentCompletionData;

char *argumentCompletionEngine(const char *text, int state) {
  static char *opts[] = {"true", "false"};
  static int index = 0;
  char *opt;

  if (state == 0)
    index = 0;

  while ((opt = opts[index++])) {
    if (strncmp(text, text, strlen(text)) == 0) {
      return strdup(opt);
    }
  }

  return NULL;
}

char **completionGenerator(const char *text, int start, int end) {
  rl_attempted_completion_over = 1;

  CraneCommand *command = parseCommand((char *)text);
  if (!command) {
    free(command);
    return NULL;
  }

  if (command->argumentCount == 0) {
    return rl_completion_matches(text, commandCompletionEngine);
  } else {
    CraneCommandEntry *possibleCommand =
        findCommand(context->commandMap, command->name);
    if (!possibleCommand) {
      return NULL;
    }

    if (possibleCommand->argumentCount <= command->argumentCount) {
      return NULL;
    }

    if (possibleCommand->arguments[command->argumentCount]->type ==
        CraneArgumentTypeFile) {
      rl_attempted_completion_over = 0; // Re-enable file completion
    } else if (possibleCommand->arguments[command->argumentCount]->type ==
               CraneArgumentTypeBoolean) {
      return rl_completion_matches(text, argumentCompletionEngine);
    }

    return NULL;
  }
}

void cleanup() {
  // Cleanup Steps
  if (context->openedFile != NULL)
    fclose(context->openedFile->handle);
  for (int i = 0; i < context->fileMap->count; i++) {
    if (context->fileMap->entries[i] == NULL)
      continue;

    fclose(context->fileMap->entries[i]);
  }
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
}

void printHelp() {
  printf("Crane v1.0.0 - (c) Jules Amalie\n");
  printf("\n");
  printf("OPTIONS\n");
  printf("    --no-core                       Disregards loading the Core module\n");
  printf("    --staging                       Loads the Staging module\n");
  printf("    --core </path/to/core>          Sets the location of the Core module\n");
  printf("    --staging </path/to/staging>    Sets the location of the Staging module\n");
}

int main(int argc, char **argv) {
  rl_attempted_completion_function = completionGenerator;
  atexit(cleanup);

  bool noCore = false;
  context = newContext();

  CraneCommandEntry *loadCommand = createCommand("load", Crane_load, false);
  setCommandDescription(loadCommand, "Loads a module from a dynamic library");
  addCommandArgument(loadCommand, createCommandArgument(
                                      "module", CraneArgumentTypeFile, false));
  insertCommand(context->commandMap, loadCommand);

  CraneCommandEntry *helpCommand = createCommand("help", Crane_help, false);
  setCommandDescription(helpCommand, "Prints this help text for this command and many others");
  addCommandArgument(
      helpCommand,
      createCommandArgument("command", CraneArgumentTypeString, false));
  insertCommand(context->commandMap, helpCommand);

  insertCommand(context->commandMap, createCommand("?", Crane_QMark, false));
  setCommandDescription(findCommand(context->commandMap, "?"), "Prints the result of the last command and if it failed or not");

  for (int i = 1; i < argc; i++) {
    if (StringEquals(argv[i], "--no-core")) {
      noCore = true;
    } else if (StringEquals(argv[i], "--staging")) {
      CraneCommandEntry *loadCommand = findCommand(context->commandMap, "load");
      if (loadCommand == NULL) {
        printf("Failed to initialise Crane. Could not load Staging library.\n");
        return 1;
      }

      CraneCommand *stagingLoad = calloc(1, sizeof(CraneCommand));

      stagingLoad->arguments = calloc(1, sizeof(char *));
      stagingLoad->arguments[0] = "extern/staging.so";

      int result = loadCommand->handler(stagingLoad, context);
      if (result != 0) {
        printf("Failed to initialise Crane. Staging library not found in "
               "'extern/staging.so'\n");
        return 1;
      }

      free(stagingLoad);
    } else if (StringEquals(argv[i], "--help") || StringEquals(argv[i], "-h")) {
      printHelp();

      return 0;
    } else if (StringEquals(argv[i], "--core")) {
      if ((i + 1) >= argc) {
        printf("Expected argument to --core\n");

        return 1;
      }
      CraneCoreLocation = argv[++i];
    } else if (StringEquals(argv[i], "--staging")) {
      if ((i + 1) >= argc) {
        printf("Expected argument to --staging\n");

        return 1;
      }
      CraneStagingLocation = argv[++i];
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
      printf("Failed to initialise Crane. Core library not found in "
             "'extern/core.so'\n");
      return 1;
    }

    free(coreLoad);
  }

  while (true) {
    CraneCommand *command = inputCommand(context);

    if (command == NULL) {
      continue;      
    }

    if (StringEquals(command->name, "quit") ||
        StringEquals(command->name, "exit")) {
      free(command);
      break;
    }

    CraneCommandEntry *foundCommand =
        findCommand(context->commandMap, command->name);
    if (foundCommand != NULL) {
      if ((foundCommand->argumentCount != command->argumentCount &&
           !foundCommand->isVariadic) ||
          (foundCommand->isVariadic &&
           foundCommand->argumentCount < command->argumentCount)) {
        printf("The '%s' command requires %s%d argument%s\n",
               foundCommand->name, foundCommand->isVariadic ? "at least " : "",
               foundCommand->argumentCount,
               foundCommand->argumentCount == 1 ? "" : "s");
      } else if (foundCommand->requiresOpenFile && context->openedFile == NULL) {
        printf("The '%s' command requires a file to be open and selected, there is none.\n", foundCommand->name);
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

  return 0;
}
