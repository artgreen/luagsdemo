
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"
#pragma noroot

segment "luastatus";

#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "status.h"

extern Status *status;

// Lua function to create a new status object
int l_newStatus(lua_State *L) {
    // we don't want lua to create the status variable
    // but if we did, we'd do it here.
    return 1;
}

// Lua function to free a status object (Garbage Collection)
int l_freeStatus(lua_State *L) {
    // We don't want Lua to free the status variable
    // but if we did, we'd do it here.
    return 0;
}

// Lua function to get the ticks of a status object
int l_getStatusTicks(lua_State *L) {
    lua_pushinteger(L, status->ticks);
    return 1;
}

// Lua function to set the ticks of a status object
int l_setStatusTicks(lua_State *L) {
    status->ticks = luaL_checkinteger(L, 1);
    return 0;
}

// Lua function to get the name of a status object
int l_getStatusName(lua_State *L) {
    lua_pushstring(L, status->name);
    return 1;
}

// Lua function to set the name of a status object
int l_setStatusName(lua_State *L) {
    strcpy(status->name, (char *)luaL_checkstring(L, 1));
    return 0;
}
// List of functions to be registered with the status module
static const luaL_Reg status_functions[] = {
        {"freeStatus", l_freeStatus},
        {"getTicks", l_getStatusTicks},
        {"setTicks", l_setStatusTicks},
        {"getName", l_getStatusName},
        {"setName", l_setStatusName},
        {"newStatus", l_newStatus},
        {NULL, NULL}
};

// Function to create the status module
int luaopen_status(lua_State *L) {
    // Create a table to hold the status functions
    luaL_newlib(L, status_functions);
    return 1;
}

// Function to load the module itself
void load_status(lua_State *L) {
    // Register the status module with the LUA state
    luaL_requiref(L, "status", luaopen_status, 1);
    // Remove the module from the stack, since luaL_requiref pushes the module on the stack
    lua_pop(L, 1);
}
