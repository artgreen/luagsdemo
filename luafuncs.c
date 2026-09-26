#pragma memorymodel 1
#pragma noroot
#include "lua.h"
#include "lauxlib.h"
#include "luafuncs.h"
/* Preserve Lua integer width and defined wrapping arithmetic (also on ORCA/C). */
static int multiplication(lua_State *L) {
    lua_Unsigned a = (lua_Unsigned)luaL_checkinteger(L, 1);
    lua_Unsigned b = (lua_Unsigned)luaL_checkinteger(L, 2);
    lua_pushinteger(L, (lua_Integer)(a * b));
    return 1;
}
void export_funcs(lua_State *L) {
    lua_pushcfunction(L, multiplication);
    lua_setglobal(L, "mul");
}
