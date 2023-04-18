
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path "include"

#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "collection.h"
#include "status.h"

// global variable shared between C and Lua
Status *status;

int main(int argc, char *argv[]) {
    char *files[2] = {
            "col_test.lua",
            "stat_test.lua"
    };

    // Initialize the LUA state
    printf("Initialize the LUA state\n");
    lua_State *L = luaL_newstate();
    printf("Opening libs\n");
    luaL_openlibs(L);

    // Load the LUA Collection library
    printf("Loading Lua interface libraries\n");
    load_collection(L);
    // Load the LUA Status library
    load_status(L);

    // Export the variable "export" to the LUA engine
    printf("Exporting the status variable\n");
    status = newStatus(99, "Initial status");

    // Load and execute the Lua scripts
    printf("Executing tests\n");
    for( int i = 0; i < 2; i++) {
        printf("\nRunning %s...\n", files[i]);
        if (luaL_dofile(L, files[i]) != LUA_OK) {
            fprintf(stderr, "Error running script: %s\n", lua_tostring(L, -1));
            break;
        } else {
            lua_pop(L, lua_gettop(L));
        }
    }

    // Close the LUA state
    // this will also call cleanup functions
    lua_close(L);
    printf("Shutting down... byeeeeeeeeeeeeeeee!\n");
    return 0;
}
