/**
 *
 * staging.c - The rudimentary staging platform for Crane
 *
 */

#include "contributions.h"

int _leven(const char *f, size_t fLength, const char *s, size_t sLength) {
  int x, y, z;

  if (!fLength)
    return sLength;
  if (!sLength)
    return fLength;

  if (f[fLength - 1] == s[sLength - 1])
    return _leven(f, fLength - 1, s, sLength - 1);

  x = _leven(f, fLength - 1, s, sLength - 1);
  y = _leven(f, fLength, s, sLength - 1);
  z = _leven(f, fLength - 1, s, sLength);

  if (x > y)
    x = y;
  if (x > z)
    x = z;

  return x + 1;
}

int leven(const char *first, const char *second) {
  if (strcmp(first, second) == 0)
    return 0;

  size_t firstLength = strlen(first);
  size_t secondLength = strlen(second);

  return _leven(first, firstLength, second, secondLength);
}

/**
 *
 * Idea: Levenshtein-based "did you mean" algorithm for unknown commands.
 *
 */
contributableCommand(leven) {
  int distance = leven(command->arguments[0], command->arguments[1]);

  printf("Levenshtein Distance: %d\n", distance);

  return 0;
}

#define contributeStagedCommand(name, isVariadic)                              \
  do {                                                                         \
    contributeCommand(contributions, name, isVariadic);                        \
    contributions->contributedCommands[contributions->contributedCount - 1]    \
        ->shouldOverride = true;                                               \
  } while (false)

CraneContributedCommands *Crane_contributedCommands() {
  CraneContributedCommands *contributions = initContributedCommands();

  contributeStagedCommand(leven, false);
  setContributionDescription(contributions, "Finds the Levenshtein index between 2 strings");
  addContributedArgument(contributions, "firstString", CraneArgumentTypeString, false);
  addContributedArgument(contributions, "secondString", CraneArgumentTypeString, false);

  return contributions;
}
