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


int main(int argc, char **argv) {
  CraneContext *context;
  bool noCore = false;

  context = newContext();
  insertCommand(context->commandMap, createCommand("load", Crane_load, 1, false));
  
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
        context->failedLastCommand = foundCommand->handler(command, context);
      }
    } else {
      context->failedLastCommand = true;
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
