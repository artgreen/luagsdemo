
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Local libraries
#include "collection.h"
#include "status.h"
#include "luafuncs.h"
#include "luags.h"

// global variable shared between C and Lua
Status *status;

int main(int argc, char *argv[]) {
    const char **files;

    // Initialize the LUA state
    printf("Initialize the LUA state\n");
    lg_open();

    printf("Opening Lua standard libraries\n");
    lg_openlibs();

    printf("Using Lua to configure ourselves via config.lua\n");
    lg_run_file("config.lua");
    int num_scripts = lg_get_string_array("scripts", files);

    printf("\nLoading Lua interface modules\n");
    // Load custom modules that we've written
    // Load the LUA Collection module
    lg_load_module(load_collection);
    // Load the LUA Status module
    lg_load_module(load_status);
    // Export single functions
    // we'll call these in the coltest Lua script below
    lg_load_module(export_funcs);

    // Allocate the variable "status" to the Lua engine
    status = malloc(sizeof(Status));
    status->ticks = 99;
    strcpy(status->name, "Initial status");

    printf("\nExecuting %d tests\n", num_scripts);
    // Load and execute the Lua scripts
    for( int i = 0; i < num_scripts; i++) {
        lg_run_file(files[i]);
    }
    lg_run_file("poker.lua");

    // Close the LUA state
    // this will also call cleanup functions
    printf("\nClosing down Lua interface\n");
    lg_close();
    free(status);

    printf("Shutting down... byeeeeeeeeeeeeeeee!\nPress RETURN to exit\n");
//    char dummy[10]; gets(dummy);
    return 0;
}
