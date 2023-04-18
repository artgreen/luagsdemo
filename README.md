# luagsdemo
Program to demo embedding the IIgs flavor of LUA

Built with ORCA/C.

    $ iix luademo foo.lua
    Initialize the LUA state
    Opening libs
    Loading Lua interface libraries
    Loading Collection library
    Exporting the status variable
    newStatus: Created Status object at address 2D452
    Executing tests

    Running col_test.lua...
    Collection size:        10
    Values in nums:
    0               42
    1               23
    2               12
    3               99
    4               0
    5               0
    6               0
    7               0
    8               0
    9               0

    Running stat_test.lua...
    Initial ticks:  99
    New ticks:      42
    Initial name:   Initial status
    New name:       New status
    Shutting down... byeeeeeeeeeeeeeeee!

    // global variable shared between C and Lua
    Status *status;
    
    int main(int argc, char *argv[]) {
        char *files[2] = {
            "col_test.lua",
            "stat_test.lua"
        };
    
        // Initialize the LUA state
        printf("Initialize the LUA state\n");
        lua_State *L = luaL_newstate();
        printf("Opening libs\n");
        luaL_openlibs(L);
    
        // Load the LUA Collection library
        printf("Loading Lua interface libraries\n");
        load_collection(L);
        // Load the LUA Status library
        load_status(L);
    
        // Allocate the variable "status" to the Lua engine
        printf("Allocating the status variable\n");
        status = newStatus(99, "Initial status");
    
        // Load and execute the Lua scripts
        printf("Executing tests\n");
        for( int i = 0; i < 2; i++) {
            printf("\nRunning %s...\n", files[i]);
            if (luaL_dofile(L, files[i]) != LUA_OK) {
                fprintf(stderr, "Error running script: %s\n", lua_tostring(L, -1));
                break;
            } else {
                lua_pop(L, lua_gettop(L));
            }
        }
    
        // Close the LUA state
        // this will also call cleanup functions
        lua_close(L);
        printf("Shutting down... byeeeeeeeeeeeeeeee!\n");
        return 0;
    }
