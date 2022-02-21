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

inline static void contributeCommand(CraneContributedCommands *contributions,
                                     char *name, CraneCommandHandler handler,
                                     int argumentCount, bool isVariadic) {
  CraneCommandEntry *entry = calloc(1, sizeof(CraneCommandEntry));

  entry->name = strdup(name);
  entry->handler = handler;
  entry->argumentCount = argumentCount;
  entry->isVariadic = isVariadic;

  if (contributions->contributedCount > contributions->contributedSizeCap) {
    printf("Allocating after %d spaces\n", contributions->contributedCount);
    contributions->contributedSizeCap *= 2;
    contributions->contributedCommands = realloc(
        contributions->contributedCommands,
        (contributions->contributedSizeCap) * sizeof(CraneCommandEntry *));
  }

  contributions->contributedCommands[contributions->contributedCount++] = entry;
}

#endif
