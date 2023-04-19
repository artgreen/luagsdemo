
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"
#pragma noroot

segment "luafuncs";

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "luafuncs.h"

/*
 * This code is borrowed from: https://lucasklassmann.com/blog/2019-02-02-embedding-lua-in-c/
 * I can't write a more clear example of how to expose a function to C.
 */
// Define our function, we have to follow the protocol of lua_CFunction that is
// typedef int (*lua_CFunction) (lua_State *L);
// When this function is called by Lua, the stack contains the arguments needed,
// what we need to do check if the arguments have the type that we expect.
int multiplication(lua_State *L) {

    // Check if the first argument is integer and return the value
    int a = luaL_checkinteger(L, 1);

    // Check if the second argument is integer and return the value
    int b = luaL_checkinteger(L, 2);

    // multiply and store the result inside a type lua_Integer
    lua_Integer c = a * b;

    // Here we prepare the values to be returned.
    // First we push the values we want to return onto the stack in direct order.
    // Second, we must return the number of values pushed onto the stack.

    // Pushing the result onto the stack to be returned
    lua_pushinteger(L, c);

    return 1; // The number of returned values
}

void export_funcs(lua_State *L) {
    // Push the pointer to function
    lua_pushcfunction(L, multiplication);
    // Get the value on top of the stack
    // and set as a global, in this case is the function
    // this creates "mul" which can be used by scripts
    lua_setglobal(L, "mul");
}
