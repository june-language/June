#include "crane_map.h"
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

CraneMap *createMap() {
  CraneMap *map = calloc(1, sizeof(CraneMap));

  map->size = 5;
  map->count = 0;
  map->entries = calloc(map->size, sizeof(CraneMap *));

  return map;
}

CraneMapEntry *findEntry(CraneMap *map, char *name) {
  int entryHash = Crane_MapHash(name, map->size);

  int i = 0;
  while (true) {
    if ((entryHash + i) >= map->size)
      break;

    CraneMapEntry *entry = map->entries[entryHash + i];

    if (entry != NULL) {
      if (strcmp(entry->name, name) == 0) {
        return entry;
      }
    }
    i++;
  }

  return NULL;
}

void insertEntry(CraneMap *map, char *name, void *entry) {
  if (findEntry(map, name) != NULL)
    return;

  if (map->size == map->count) {
    map->size *= 2;
    map->entries = realloc(map->entries, sizeof(CraneMapEntry *) * map->size);
  }

  int entryHash = Crane_MapHash(name, map->size);
  while (map->entries[entryHash] != NULL) {
    entryHash = (entryHash + 1) % map->size;
  }

  CraneMapEntry *mapEntry = calloc(1, sizeof(CraneMapEntry));
  mapEntry->name = strdup(name);
  mapEntry->value = entry;

  map->entries[entryHash] = mapEntry;
}

void deleteEntry(CraneMap *map, char *name) {
  int entryHash = Crane_MapHash(name, map->size);

  int i = 0;
  while (true) {
    if ((entryHash + i) > map->size)
      break;

    CraneMapEntry *entry = map->entries[entryHash + i];

    if (entry != NULL) {
      if (strcmp(entry->name, name) == 0) {
        map->entries[entryHash + i] = NULL;
        free(entry->name);
        free(entry->value);
      }
    }
    i++;
  }
}

CraneMapEntry **aggregateMapItems(CraneMap *map) {
  CraneMapEntry **entries = calloc(map->count, sizeof(CraneMapEntry *));

  for (int i = 0; i < map->size; i++) {
    if (map->entries[i] != NULL) {
      entries[i % map->count] = map->entries[i];
    }
  }

  return entries;
}

void deleteMap(CraneMap *map) {
  for (int i = 0; i < map->size; i++) {
    CraneMapEntry *item = map->entries[i];
    if (item != NULL) {
      free(item->name);
      free(item->value);
    }
  }

  free(map->entries);
  free(map);
}
