
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
int lg_open(int no_open) {
    // Initialize the LUA state
    _L = luaL_newstate();
    if( _L != (lua_State *) NULL ) {
        // do not open Lua libs if no_open is set
        if( no_open ) luaL_openlibs(_L);
        return 1;
    } else {
        return 0;
    }

}
/*
 * Close the Lua library
 * No parameters
 * Returns: Nothing.
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
 * Result: NULL on success, on failure returns the Lua error message
 */
const char * lg_run_file(char *file_name) {
    printf("\nRunning %s...\n", file_name);
    if (luaL_dofile(_L, file_name) != LUA_OK) {
        return(lua_tostring(_L, -1));
    } else {
        lua_pop(_L, lua_gettop(_L));
    }
    return (char *) NULL;
}
/*
 *
 */

/*
 *
 */
void lg_load_module(void (*module_func)(lua_State *)) {
    printf("Loading module at address %p\n", module_func);
    module_func(_L);
}
/*
 *
 */
void get_array(lua_State *L, char *name) {
    char temp[15];

    lua_getglobal(L, "scripts");
    lua_rawgeti(L, -1, 1);      // foo is on the top, access with -1 and access to foo[1] with 1
    strcpy(temp, lua_tostring(L, -1)); // foo[1] is now on the top of the stack
}
