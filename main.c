
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path "include"

// Standard libraries
#include <stdio.h>
// Lua libraries
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
// Local libraries
#include "collection.h"
#include "status.h"
#include "luafuncs.h"
#include "luags.h"

// global variable shared between C and Lua
Status *status;

int main(int argc, char *argv[]) {
    char *files[2] = {
            "coltest.lua",
            "stattest.lua"
    };

    // Initialize the LUA state
    printf("Initialize the LUA state\n");
    lg_open(0);
    lua_State *L = lg_state();
    printf("Opening libs\n");
    luaL_openlibs(L);

    // Load the LUA Collection library
    printf("Loading Lua interface libraries\n");
    load_collection(lg_state());
    // Load the LUA Status library
    load_status(lg_state());
    // Export single functions
    export_funcs(lg_state());        // we'll call these in a Lua script below

    // Allocate the variable "status" to the Lua engine
    printf("Allocating the status variable\n");
    status = newStatus(99, "Initial status");

    printf("\nExecuting tests\n");

    // Load and execute the Lua scripts
    for( int i = 0; i < 2; i++) {
        lg_run_file(files[i]);
    }

    // Close the LUA state
    // this will also call cleanup functions
    lg_close();
    freeStatus(status);

    printf("Shutting down... byeeeeeeeeeeeeeeee!\nPress RETURN to exit");
//    char dummy[10]; gets(dummy);
    return 0;
}
