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

int Crane_load(CraneCommand *command, CraneContext *context) {
  if (command->argumentCount == 0) {
    printf("You must specify a file to load.\n");
    return 1;
  }

  char *fileToLoad = command->arguments[0];
  if (access(fileToLoad, R_OK) != 0) {
    printf("The file '%s' doesn't exist.\n", fileToLoad);
    return 1;
  }

  void *handle = dlopen(fileToLoad, RTLD_NOW | RTLD_GLOBAL);
  if (handle == NULL) {
    printf("Failed to open the library.\n%s\n", dlerror());
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

int main() {
  CraneContext *context = newContext();
  insertCommand(context->commandMap, createCommand("load", Crane_load, 1));

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
      context->failedLastCommand = foundCommand->handler(command, context);
    } else {
      context->failedLastCommand = true;
      printf("No such command exists.\n");
    }
    printf("\n");

    free(command);
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
  deleteMap(context->dynHandleMap);
  free(context->commandMap);
  free(context);

  return 0;
}
