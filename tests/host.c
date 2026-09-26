#include <stdio.h>
#include "lua.h"
#include "lualib.h"
#include "luags.h"
#pragma memorymodel 1
#pragma stacksize LUA_IIGS_STACK_SIZE
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "Host check failed at line %d\n", __LINE__); lg_close(); return 1; } } while (0)
static int setup(lua_State *L) { luaL_openlibs(L); return 0; }
int main(void) {
    char anchor, files[LG_MAX_SCRIPTS][LG_PATH_SIZE];
    lua_Integer value;
    int count, i;
    lua_iigs_initstack(&anchor);
    CHECK(!lg_open());
    CHECK(lg_open()); /* A second open must not leak or replace the state. */
    CHECK(!lg_initialize(setup));
    CHECK(!lg_run_string("value=70000; function twice(n) return n*2 end; scripts={'one','two'}"));
    CHECK(!lg_get_integer("value", &value) && value == 70000L);
    CHECK(lg_get_integer("missing", &value));
    CHECK(!lg_call_integer("twice", value, &value) && value == 140000L);
    CHECK(lg_call_integer("missing", value, &value));
    CHECK(!lg_run_string("function bad(n) return 'wrong' end"));
    CHECK(lg_call_integer("bad", 0, &value));
    CHECK(!lg_run_string("function bad(n) error('callback error') end"));
    CHECK(lg_call_integer("bad", 0, &value));
    CHECK(lg_run_string("error({})"));
    CHECK(lg_run_string("this is not Lua"));
    CHECK(lg_run_file("tests/does-not-exist.lua"));
    CHECK(!lg_get_scripts(files, &count) && count == 2);
    CHECK(!lg_run_string("scripts={false}"));
    CHECK(lg_get_scripts(files, &count) && count == 0);
    /* Repeated return values and errors must not accumulate on the Lua stack. */
    for (i = 0; i < 200; ++i) {
        CHECK(!lg_run_string("return 1,2,3"));
        CHECK(!lg_call_integer("twice", (lua_Integer)i, &value) && value == (lua_Integer)i*2);
    }
    CHECK(!lg_run_string("value=42"));
    CHECK(!lg_get_integer("value", &value) && value == 42);
    CHECK(!lg_run_string("scripts=nil; setmetatable(_G,{__index=function() error('lookup error') end})"));
    CHECK(lg_get_integer("missing", &value));
    CHECK(lg_call_integer("missing", 1, &value));
    CHECK(lg_get_scripts(files, &count) && count == 0);
    CHECK(!lg_run_string("setmetatable(_G,nil)"));
    lg_close(); lg_close();
    CHECK(!lg_open()); CHECK(!lg_initialize(setup)); lg_close();
    puts("Host checks passed");
    return 0;
}
