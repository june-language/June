#include "context.h"

CraneContext *newContext() {
  CraneContext *ctx = calloc(1, sizeof(CraneContext));

  ctx->openedFile = NULL;
  ctx->failedLastCommand = false;
  ctx->dynHandleMap = createMap();
  ctx->commandMap = createCommandMap();

  return ctx;
}
