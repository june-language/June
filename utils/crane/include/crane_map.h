#ifndef map_h
#define map_h

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _CraneMapEntry {
  char *name;
  void *value;
} CraneMapEntry;

/**
 * Contrary to the CraneCommandMap structure,
 * this is a general purpose map for arbitrary
 * `void*` castable
 */

typedef struct _CraneMap {
  int size;
  int count;
  void **entries;
} CraneMap;

CraneMap *createMap();
CraneMapEntry *findEntry(CraneMap *map, char *name);
void insertEntry(CraneMap *map, char *name, void *entry);
CraneMapEntry **aggregateMapItems(CraneMap *map);
void deleteMap(CraneMap *map);

#endif
