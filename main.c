
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
#include "status.h"

int luaopen_collection(lua_State *L);
int luaopen_status(lua_State *L);
int l_freeStatus(lua_State *L);

Status *status;

// Create a metatable for the Status object
void export_status(lua_State *L) {
    char *name = "Initial status";
    status = newStatus(0, name);

    luaL_newmetatable(L, "status.status");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_freeStatus);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    // Create a userdata object that points to the existing status object
    void *userdata = lua_newuserdata(L, sizeof(Status *));
    *(Status **)userdata = status;
    luaL_getmetatable(L, "status.status");
    lua_setmetatable(L, -2);
}
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

    // Export the variable "export" to the LUA engine
    printf("Exporting the status variable\n");
    export_status(L);

    // Register the status module with the LUA state
    printf("Loading Status library\n");
    luaL_requiref(L, "status", luaopen_status, 1);
    lua_pop(L, 1);

    // Load and execute the LUA script
    printf("Executing col_test.lua\n");
    if (luaL_loadfile(L, "col_test.lua") || lua_pcall(L, 0, 0, 0)) {
        fprintf(stderr, "Error running script: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    // Close the LUA state
    printf("Closing LUA state... Byeeeeeeeeeee!\n");
    lua_close(L);
    return 0;
}
