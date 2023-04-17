
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"
#pragma noroot

#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "status.h"

// Lua function to create a new status object
int l_newStatus(lua_State *L) {
    long ticks = luaL_checkinteger(L, 1);
    char *name = (char *)luaL_checkstring(L, 2);
    Status *status = newStatus(ticks, name);
    void *userdata = lua_newuserdata(L, sizeof(Status *));
    *(Status **)userdata = status;
    luaL_getmetatable(L, "status.status");
    lua_setmetatable(L, -2);
    return 1;
}

// Lua function to free a status object
int l_freeStatus(lua_State *L) {
    Status **status = (Status **)luaL_checkudata(L, 1, "status.status");
    freeStatus(*status);
    return 0;
}

// Lua function to get the ticks of a status object
int l_getStatusTicks(lua_State *L) {
    Status **status = (Status **)luaL_checkudata(L, 1, "status.status");
    long ticks = (*status)->ticks;
    lua_pushinteger(L, ticks);
    return 1;
}

// Lua function to set the ticks of a status object
int l_setStatusTicks(lua_State *L) {
    Status **status = (Status **)luaL_checkudata(L, 1, "status.status");
    long ticks = luaL_checkinteger(L, 2);
    (*status)->ticks = ticks;
    return 0;
}

// Lua function to get the name of a status object
int l_getStatusName(lua_State *L) {
    Status **status = (Status **)luaL_checkudata(L, 1, "status.status");
    char *name = (*status)->name;
    lua_pushstring(L, name);
    return 1;
}

// Lua function to set the name of a status object
int l_setStatusName(lua_State *L) {
    Status **status = (Status **)luaL_checkudata(L, 1, "status.status");
    char *name = (char *)luaL_checkstring(L, 2);
    (*status)->name = name;
    return 0;
}

// List of functions to be registered with the status module
static const luaL_Reg status_functions[] = {
        {"freeStatus", l_freeStatus},
        {"getTicks", l_getStatusTicks},
        {"setTicks", l_setStatusTicks},
        {"getName", l_getStatusName},
        {"setName", l_setStatusName},
        {NULL, NULL}
};

// Function to create the status module
int luaopen_status(lua_State *L) {
    // Create a metatable for the Status object
    luaL_newmetatable(L, "status.status");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_freeStatus);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    // Create a table to hold the status functions
    luaL_newlib(L, status_functions);

    // Return the table as the status module
    return 1;
}
