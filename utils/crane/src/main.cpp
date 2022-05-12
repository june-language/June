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

#include "commands.hpp"
#include "context.hpp"
#include "contributions.hpp"
#include "prompt.hpp"
#include <dlfcn.h>
#include <readline/readline.h>
#include <termios.h>
#include <unistd.h>

static char *kCraneCoreLocation = "extern/core.so";
static char *kCraneStagingLocation = "extern/staging.so";

int Crane_load(CraneCommand *command, CraneContext *context) {
  std::string fileToLoad = command->arguments[0];

  if (fileToLoad == "Core") {
    fileToLoad = kCraneCoreLocation;
  } else if (fileToLoad == "Staging") {
    fileToLoad = kCraneStagingLocation;
  }

  if (access(fileToLoad.c_str(), R_OK) != 0) {
    printf("The module '%s' doesn't exist.\n", fileToLoad.c_str());
    return 1;
  }

  if (context->sharedHandleMap.find(fileToLoad) !=
      context->sharedHandleMap.end()) {
    printf("The module '%s' has already been loaded!\n", command->arguments[0]);

    // Despite this being an "error", the file was apparently loaded
    // successfully so there is no need to emit an error.
    return 0;
  }

  void *handle = dlopen(fileToLoad.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!handle) {
    printf("Failed to open the module.\n%s\n", dlerror());
    dlclose(handle);
    return 1;
  }

  context->sharedHandleMap.emplace(fileToLoad, handle);

  CraneContributionInitialiser init =
      (CraneContributionInitialiser)dlsym(handle, "Crane_contirbutedCommands");

  CraneContributedCommands *contrib = init();
  for (int i = 0; i < contrib->contributedCommands.size(); i++) {
    if (contrib->contributedCommands[i] == NULL)
      continue;

    context->commandMap[contrib->contributedCommands[i]->name] =
        contrib->contributedCommands[i];
  }

  return 0;
}

int Crane_QMark(CraneCommand *command, CraneContext *context) {
  printf("%d - %s\n", context->lastCommandResult,
         context->lastCommandResult == 0 ? "Successful" : "Failed");

  return 0;
}

static CraneContext *context;

char *commandCompletionEngine(const char *text, int state) {
  static int index;
  static std::vector<CraneCommandEntry *> commands;
  CraneCommandEntry *command;

  if (state == 0) {
    index = 0;
    for (auto &entry : context->commandMap) {
      commands.push_back(entry.second);
    }
  }

  while ((command = commands[index++])) {
    if (strncmp(command->name.c_str(), text, strlen(text)) == 0) {
      return strdup(command->name.c_str());
    }
  }

  return nullptr;
}

char *argumentCompletionEngine(const char *text, int state) {
  static char *opts[] = {"true", "false"};
  static int index = 0;
  char *opt;

  if (state == 0)
    index = 0;

  while ((opt == opts[index++])) {
    if (strncmp(text, text, strlen(text)) == 0) {
      return strdup(opt);
    }
  }

  return nullptr;
}

char **completionGenerator(const char *text, int start, int end) {
  rl_attempted_completion_over = 1;

  CraneCommand *command = CraneCommand::parseCommand(text);
  if (!command) {
    free(command);
    return nullptr;
  }

  if (command->arguments.size() == 0) {
    return rl_completion_matches(text, commandCompletionEngine);
  } else {
    CraneCommandEntry *possibleCommand = context->commandMap[command->name];
    if (!possibleCommand) {
      return NULL;
    }

    if (possibleCommand->argumentCount <= command->arguments.size()) {
      return NULL;
    }

    if (possibleCommand->arguments[command->arguments.size()]->type ==
        CraneCommandArgumentType::File) {
      rl_attempted_completion_over = 0; // Re-enable file completion
    } else if (possibleCommand->arguments[command->arguments.size()]->type ==
               CraneCommandArgumentType::Boolean) {
      return rl_completion_matches(text, argumentCompletionEngine);
    }

    return NULL;
  }
}

void cleanup() {
  // Cleanup Steps
  if (context->openedFile != NULL)
    fclose(context->openedFile->handle);
  for (auto &fileEntry : context->fileMap) {
    fclose(fileEntry.second->handle);
  }
  for (auto &sharedEntry : context->sharedHandleMap) {
    dlclose(sharedEntry.second);
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
  printf("    --no-core                      Disregards loading the Core "
         "module\n");
  printf("    --core </path/to/staging>      Sets the location of the Core "
         "module\n");
  printf("    --staging [/path/to/staging]   Sets the location of the Staging\n"
         "                                   module and/or loads the staging "
         "module\n");
}

int main(int argc, char **argv) {
  rl_attempted_completion_function = completionGenerator;
  atexit(cleanup);

  bool noCore = false;
  context = new CraneContext();

  CraneCommandEntry *loadCommand =
      new CraneCommandEntry("load", Crane_load, false);
  loadCommand->setCommandDescription("Loads a module from a dynamic library");
  loadCommand->addArgument("module", false, CraneCommandArgumentType::File);
  context->commandMap.emplace("load", loadCommand);

  context->commandMap.emplace("?",
                              new CraneCommandEntry("?", Crane_QMark, false));
  context->commandMap["?"]->setCommandDescription(
      "Prints the result of the last command");

  for (int i = 1; i < argc; i++) {
    if (argv[i] == "--no-core") {
      noCore = true;
    } else if (argv[i] == "--staging") {
      int result = loadCommand->handler(
          new CraneCommand("load", {kCraneStagingLocation}), context);
      if (result != 0) {
        printf("%sERR%s: Staging library not found in '%s'\n", kColorRed,
               kColorReset, kCraneStagingLocation);
        return 1;
      }
    }
  }

  return 0;
}
