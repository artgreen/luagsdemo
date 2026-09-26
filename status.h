#ifndef STATUS_H
#define STATUS_H
#include "lua.h"
typedef struct { lua_Integer ticks; char name[20]; } Status;
extern Status app_status; /* Owned by the host, never freed by Lua. */
int luaopen_status(lua_State *L);
#endif
