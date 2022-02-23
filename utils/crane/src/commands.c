#include "commands.h"
#include <math.h>

#define HashPrime 87178291199UL

static int Crane_MapHash(const char *s, const int buckets) {
  long hash = 0;
  const int stringLength = strlen(s);
  for (int i = 0; i < stringLength; i++) {
    hash += (long)pow(HashPrime, stringLength - (i + 1)) * s[i];
    hash = hash % buckets;
  }

  return (int)hash;
}

CraneCommandEntry *createCommand(char *name, CraneCommandHandler handler,
                                 bool isVariadic) {
  CraneCommandEntry *command = calloc(1, sizeof(CraneCommandEntry));

  command->name = strdup(name);
  command->handler = handler;
  command->argumentCount = 0;
  command->arguments = NULL;
  command->isVariadic = isVariadic;

  return command;
}

CraneCommandArgument *createCommandArgument(char *name,
                                            CraneCommandArgumentType type) {
  CraneCommandArgument *argument = calloc(1, sizeof(CraneCommandArgument));

  argument->name = strdup(name);
  argument->type = type;

  return argument;
}

void addCommandArgument(CraneCommandEntry *entry,
                        CraneCommandArgument *argument) {
  if (entry->arguments == NULL) {
    entry->argumentCount = 1;
    entry->arguments = calloc(1, sizeof(CraneCommandArgument *));
  } else {
    entry->argumentCount++;
    entry->arguments =
        realloc(entry->arguments,
                sizeof(CraneCommandArgument *) * entry->argumentCount);
  }

  entry->arguments[entry->argumentCount - 1] = argument;
}

CraneCommandMap *createCommandMap() {
  CraneCommandMap *map = calloc(1, sizeof(CraneCommandMap));

  map->size = 16;
  map->count = 0;
  map->commands = calloc(map->size, sizeof(CraneCommandEntry *));

  return map;
}

CraneCommandEntry *findCommand(CraneCommandMap *map, char *name) {
  int commandHash = Crane_MapHash(name, map->size);

  int i = 0;
  while (true) {
    if ((commandHash + i) >= map->size)
      break;

    CraneCommandEntry *entry = map->commands[commandHash + i];

    if (entry != NULL) {
      if (strcmp(entry->name, name) == 0) {
        return entry;
      }
    }
    i++;
  }

  return NULL;
}

bool insertCommand(CraneCommandMap *map, CraneCommandEntry *command) {
  // Command already exists, do not insert
  if (findCommand(map, command->name) != NULL)
    return false;

  if (map->size == map->count) {
    map->size *= 2;
    map->commands =
        realloc(map->commands, sizeof(CraneCommandEntry *) * map->size);
  }

  int commandHash = Crane_MapHash(command->name, map->size);
  while (map->commands[commandHash] != NULL) {
    commandHash = (commandHash + 1) % map->size;
  }
  map->commands[commandHash] = command;

  return true;
}

void deleteCommandMap(CraneCommandMap *map) {
  for (int i = 0; i < map->size; i++) {
    CraneCommandEntry *item = map->commands[i];
    if (item != NULL) {
      free(item);
    }
  }

  free(map->commands);
  free(map);
}
