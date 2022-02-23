#ifndef contributions_h
#define contributions_h

#include "commands.h"
#include "context.h"

#define kCraneDefaultContributionSize 5

/* typedef int (*CraneCommandHandler)(CraneCommand*, CraneContext *context); */
#define contributableCommand(commandName)                                      \
  int CraneContributed_##commandName(CraneCommand *command,                    \
                                     CraneContext *context)

#define contributeCommand(contributions, name, isVariadic)                     \
  _contributeCommand(contributions, #name, CraneContributed_##name, isVariadic)

typedef struct _CraneContributedCommands {
  int contributedCount;
  int contributedSizeCap;
  CraneCommandEntry **contributedCommands;
} CraneContributedCommands;

typedef CraneContributedCommands *(*CraneContributionInitialiser)();

inline static CraneContributedCommands *initContributedCommands() {
  CraneContributedCommands *contributedCommands =
      calloc(1, sizeof(CraneContributedCommands));

  contributedCommands->contributedCount = 0;
  contributedCommands->contributedSizeCap = kCraneDefaultContributionSize;
  contributedCommands->contributedCommands =
      calloc(kCraneDefaultContributionSize, sizeof(CraneCommandEntry *));

  return contributedCommands;
}

inline static void _contributeCommand(CraneContributedCommands *contributions,
                                      char *name, CraneCommandHandler handler,
                                      bool isVariadic) {
  CraneCommandEntry *entry = calloc(1, sizeof(CraneCommandEntry));

  entry->name = strdup(name);
  entry->handler = handler;
  entry->argumentCount = 0;
  entry->arguments = NULL;
  entry->isVariadic = isVariadic;

  if (contributions->contributedCount > contributions->contributedSizeCap) {
    contributions->contributedSizeCap *= 2;
    contributions->contributedCommands = realloc(
        contributions->contributedCommands,
        (contributions->contributedSizeCap) * sizeof(CraneCommandEntry *));
  }

  contributions->contributedCommands[contributions->contributedCount++] = entry;
}

inline static void
addContributedArgument(CraneContributedCommands *contributions, char *name,
                       CraneCommandArgumentType type) {
  CraneCommandEntry *entry =
      contributions->contributedCommands[contributions->contributedCount - 1];

  if (entry->arguments == NULL) {
    entry->argumentCount = 1;
    entry->arguments = calloc(1, sizeof(CraneCommandArgument *));
  } else {
    entry->argumentCount++;
    entry->arguments =
        realloc(entry->arguments,
                sizeof(CraneCommandArgument *) * entry->argumentCount);
  }

  entry->arguments[entry->argumentCount - 1] =
      calloc(1, sizeof(CraneCommandArgument));
  ;
  entry->arguments[entry->argumentCount - 1]->name = name;
  entry->arguments[entry->argumentCount - 1]->type = type;
}

#endif
