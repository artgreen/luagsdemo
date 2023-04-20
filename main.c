
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
//    char *files[2] = {
//            "coltest.lua",
//            "stattest.lua",
//    };
    const char **files;

    // Initialize the LUA state
    printf("Initialize the LUA state\n");
    lg_open();

    printf("Opening libs\n");
    lg_openlibs();

    printf("Reading config from config.lua\n");
    lg_run_file("config.lua");
    lg_get_string_array("scripts", files);

    printf("\nLoading Lua interface modules\n");
    // Load the LUA Collection module
    lg_load_module(load_collection);
    // Load the LUA Status module
    lg_load_module(load_status);
    // Export single functions
    // we'll call these in a Lua script below
    lg_load_module(export_funcs);

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
