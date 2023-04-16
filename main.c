
#pragma optimize    9
#pragma lint       -1
#pragma debug       0

#pragma path "include"

#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int main(int argc, char *argv[]) {
    // Initialize the LUA state
    printf("Initialize the LUA state\n");
    lua_State *L = luaL_newstate();
    printf("Opening libs\n");
    luaL_openlibs(L);

    // Close the LUA state
    printf("Closing LUA state... Byeeeeeeeeeee!\n");
    lua_close(L);
    return 0;
}
