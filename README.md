# luagsdemo
Program to demo embedding the IIgs flavor of LUA

Built using the lua.lib produced by https://github.com/artgreen/lua-iigs

Examples showing how lua.lib could be embedded into an arbitrary C program.

- Setting up the Lua engine
- Creating a Lua module in C and exporting it to the Lua engine
- Calling Lua script files
- Using Lua as a configuration language
- Exporting an arbitrary function to the Lua engine
- Exporting a global C variable to the Lua engine
- Showing how to access a struct in Lua (Lua requires getters and setters)

Implements ths start of a C/Lua interface layer intended to hide Lua from the calling program altogether.

Works as both a System16 program and ORCA EXE.

To do:
- Execute a Lua script and then retrieve global Lua variables from C
- Execute Lua code in a C string
- Calling a Lua function from C

Issues:
- Sometimes I hear voices.

Powered by 
 - ORCA/C https://github.com/byteworksinc/ORCA-C
 - GoldenGate https://juiced.gs/store/golden-gate/
