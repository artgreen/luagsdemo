
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"
#pragma noroot


#include <stdio.h>
#include <string.h>
#include "luags.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "luags.h"

/*
 * Lua state
 * This interface will completely hide Lua internals.
 */
lua_State *_L;

/*
 * Connect to the Lua library
 * no_open: Boolean. If != 0, do not open the Lua library set. This is used to
 * control what libraries users can be called in scripts.  This can prevent
 * security concerns.
 * Returns: 0 on success, 1 on failure
 */
int lg_open(void) {
    // Initialize the LUA state
    _L = luaL_newstate();
    if( _L != (lua_State *) NULL ) {
        return 1;
    } else {
        return 0;
    }
}
/*
 * Close the Lua library
 * No parameters
 * Returns: nothing.
 */
void lg_close(void) {
    // Close the LUA state
    // this will also call cleanup functions
    lua_close(_L);
}
lua_State *lg_state(void) {
    return _L;
}
/*
 * Execute a Lua script file
 * file_name: Name of a file containing Lua script or Lua bytecode
 * Returns: NULL on success, on failure returns the Lua error message
 */
const char * lg_run_file(const char *file_name) {
    printf("\nRunning %s...\n", file_name);
    if (luaL_dofile(_L, file_name) != LUA_OK) {
        return(lua_tostring(_L, -1));
    } else {
        lua_pop(_L, lua_gettop(_L));
    }
    return (char *) NULL;
}
/*
 * Open standard Lua libraries
 * This opens the standard Lua libraries for functions like print().
 * You may not want to expose all libraries to running scripts if the scripts
 * can be modified by users.  For example, they could use the files functions
 * to open files in a script running in your program.
 */
void lg_openlibs(void) {
    luaL_openlibs(_L);
}
/*
 * Load a custom module
 * module_func: a pointer to a function that sets up the custom module.
 * Returns: nothing.
 * The function's signature must be:
 *      void function_name(lua_State *L)
 */
void lg_load_module(void (*module_func)(lua_State *)) {
    printf("Loading module at address %p\n", module_func);
    module_func(_L);
}
/*
 *  Return a string array
 *  name: name of the global variable in Lua
 *  array: the base pointer to the string array.
 *  Returns: number of elements in the array
 *  The function allocates
 */
int lg_get_string_array(char *name, const char **array) {
    lua_getglobal(_L, name);               // Lookup the global variable name

    lua_len(_L, -1);                   // Get the length of the array
    int n = lua_tointeger(_L, -1);           // Create our max count variable
    lua_pop(_L, 1);                          // Pop the length from the stack

    for (int i = 1; i <= n; i++) {
        lua_pushinteger(_L, i);          // Push index we want onto stack
        lua_gettable(_L, -2);           // Get the value at index i
        array[i-1] = lua_tostring(_L, -1);    // Save the value in the array
        lua_pop(_L, 1);                       // Pop the value from the stack
    }
    return n;
}
