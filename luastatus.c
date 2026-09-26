#pragma memorymodel 1
#pragma noroot
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "status.h"
static int get_ticks(lua_State *L) { lua_pushinteger(L, app_status.ticks); return 1; }
static int set_ticks(lua_State *L) { app_status.ticks = luaL_checkinteger(L, 1); return 0; }
static int get_name(lua_State *L) { lua_pushstring(L, app_status.name); return 1; }
static int set_name(lua_State *L) {
    size_t n;
    const char *name = luaL_checklstring(L, 1, &n);
    luaL_argcheck(L, n < sizeof(app_status.name) && !memchr(name, 0, n), 1,
                  "name must be at most 19 bytes without embedded NULs");
    memcpy(app_status.name, name, n);
    app_status.name[n] = 0;
    return 0;
}
static const luaL_Reg functions[] = {
    {"getTicks", get_ticks}, {"setTicks", set_ticks},
    {"getName", get_name}, {"setName", set_name}, {NULL, NULL}
};
int luaopen_status(lua_State *L) { luaL_newlib(L, functions); return 1; }
