#ifndef contributions_hpp
#define contributions_hpp

#include "commands.hpp"
#include "context.hpp"

/* typedef int (*CraneCommandHandler)(CraneCommand*, CraneContext *context); */
#define contributableCommand(commandName)                                      \
  extern "C" int CraneContributed_##commandName(CraneCommand *command,         \
                                                CraneContext *context)

#define contributeCommand(contributions, name, isVariadic)                     \
  _contributeCommand(contributions, #name, CraneContributed_##name, isVariadic)

// TODO: do more with this...
struct CraneContributedCommands {
  std::vector<CraneCommandEntry*> contributedCommands;
};

typedef CraneContributedCommands *(*CraneContributionInitialiser)();

#endif

