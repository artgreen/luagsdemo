#ifndef COLLECTION_H
#define COLLECTION_H
#include <stddef.h>
#include "lua.h"
typedef struct { lua_Integer *data; size_t size; } Collection;
Collection *newCollection(size_t size);
void freeCollection(Collection *collection);
int luaopen_collection(lua_State *L);
#endif
