#include "context.h"

CraneContext *newContext() {
  CraneContext *ctx = calloc(1, sizeof(CraneContext));

  ctx->openedFile = NULL;
  ctx->openedFilePath = NULL;
  ctx->lastCommandResult = 0;
  ctx->dynHandleMap = createMap();
  ctx->commandMap = createCommandMap();

  return ctx;
}
