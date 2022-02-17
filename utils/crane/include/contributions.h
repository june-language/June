#ifndef contributions_h
#define contributions_h

#include "commands.h"
#include "context.h"

#define kCraneDefaultContributionSize 5

/* typedef int (*CraneCommandHandler)(CraneCommand*, CraneContext *context); */
#define contributableCommand(commandName)                                      \
  int CraneContributed_##commandName(CraneCommand *command,                    \
                                     CraneContext *context)

typedef struct _CraneContributedCommands {
  int contributedCount;
  CraneCommandEntry **contributedCommands;
} CraneContributedCommands;

typedef CraneContributedCommands *(*CraneContributionInitialiser)();

inline static CraneContributedCommands *initContributedCommands() {
  CraneContributedCommands *contributedCommands =
      calloc(1, sizeof(CraneContributedCommands));

  contributedCommands->contributedCount = kCraneDefaultContributionSize;
  contributedCommands->contributedCommands =
      calloc(kCraneDefaultContributionSize, sizeof(CraneCommandEntry *));

  return contributedCommands;
}

inline static void contributeCommand(CraneContributedCommands *contributions,
                                     char *name, CraneCommandHandler handler,
                                     int argumentCount) {
  CraneCommandEntry *entry = calloc(1, sizeof(CraneCommandEntry));

  entry->name = strdup(name);
  entry->handler = handler;
  entry->argumentCount = argumentCount;

  if (contributions->contributedCount > kCraneDefaultContributionSize) {
    contributions->contributedCommands = realloc(
        contributions->contributedCommands,
        (++contributions->contributedCount) * sizeof(CraneCommandEntry *));
  }

  contributions->contributedCommands[contributions->contributedCount++] = entry;
}

#endif
