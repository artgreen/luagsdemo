#pragma memorymodel 1
#pragma noroot
#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "luags.h"

static lua_State *state;

/* Each operation restores the stack, including error paths. */
static int finish(int status, int base) {
    if (status != LUA_OK) {
        const char *message = lua_tostring(state, -1);
        fprintf(stderr, "Lua error: %s\n", message ? message : "non-string error");
    }
    lua_settop(state, base);
    return status != LUA_OK;
}
int lg_open(void) {
    if (state != NULL) return 1;
    state = luaL_newstate();
    return state == NULL;
}
void lg_close(void) {
    if (state != NULL) { lua_close(state); state = NULL; }
}
int lg_initialize(lua_CFunction setup) {
    int base = lua_gettop(state);
    lua_pushcfunction(state, setup);
    return finish(lua_pcall(state, 0, 0, 0), base);
}
int lg_run_file(const char *name) {
    int base = lua_gettop(state);
    int status = luaL_loadfile(state, name);
    if (status == LUA_OK) status = lua_pcall(state, 0, 0, 0);
    return finish(status, base);
}
int lg_run_string(const char *code) {
    int base = lua_gettop(state);
    int status = luaL_loadstring(state, code);
    if (status == LUA_OK) status = lua_pcall(state, 0, 0, 0);
    return finish(status, base);
}

/* Accessors run inside Lua's error boundary too: a global lookup can invoke
 * __index, and even an ordinary push may allocate. Requests live on the C
 * caller's stack until the protected call returns. */
static int request(lua_CFunction fn, void *data) {
    int base = lua_gettop(state);
    lua_pushcfunction(state, fn);
    lua_pushlightuserdata(state, data);
    return finish(lua_pcall(state, 1, 0, 0), base);
}
typedef struct { char (*files)[LG_PATH_SIZE]; int count; } ScriptRequest;
static int read_scripts(lua_State *L) {
    ScriptRequest *r = lua_touserdata(L, 1);
    size_t n, i, length;
    const char *value;
    lua_getglobal(L, "scripts");
    if (!lua_istable(L, -1)) goto invalid;
    n = lua_rawlen(L, -1);
    if (n > LG_MAX_SCRIPTS) goto invalid;
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        lua_Integer key;
        if (!lua_isinteger(L, -2)) goto invalid;
        key = lua_tointeger(L, -2);
        if (key < 1 || key > (lua_Integer)n) goto invalid;
        lua_pop(L, 1);
    }
    for (i = 1; i <= n; ++i) {
        lua_rawgeti(L, -1, (lua_Integer)i);
        if (lua_type(L, -1) != LUA_TSTRING) goto invalid;
        value = lua_tolstring(L, -1, &length);
        if (length == 0 || length >= LG_PATH_SIZE || memchr(value, 0, length)) goto invalid;
        memcpy(r->files[i-1], value, length);
        r->files[i-1][length] = 0;
        lua_pop(L, 1);
    }
    r->count = (int)n;
    return 0;
invalid:
    return luaL_error(L, "Invalid scripts: expected an array of at most 8 nonempty paths (63 bytes each)");
}
int lg_get_scripts(char files[][LG_PATH_SIZE], int *count) {
    ScriptRequest r;
    int status;
    r.files = files; r.count = 0;
    status = request(read_scripts, &r);
    *count = r.count;
    return status;
}
typedef struct { const char *name; lua_Integer value; } IntegerRequest;
static int read_integer(lua_State *L) {
    IntegerRequest *r = lua_touserdata(L, 1);
    lua_getglobal(L, r->name);
    if (!lua_isinteger(L, -1)) return luaL_error(L, "Expected integer global: %s", r->name);
    r->value = lua_tointeger(L, -1);
    return 0;
}
int lg_get_integer(const char *name, lua_Integer *value) {
    IntegerRequest r;
    int status;
    r.name = name; r.value = 0;
    status = request(read_integer, &r);
    if (!status) *value = r.value;
    return status;
}
static int call_integer(lua_State *L) {
    IntegerRequest *r = lua_touserdata(L, 1);
    lua_getglobal(L, r->name);
    lua_pushinteger(L, r->value);
    lua_call(L, 1, 1); /* request() protects lookup, call, and result validation. */
    if (!lua_isinteger(L, -1)) return luaL_error(L, "Expected integer result: %s", r->name);
    r->value = lua_tointeger(L, -1);
    return 0;
}
int lg_call_integer(const char *name, lua_Integer arg, lua_Integer *result) {
    IntegerRequest r;
    int status;
    r.name = name; r.value = arg;
    status = request(call_integer, &r);
    if (!status) *result = r.value;
    return status;
}
