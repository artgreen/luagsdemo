#pragma memorymodel 1
#pragma noroot
#include "lua.h"
#include "lauxlib.h"
#include "collection.h"
#define COLLECTION "luagsdemo.collection"
#define MAX_ITEMS 4096
static Collection *check(lua_State *L) {
    Collection **p = luaL_checkudata(L, 1, COLLECTION);
    luaL_argcheck(L, *p != NULL, 1, "collection is closed");
    return *p;
}
static size_t index_at(lua_State *L, Collection *c) {
    lua_Integer index = luaL_checkinteger(L, 2);
    luaL_argcheck(L, index >= 1 && index <= (lua_Integer)c->size, 2, "index out of range");
    return (size_t)(index - 1);
}
static int create(lua_State *L) {
    lua_Integer n = luaL_checkinteger(L, 1);
    Collection **p;
    luaL_argcheck(L, n > 0 && n <= MAX_ITEMS, 1, "size must be 1..4096");
    /* Install the finalizer before allocating the resource it owns. */
    p = lua_newuserdatauv(L, sizeof(*p), 0);
    *p = NULL;
    luaL_setmetatable(L, COLLECTION);
    *p = newCollection((size_t)n);
    if (*p == NULL) return luaL_error(L, "cannot allocate collection");
    return 1;
}
static int close_collection(lua_State *L) {
    Collection **p = luaL_checkudata(L, 1, COLLECTION);
    freeCollection(*p);
    *p = NULL;
    return 0;
}
static int size(lua_State *L) { lua_pushinteger(L, (lua_Integer)check(L)->size); return 1; }
static int get(lua_State *L) {
    Collection *c = check(L);
    size_t i = index_at(L, c);
    lua_pushinteger(L, c->data[i]);
    return 1;
}
static int set(lua_State *L) {
    Collection *c = check(L);
    size_t i = index_at(L, c);
    c->data[i] = luaL_checkinteger(L, 3);
    return 0;
}
static const luaL_Reg methods[] = {
    {"size", size}, {"get", get}, {"set", set}, {"close", close_collection}, {NULL, NULL}
};
static const luaL_Reg functions[] = {{"new", create}, {NULL, NULL}};
int luaopen_collection(lua_State *L) {
    luaL_newmetatable(L, COLLECTION);
    luaL_setfuncs(L, methods, 0);
    lua_pushvalue(L, -1); lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, close_collection); lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, close_collection); lua_setfield(L, -2, "__close");
    lua_pushliteral(L, "collection"); lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newlib(L, functions);
    return 1;
}
