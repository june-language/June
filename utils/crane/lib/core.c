#include "contributions.h"

contributableCommand(testCommand) {
  printf("This test command worked!\n");

  return 0;
}

CraneContributedCommands *Crane_contributedCommands() {
  CraneContributedCommands *contributions = initContributedCommands();

  contributeCommand(contributions, "test", CraneContributed_testCommand, 0);

  return contributions;
}
