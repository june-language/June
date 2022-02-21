#include "contributions.h"

contributableCommand(open) {
  if (context->openedFile != NULL) {
    printf("There is already a file open! This will close the current file\n"
           "Are you sure? [y/n] ");

    int c;
    while ((c = getchar()) != '\n') {
      if (c == 'y' || c == 'Y') {
        fclose(context->openedFile);
        context->openedFile = NULL;
        context->openedFilePath = "";
        break;
      } else {
        printf("'%s' was not opened.", command->arguments[0]);
        return 0;
      }
    }
    // getchar() reads newline as well so we need to consume that
    // before it gets consumed in prompt.c
    getchar();
  }

  FILE *presaveOpen = fopen(command->arguments[0], "rb+");
  if (presaveOpen == NULL) {
    perror("Failed to open the file");
    return 1;
  }

  context->openedFile = presaveOpen;
  context->openedFilePath = command->arguments[0];

  return 0;
}

CraneContributedCommands *Crane_contributedCommands() {
  CraneContributedCommands *contributions = initContributedCommands();

  contributeCommand(contributions, "open", CraneContributed_open, 1, false);

  return contributions;
}
