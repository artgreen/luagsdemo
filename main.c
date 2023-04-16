
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path "include"

#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "collection.h"

int luaopen_collection(lua_State *L);

int main(int argc, char *argv[]) {
    // Initialize the LUA state
    printf("Initialize the LUA state\n");
    lua_State *L = luaL_newstate();
    printf("Opening libs\n");
    luaL_openlibs(L);

    // Load the LUA Collection library
    printf("Loading Collection library\n");
    luaL_requiref(L, "collection", luaopen_collection, 1);
    lua_pop(L, 1);

//    // Load and execute the LUA script
//    printf("Executing bridge.lua\n");
//    if (luaL_loadfile(L, "bridge.lua") || lua_pcall(L, 0, 0, 0)) {
//        fprintf(stderr, "Error running script: %s\n", lua_tostring(L, -1));
//        lua_close(L);
//        return 1;
//    }

    // Close the LUA state
    printf("Closing LUA state... Byeeeeeeeeeee!\n");
    lua_close(L);
    return 0;
}
