#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "collection.h"
#include "status.h"
#include "luafuncs.h"
#include "luags.h"
#include "inventory.h"
#pragma memorymodel 1
#pragma stacksize LUA_IIGS_STACK_SIZE

Status app_status = {99, "Initial status"};
static int setup(lua_State *L) {
    luaL_openlibs(L);
    luaL_requiref(L, "collection", luaopen_collection, 1); lua_pop(L, 1);
    luaL_requiref(L, "status", luaopen_status, 1); lua_pop(L, 1);
    luaL_requiref(L, "inventory", luaopen_inventory, 1); lua_pop(L, 1);
    export_funcs(L);
    return 0;
}
int main(int argc, char *argv[]) {
    char anchor; /* This frame stays alive through lua_close. */
    char files[LG_MAX_SCRIPTS][LG_PATH_SIZE];
    lua_Integer count_value, answer;
    int count, i, result = 1;
    if (argc > 3 || (argc == 3 && strcmp(argv[1], "--script"))) {
        fprintf(stderr, "Usage: luademo [config.lua] | --script file.lua\n");
        return 1;
    }
    lua_iigs_initstack(&anchor);
    if (lg_open()) { fprintf(stderr, "Cannot create Lua state\n"); return 1; }
    if (lg_initialize(setup)) goto done;
    if (argc == 3) { result = lg_run_file(argv[2]); goto done; }
    if (lg_run_file(argc == 2 ? argv[1] : "config.lua")) goto done;
    if (lg_get_scripts(files, &count)) goto done;
    printf("Lua IIgs embedding demo: %d scripts\n", count);
    for (i = 0; i < count; ++i) {
        printf("Running %s\n", files[i]);
        if (lg_run_file(files[i])) goto done;
    }
    /* The remaining original TODOs: C strings, globals, and Lua callbacks. */
    if (lg_run_string("host_value = 21; function double_value(n) return n * 2 end")) goto done;
    if (lg_get_integer("host_value", &count_value)) goto done;
    if (lg_call_integer("double_value", count_value, &answer)) goto done;
    printf("C -> Lua -> C: %ld -> %ld\n", (long)count_value, (long)answer);
    printf("C sees status: %ld, %s\n", (long)app_status.ticks, app_status.name);
    result = 0;
done:
    lg_close();
    if (!result) printf("Demo completed\n");
    return result;
}
